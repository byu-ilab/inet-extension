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

WebCacheNewAPI::WebCacheNewAPI() {
	resourceCache = NULL;
}

WebCacheNewAPI::~WebCacheNewAPI() {
	if (resourceCache)
	{
	  delete resourceCache;
	}
}
void WebCacheNewAPI::initialize() {
	httptServerBase::initialize();

	controller = check_and_cast<httptController *>(simulation.getSystemModule()->getSubmodule(par("controller")));

    //controller = dynamic_cast<httptController*>(getParentModule()->getParentModule()->getSubmodule("controller")); // not sure if this is it
	/*if (controller == NULL) {
		error("Controller module not found");
	}
	*/
	upstream_cache = par("serverwww").stringValue();
	request_timeout = par("request_timeout");

	int cache_size = par("cacheSize");
	cache_size *= 1024;
	resourceCache = new LRUCache(cache_size);
	updateDisplay();
	requestsReceived = 0;
	serverSocketsBroken=0;
	serverSocketsOpened=0;
	clientSocketsBroken = 0;
	clientSocketsOpened = 0;
	hits = 0;
	misses = 0;
	WATCH(numBroken);
	WATCH(socketsOpened);

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
	EV_SUMMARY<<"Current Used Cache Space (Bytes): "<<(((int)par("cacheSize")) - (((LRUCache *)resourceCache)->getRemainingCapacity()))<<endl;
	EV_SUMMARY<<"Current Unused Cache Space (Bytes): "<<((LRUCache *)resourceCache)->getRemainingCapacity()<<endl;
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

	switch(ret_status) {
		case TCPSocketAPI::CB_E_UNKNOWN:
			opp_error("WebCacheNewAPI::acceptCallback: unknown error.");
			break;
		default: // read data from new socket
			ConnInfo * ci = new ConnInfo();
			ci->sockType = SERVER;
			ci->ds_request = NULL;
			tcp_api->recv(ret_status, ci);
			break;
	}
}

