//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "WebCacheNewAPI.h"
#include "TCPSocketAPIAppUtils.h"

Define_Module(WebCacheNewAPI);

// EnableDebugging(false); // TODO
#define DEBUG_CLASS false

WebCacheNewAPI::WebCacheNewAPI() : pendingRequests(), contentFilter(), shouldFilter(false) {

	resourceCache = NULL;

	requestsReceived = 0;
	serverSocketsBroken=0;
	serverSocketsOpened=0;

	clientSocketsBroken = 0;
	clientSocketsOpened = 0;
	currentSocketsOpenToServer = 0;

	hits = 0;
	misses = 0;

	socket_cap = 8;
	cur_socket = 0;
	activeSockets = new int[socket_cap];
	for(int i=0; i<socket_cap; i++)
		activeSockets[i] = -1;
}

WebCacheNewAPI::~WebCacheNewAPI() {
	if (resourceCache)
	{
	  delete resourceCache;
	}
}
void WebCacheNewAPI::initialize() {
	httptServerBase::initialize();

	upstream_cache = par("serverwww").stdstringValue();
	request_timeout = par("request_timeout");

	int64 cache_size = par("cacheSize").longValue();
		ASSERT(cache_size > 0);
	resourceCache = new LRUCache((uint64) cache_size);

	string config_filename = par("filterConfig").stdstringValue();
	if (!config_filename.empty())
	{
		shouldFilter = true;
		contentFilter.configureFromXML(config_filename);
	}

	updateDisplay();

	WATCH(serverSocketsOpened);
	WATCH(serverSocketsBroken);

	WATCH(clientSocketsOpened);
	WATCH(clientSocketsBroken);
	WATCH(currentSocketsOpenToServer);

	WATCH(hits);
	WATCH(misses);

	tcp_api = findTCPSocketAPI(this);
    cMessage * start = new cMessage("START",START);
    scheduleAt(simTime()+activationTime,start);
}

void WebCacheNewAPI::finish() {
	// Report sockets related statistics.
	EV_SUMMARY << "Server Sockets opened: " << serverSocketsOpened << endl;
	EV_SUMMARY << "Server Broken connections: " << serverSocketsBroken << endl;

	// Record the sockets related statistics
	recordScalar("server.sock.opened", serverSocketsOpened);
	recordScalar("server.sock.broken", serverSocketsBroken);

	// Report sockets related statistics.
	EV_SUMMARY << "Client Sockets opened: " << clientSocketsOpened << endl;
	EV_SUMMARY << "Client Broken connections: " << clientSocketsBroken << endl;
	// Record the sockets related statistics
	recordScalar("client.sock.opened", clientSocketsOpened);
	recordScalar("client.sock.broken", clientSocketsBroken);

	// record cache-related statistics
	EV_SUMMARY<<"Cache Hits: "<< hits<<endl;
	EV_SUMMARY<<"Cache Misses: "<< misses<<endl;

	// figure out the units
	uint64 totalSpace = resourceCache->getCapacity();
	uint64 freeSpace = resourceCache->getRemainingCapacity();
	uint64 usedSpace = totalSpace - freeSpace;
	ByteUnit unit = determineByteUnit(totalSpace, unittype_bibyte);
	string unitstr = getByteUnitAsString(unit);
	double convfactor = getMultiplicativeFactor(unit_B, unittype_bibyte, unit, unittype_bibyte);

	EV_SUMMARY<<"Current Used Cache Space: " << ( (double) usedSpace * convfactor ) << " " << unitstr <<endl;

	EV_SUMMARY<<"Current Unused Cache Space: " << ((double) freeSpace * convfactor ) << " " << unitstr <<endl;
}

void WebCacheNewAPI::handleMessage(cMessage * msg) {
	// setup listener socket.
	if (msg->getKind() == START) {
		int port = par("port");
	    int fd = tcp_api->socket(this);
	    tcp_api->bind(fd,"",port);
	    tcp_api->listen(fd);

	    tcp_api->accept (fd);
	}
//	if (msg->isSelfMessage()){
//    	//opp_error("WebCacheNewAPI::handleMessage(): received a non self message");
//    }
	httptServerBase::handleMessage(msg); // update the display
	delete msg;
}

/// Handles the acceptance of a new socket.
/// @param socket_id -- the descriptor for the listening socket
/// @param ret_status -- the status of the previously invoked accept method or
///		the descriptor for the accepted socket
/// @param yourPtr -- the pointer to the data/struct/object previously passed
///  	to accept
/// @details
/// On success: ret_status will be the descriptor of the accepted socket
/// On error: ret_status will be TCPSocketAPI::CB_E_UNKNOWN from the
/// 	TCPSocketAPI::CALLBACK_ERROR enumeration
void WebCacheNewAPI::acceptCallback(int socket_id, int ret_status, void * yourPtr) {
	Enter_Method_Silent();

	// signal next accept
	tcp_api->accept(socket_id);

	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		error("unknown socket error: %s", TCPSocketAPI::getCallbackErrorName(ret_status).c_str());
	}

	// read data from new socket
	ConnInfo * ci = new ConnInfo();
	ci->sockType = SERVER;
	ci->ds_request = NULL;

	tcp_api->recv(ret_status, ci);
	serverSocketsOpened++;
}

// @param socket_id -- the id of the connected socket
// @param ret_status -- the status of the previously invoked connect method
// @param yourPtr -- the pointer to the data passed to the connect method
void WebCacheNewAPI::connectCallback(int socket_id, int ret_status, void * myPtr){

	Enter_Method_Silent();

	// TODO check that socket_id is valid socket, i.e. in the set?

	ConnInfo * data = static_cast<ConnInfo *>(myPtr);
	if (!data)
	{
		LOG_DEBUG("No connection info returned!");
		closeSocket(socket_id);

		clientSocketsBroken++;
		currentSocketsOpenToServer--;

		return;
	}

	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		LOG_DEBUG("Socket error: "<<TCPSocketAPI::getCallbackErrorName(ret_status));
		closeSocket(socket_id);

		clientSocketsBroken++;
		currentSocketsOpenToServer--;

		return;
	}

	makeUpstreamRequest(socket_id, data);
}

// @param socket_id -- the id of the accepted socket
// @param ret_status -- the status of the previously invoked recv method or
//						the number of bytes received
// @param msg -- a pointer to the received message
// @param yourPtr -- the pointer to the data passed to the accept method
void WebCacheNewAPI::recvCallback(int socket_id, int ret_status,
	cPacket * msg, void * myPtr){
	Enter_Method_Silent();
	// recv is called on downstream connections to receive requests,
	// and it is called on upstream connections to receive the replies for requests.


	ConnInfo * data = static_cast<ConnInfo *>(myPtr);
	if (!data)
	{
		LOG_DEBUG("No watch data returned!");
		closeSocket(socket_id);
		return;
	}

	// Use a switch statement if different actions should be taken for a given error
	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		// msg is NULL so don't call delete
		LOG_DEBUG("Callback error is: "<<TCPSocketAPI::getCallbackErrorName(ret_status));
		closeSocket(socket_id);

		if (ret_status == TCPSocketAPI::CB_E_RESET ||
				ret_status == TCPSocketAPI::CB_E_REFUSED ||
				ret_status == TCPSocketAPI::CB_E_UNKNOWN)
		{
			if (data->sockType == SERVER)
			{
				serverSocketsBroken++;
				LOG_DEBUG("Server sockets broken so far: "<<serverSocketsBroken);
			}
			else if (data->sockType == CLIENT)
			{
				clientSocketsBroken++;
				currentSocketsOpenToServer--;
				LOG_DEBUG("Client sockets broken so far: "<<clientSocketsBroken);
			}
			// else there is no variable to update
		}
		return;
	}

	if (data->sockType == CLIENT)
	{
		processUpstreamResponse(socket_id, msg, data);
		return;
	}

	// handleReceivedMessage will return an error reply if there is a problem with the
	// message, otherwise control will get passed to handleGetRequest which will return
	// NULL
	httptReplyMessage * errorReply = handleRequestMessage(msg);
	if (errorReply)
	{
		delete msg;
		tcp_api->send(socket_id, errorReply);
		tcp_api->recv(socket_id,myPtr);
	}
	else
	{
		processDownstreamRequest(socket_id, msg, data);
	}
}

/**
 * Sends a request to an upstream cache or server.
 * @param ds_request the request received from a client downstream.
 */