// @param socket_id -- the id of the connected socket
// @param ret_status -- the status of the previously invoked connect method
// @param yourPtr -- the pointer to the data passed to the connect method
void WebCacheNewAPI::connectCallback(int socket_id, int ret_status, void * myPtr){
	Enter_Method_Silent();
	// check that socket_id is valid socket, i.e. in the set?
	ConnInfo * data = static_cast<ConnInfo *>(myPtr);
	if (!data) {
		cout << "connect: No connection info returned!";
		closeSocket(socket_id);
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
	if (!data) {
		cout << "WebCacheNewAPI::recvCallback: No watch data returned!";
		closeSocket(socket_id);
		return;
	}

	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		// msg is NULL so don't call delete
		closeSocket(socket_id);
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

	// Use the switch if different actions should be taken for a given error
//	bool actAsClient = data->sockType == CLIENT;
//	switch(ret_status) {
//	case TCPSocketAPI::CB_E_TIMEOUT:
//		handleTimeout(socket_id);
//		break;
//	case TCPSocketAPI::CB_E_UNKNOWN:
//		// do nothing special
//		break;
//	case TCPSocketAPI::CB_E_REFUSED:
//	case TCPSocketAPI::CB_E_RESET:
//	case TCPSocketAPI::CB_E_CLOSED:
//		closeSocket(socket_id);
//		break;
//	default: // positive # of bytes received
//		if (actAsClient)
//			processUpstreamResponse(socket_id, msg, data);
//		else
//		{
//			// handleReceivedMessage will return an error reply if there is a problem with the
//			// message, otherwise control will get passed to handleGetRequest which will return
//			// NULL
//			httptReplyMessage * errorReply = handleRequestMessage(msg);
//			if (errorReply)
//			{
//				delete msg;
//				tcp_api->send(socket_id, errorReply);
//			}
//			else
//			{
//				processDownstreamRequest(socket_id, msg, data);
//			}
//		}
//		break;
//	}
}

/**
 * Sends a request to an upstream cache or server.
 * @param ds_request the request received from a client downstream.
 */
void WebCacheNewAPI::makeUpstreamRequest(int socket_id, ConnInfo * data) {

	//httptRequestMessage * ds_request = data->ds_request;
	//httptRequestMessage * us_request = ds_request->dup();
	httptRequestMessage * us_request = new httptRequestMessage(*(data->ds_request));
	if (dynamic_cast<httptByteRangeRequestMessage *>(us_request))
	{
		throw cRuntimeError(this, "request to upstream server is still a byte range request");
	}
	us_request->setTargetUrl(par("serverwww"));
	us_request->setOriginatorUrl(wwwName.c_str());//ds_request->targetUrl());

	ConnInfo * ci =new ConnInfo;
	ci->sockType = CLIENT;
	ci->ds_request = NULL;

	tcp_api->send(socket_id,us_request);
	tcp_api->recv(socket_id,ci);
	delete data->ds_request;
	delete data;
}

// Receive a response containing move data from an upstream cache or server.
void WebCacheNewAPI::processUpstreamResponse(int socket_id, cPacket * msg, ConnInfo * data) {

	httptReplyMessage * reply = dynamic_cast<httptReplyMessage *>(msg);
	// TODO use check_and_cast?
	if (!reply) {
		cout << "processResponse: message is not an httptReply!";
		closeSocket(socket_id);
		return;
	}
	logResponse(reply);

	if (!isErrorMessage(reply)) {
		// add resource to cache
		Resource * wr = new WebResource(extractURLFromResponse(reply),reply->getByteLength());
		if (wr->getSize() <= resourceCache->getCapacity()) {
		  resourceCache->add(wr);
		}

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
	closeSocket(socket_id); // close the socket to upstream server.
	delete reply;
	delete data;
}

void WebCacheNewAPI::respondToClientRequest(int socket_id, httptRequestMessage * request, Resource * resouce)
{
	ASSERT(request && resouce);

	httptReplyMessage * reply = NULL;
	httptByteRangeRequestMessage * br_request = dynamic_cast<httptByteRangeRequestMessage *>(request);
	if (br_request)
	{
		reply  = generateByteRangeReply(br_request, resouce->getID(), resouce->getSize(), rt_text); // TODO add type extractor from extension? or add type to web resource?
	}
	else
	{
		reply = new httptReplyMessage();
		fillinReplyMessage(reply, request, resouce->getID(), 200, resouce->getSize(), rt_text);
	}
	tcp_api->send(socket_id, reply);
}

bool WebCacheNewAPI::isErrorMessage(httptReplyMessage *msg)
{
	return msg->result() != 200;
}

httptReplyMessage * WebCacheNewAPI::handleGetRequest(httptRequestMessage * msg, string resource_url)
{
	return NULL;
}
// A client (or cache) requests a file from me.
// If I have it, send it along.  Otherwise, initiate a request from an upstream host.
void WebCacheNewAPI::processDownstreamRequest(int socket_id, cPacket * msg, ConnInfo * data) {

	httptRequestMessage * request = check_and_cast<httptRequestMessage *>(msg);
	requestsReceived++;

	string url = extractURLFromRequest(request);
	Resource * wr_temp = new WebResource(url, 0); // works because comparator used only looks at the ID not the size
	Resource * wr_incache = resourceCache->has(wr_temp);
	if (wr_incache) {
		resourceCache->renew(wr_temp); // update timestamp on LRU cache.
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
			openUpstreamSocket(us_cinfo);
		}
	}
	// ask client for anything else it might send:
	tcp_api->recv(socket_id,data);
	updateDisplay(); // draw.
	//delete request;
	delete wr_temp;
	//delete data;
}
/**
 * takes a URL from a request message.  for now, this must be a correctly formatted one.
 */
string WebCacheNewAPI::extractURLFromRequest(httptRequestMessage * request) {
	cStringTokenizer tokenizer = cStringTokenizer(request->heading()," ");
	 vector<string> res = tokenizer.asVector();
	 string r_msg = res[1];
	 if (!strcmp(r_msg.c_str(), "/")) {
		 r_msg = "root";
	 }
	 r_msg = trimLeft(r_msg, "/");
	 return r_msg;
}
string WebCacheNewAPI::extractURLFromResponse(httptReplyMessage * response) {
	cStringTokenizer tokenizer = cStringTokenizer(response->getFullName(),"()");
	 vector<string> res = tokenizer.asVector();
	 string resp = res[1];
	 return resp;
}

/*
 * Handle timeout.  Assumed to be for client-like sockets only.
 */
void WebCacheNewAPI::handleTimeout(int socket_id) {
	EV_DEBUG << "handling timeout..."<<endl;
	opp_error("WebCacheNewAPI::handleTimeout: not supposed to be here.");
}

int WebCacheNewAPI::openUpstreamSocket(ConnInfo * data) {
	int fd = tcp_api->socket(this);
	tcp_api->setTimeout(fd, request_timeout);

	//sockets.insert(fd);

	// find module for server.
	int connect_port;
	char szModuleName[127];

	controller->getServerInfo(upstream_cache.c_str(),szModuleName,connect_port);
	EV << "detain it right here\n";
	tcp_api->connect(fd , szModuleName, connect_port, (void *) data);
	return fd;
}

void WebCacheNewAPI::closeSocket(int socket_id) {
	//std::set<int>::iterator i = sockets.find(socket_id);
	ConnInfo * data = static_cast<ConnInfo *>(tcp_api->getMyPtr(socket_id));
	if (data) {
		delete data;
	}
	tcp_api->close(socket_id);
	//sockets.erase(i);
}

void WebCacheNewAPI::updateDisplay() {
	if ( ev.isGUI() && resourceCache)
	{
		//httptServerBase::updateDisplay(); // DON'T do this if you don't want ugly arrows
		char buf[1024];
		float h = 0;
		if (requestsReceived > 0)
			h = 100.0 * hits / requestsReceived;
		int cacheSize = resourceCache->getCapacity(); //(int)par("cacheSize");
		int remaining = resourceCache->getRemainingCapacity();
		float full =0;
		if (cacheSize > 0)
			full = 100.0 * (cacheSize-remaining) /cacheSize;
		sprintf( buf, "Req: %ld\nHit: %.1f\%\nCap: %.1fKB\nFull: %.1f\%", requestsReceived,h,cacheSize/1000.0, full);
		getParentModule()->getDisplayString().setTagArg("t",0,buf);
	} /*else if (ev.isGUI() ){
		httptServerBase::updateDisplay();
	}*/
}