void WebCacheNewAPI::makeUpstreamRequest(int socket_id, ConnInfo * data) {

	httptRequestMessage * us_request = data->ds_request->dup();

	us_request->setTargetUrl(par("serverwww"));
	us_request->setOriginatorUrl(wwwName.c_str());
	us_request->setFirstBytePos(BRS_UNSPECIFIED);
	us_request->setLastBytePos(BRS_UNSPECIFIED);// just to be safe

	ConnInfo * ci =new ConnInfo;
	ci->sockType = CLIENT;
	ci->ds_request = NULL;

	LOG_DEBUG("requesting from server: "<<us_request->heading());
	tcp_api->send(socket_id,us_request);
	tcp_api->recv(socket_id,ci);
	delete data->ds_request;
	delete data;
}

// Receive a response containing move data from an upstream cache or server.
void WebCacheNewAPI::processUpstreamResponse(int socket_id, cPacket * msg, ConnInfo * data) {

	httptReplyMessage * reply = dynamic_cast<httptReplyMessage *>(msg);

	if (!reply) {
		LOG_DEBUG("Message is not an httptReply!");
		closeSocket(socket_id);
		currentSocketsOpenToServer--;
		return;
	}
	logResponse(reply);

	if (!isErrorMessage(reply)) {
		// determine whether the resource should be/can be added to the cache
		string uri = extractURLFromResponse(reply);
		Resource * wr = new WebResource(uri,reply->getByteLength(), reply->contentType(), reply->payload());

		string ext = parseResourceName(uri)[2];
		if (!shouldFilter || /* implicit shouldFilter && */ contentFilter.containsExtension(ext))
		{
			if (wr->getSize() <= resourceCache->getCapacity()) {
			  resourceCache->add(wr);
			  LOG_DEBUG("added: "<<wr->getID());
			}
		}
		// else just forward it to waiting clients

		// send a response to each waiting client.
		list<RequestRecord> requests_to_service = pendingRequests.getRequestsForResource(wr->getID());
		list<RequestRecord>::iterator it;
		for (it = requests_to_service.begin(); it != requests_to_service.end(); it++)
		{
			respondToClientRequest((*it).interface_id, (*it).request_msg_ptr, wr);
		}

		pendingRequests.removeAndDeleteRequestsForResource(wr->getID());
	}
	updateDisplay();

	//closeSocket(socket_id); // close the socket to upstream server.
			//Don't close here in case it can be used again right away to request something else from the server
	//currentSocketsOpenToServer--;
	delete reply;
	delete data;
}

void WebCacheNewAPI::respondToClientRequest(int socket_id, httptRequestMessage * request, Resource * resource)
{
	ASSERT(request && resource);

	// checks if it is indeed a byte range request
	httptReplyMessage * reply = generateByteRangeReply(request, resource->getID(), resource->getSize(), resource->getType());
	reply->setPayload(resource->getContent().c_str());
	LOG_DEBUG("sent to client: "<<reply->heading()<<" for resource: "<<reply->relatedUri());
	tcp_api->send(socket_id, reply);
}

bool WebCacheNewAPI::isErrorMessage(httptReplyMessage *msg)
{
	return msg->result() != 200;
}

httptReplyMessage * WebCacheNewAPI::handleGetRequest(httptRequestMessage * msg, string resource_url)
{
	// we may not be able to return a response immediately
	return NULL;
}
// A client (or cache) requests a file from me.
// If I have it, send it along.  Otherwise, initiate a request from an upstream host.
void WebCacheNewAPI::processDownstreamRequest(int socket_id, cPacket * msg, ConnInfo * data) {
	httptRequestMessage * request = check_and_cast<httptRequestMessage *>(msg);
	requestsReceived++;
	LOG_DEBUG("received request for: "<<request->heading());

	string url = extractURLFromRequest(request);
	Resource * wr_temp = new WebResource(url, 0); // works because comparator used only looks at the ID not the size
	Resource * wr_incache = resourceCache->has(wr_temp);

	if (wr_incache) {
		resourceCache->renew(wr_incache); // update timestamp on LRU cache.
		hits++;

		respondToClientRequest(socket_id, request, wr_incache);
		delete request;
	} else {
		misses++;
		// request resource, only if it is the first request of its type
		bool isNew = pendingRequests.addRequest(socket_id, url, request);
		if (isNew) {
			ConnInfo * us_cinfo = new ConnInfo;
			us_cinfo->sockType = CLIENT;
			us_cinfo->ds_request = request->dup();

			//Rotate through a specified num of sockets. If not active(the socket has been closed) then open a new socket to take its place.
			int active_fd = activeSockets[cur_socket];
			if (active_fd != -1)
				makeUpstreamRequest(active_fd, us_cinfo);
			else{
				active_fd = openUpstreamSocket(us_cinfo);
				activeSockets[cur_socket] = active_fd;
			}
			if (cur_socket+1 < socket_cap)
				cur_socket++;
			else
				cur_socket=0;
		}
	}

	// ask client for anything else it might send:
	tcp_api->recv(socket_id,data);

	updateDisplay();
	delete wr_temp;

	// DO NOT delete 'request' or 'data,' request may be stored in pendingRequests,
	// and/or it was deleted in the if clause.  'data' is being used to recv on the
	// client socket.
}
/**
 * takes a URL from a request message.  for now, this must be a correctly formatted one.
 */
string WebCacheNewAPI::extractURLFromRequest(httptRequestMessage * request) {
	string r_msg = request->uri();
	if (!r_msg.empty())
	{
		return r_msg;
	}

	// else parse it out
	cStringTokenizer tokenizer = cStringTokenizer(request->heading()," ");
	vector<string> res = tokenizer.asVector();
	r_msg = res[1];
	if (!strcmp(r_msg.c_str(), "/")) {
		r_msg = "root";
	}
	r_msg = trimLeft(r_msg, "/");
	return r_msg;
}
string WebCacheNewAPI::extractURLFromResponse(httptReplyMessage * response) {
	string r_msg = response->relatedUri();
	if (!r_msg.empty())
	{
		return r_msg;
	}

	// else parse it out, this format is not well defined  <-- TODO fix it
	cStringTokenizer tokenizer = cStringTokenizer(response->getFullName(),"()");
	vector<string> res = tokenizer.asVector();
	r_msg = res[1];
	return r_msg;
}

/*
 * Handle timeout.  Assumed to be for client-like sockets only.
 */
//void WebCacheNewAPI::handleTimeout(int socket_id) {
//	LOG_DEBUG("handling timeout...");
//	opp_error("WebCacheNewAPI::handleTimeout: not supposed to be here.");
//}

int WebCacheNewAPI::openUpstreamSocket(ConnInfo * data) {
	int fd = tcp_api->socket(this);
	tcp_api->setTimeout(fd, request_timeout);

	//sockets.insert(fd);

	// find module for server.
	int connect_port;
	char szModuleName[127];

	controller->getServerInfo(upstream_cache.c_str(),szModuleName,connect_port);
	LOG_DEBUG("detain it right here");
	tcp_api->connect(fd , szModuleName, connect_port, (void *) data);

	clientSocketsOpened++;
	currentSocketsOpenToServer++;

	return fd;
}

void WebCacheNewAPI::closeSocket(int socket_id) {
	//std::set<int>::iterator i = sockets.find(socket_id);
	ConnInfo * data = static_cast<ConnInfo *>(tcp_api->getMyPtr(socket_id));
	if (data) {
		delete data;
	}
	pendingRequests.removeAndDeleteAllRequestsOnInterface(socket_id);
	tcp_api->close(socket_id);
	//sockets.erase(i);
	for(int i=0; i<socket_cap; i++){
		if(activeSockets[i]==socket_id){
			activeSockets[i] = -1;
			return;
		}
	}
}

void WebCacheNewAPI::updateDisplay() {
	if ( ev.isGUI() && resourceCache)
	{
		//httptServerBase::updateDisplay(); // DON'T do this if you don't want ugly arrows
		char buf[1024];
		float h = 0;
		if (requestsReceived > 0)
			h = 100.0 * hits / requestsReceived;
		unsigned long cacheSize = resourceCache->getCapacity(); //(int)par("cacheSize");
		unsigned long remaining = resourceCache->getRemainingCapacity();
		float full =0;
		if (cacheSize > 0)
			full = 100.0 * (cacheSize-remaining) / cacheSize;

		// figure out the units
		ByteUnit unit = determineByteUnit(cacheSize, unittype_bibyte);
		string unitstr = getByteUnitAsString(unit);
		double convfactor = getMultiplicativeFactor(unit_B, unittype_bibyte, unit, unittype_bibyte);
		double capacity = (double) cacheSize * convfactor;
		sprintf( buf, "Req: %ld\nHit: %.1f\%\nCap: %.1f%s\nFull: %.1f\%", requestsReceived,h,
				capacity, unitstr.c_str(), full);
		getParentModule()->getDisplayString().setTagArg("t",0,buf);
	} /*else if (ev.isGUI() ){
		httptServerBase::updateDisplay();
	}*/
}
