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
#include "httpDuplicateMessageEventListener.h"
#include <sstream>

Define_Module(WebCacheNewAPI);

#define DEBUG_CLASS false

#define TRACK_HTTP_MESSAGES true

WebCacheNewAPI::WebCacheNewAPI()
	: /*pendingUpstreamRequests(),*/ pendingDownstreamRequests(), contentFilter(),
	  downstreamRequestsReceived("cache.requests.downstream.received", (unsigned long) 0),
	  hits("cache.requests.downstream.hits", (unsigned long) 0),
	  misses("cache.requests.downstream.misses", (unsigned long) 0),
	  serverSocketsBroken("cache.sockets.server.broken", (unsigned long) 0),
	  serverSocketsOpened("cache.sockets.server.opened", (unsigned long) 0),
	  upstreamRequestTxdelay("cache.requests.upstream.txdelay", SimTime(0) )
{
	shouldFilter = false;
	resourceCache = NULL;
//
//	downstreamRequestsReceived = 0;
//	serverSocketsBroken = 0;
//	serverSocketsOpened = 0;

//	clientSocketsBroken = 0;
//	clientSocketsOpened = 0;
//	currentSocketsOpenToServer = 0;

//	hits = 0;
//	misses = 0;

//	socket_cap = 0;
//	cur_socket = 0;
//	upstreamSocketDescriptors = NULL;
	upstreamSocketPool = NULL;

//	upstream_txdelay_histogram = NULL;
//	upstream_txdelay_vector = NULL;
}

WebCacheNewAPI::~WebCacheNewAPI() {

	LOG_DEBUG_FUN_BEGIN("");

	deleteSafe(resourceCache);

//	if (upstreamSocketDescriptors)
//	{
//		delete[] upstreamSocketDescriptors;
//	}

	if (upstreamSocketPool)
	{
		ConnInfo * ci = static_cast<ConnInfo *>(upstreamSocketPool->getMyRecvCallbackData());
		deleteSafe(ci);
		delete upstreamSocketPool;
	}

	for (map<int, ConnInfo *>::iterator itr = socketConnInfoMap.begin(); itr != socketConnInfoMap.end(); ++itr)
	{ // can't use the tcp_api because it may already have been destroyed
		deleteSafe(itr->second);
	}

//	deleteSafe(upstream_txdelay_histogram);
//	deleteSafe(upstream_txdelay_vector);

	LOG_DEBUG_FUN_END("");
}
void WebCacheNewAPI::initialize() {
	httptServerBase::initialize();

	upstream_server = par("serverwww").stdstringValue();
	request_timeout = par("request_timeout");

	int64 cache_size = par("cacheSize").longValue();
		ASSERT(0 < cache_size);
	resourceCache = new LRUCache((uint64) cache_size);

	string config_filename = par("filterConfig").stdstringValue();
	if (!config_filename.empty())
	{
		shouldFilter = true;
		contentFilter.configureFromXML(config_filename);
	}

	// POINT OF INTEREST
	int socket_cap = par("maxUpstreamSockets");
		ASSERT( 0 < socket_cap );
//	upstreamSocketDescriptors = new int[socket_cap];
//	for (int i =0; i < socket_cap; i++)
//	{
//		upstreamSocketDescriptors[i] = US_SOCK_NONE;
//	}

	// find module for server.
	int connect_port;
	char szModuleName[127];

	controller->getServerInfo(upstream_server.c_str(),szModuleName,connect_port);

	tcp_api = findTCPSocketAPI(this);

	ConnInfo * us_cinfo = new ConnInfo();
	us_cinfo->sockType = WCST_CLIENT;
	us_cinfo->numPendingResponses = 0;

	int max_load = 1;
	upstreamSocketPool = new ActiveTCPSocketPool(tcp_api, this, socket_cap,
			szModuleName, connect_port, request_timeout, max_load /*UNLIMITED_LOAD*/, (void *) us_cinfo);

	resend_request_threshold = par("resendRequestThreshold");
		ASSERT(0 < resend_request_threshold);

	updateDisplay();

//	upstream_txdelay_histogram = new cDoubleHistogram("cache.sockets.upstream.txdelay");
//	upstream_txdelay_vector = new cOutVector("cache.sockets.upstream.txdelay");

//	WATCH(serverSocketsOpened);
//	WATCH(serverSocketsBroken);

//	WATCH(clientSocketsOpened);
//	WATCH(clientSocketsBroken);
//	WATCH(currentSocketsOpenToServer);

//	WATCH(hits);
//	WATCH(misses);

	// register signal names
	reqev_signal = registerSignal(SIGNAME_REQEV);
	servsockev_signal = registerSignal(SIGNAME_SOCKEV);
	txdelay_signal = registerSignal(SIGNAME_TXDELAY);

	if (TRACK_HTTP_MESSAGES)
	{
		http_msg_ev_signal = registerSignal("httpmmsgevent");
		subscribe(http_msg_ev_signal, httpDuplicateMessageEventListener::getInstance());
	}

//	tcp_api = findTCPSocketAPI(this);
    cMessage * start = new cMessage("START",START);
    scheduleAt(simTime()+activationTime,start);
}

void WebCacheNewAPI::finish() {
	// Report sockets related statistics.
	EV_SUMMARY << "Server Sockets opened: " << serverSocketsOpened.unsignedLongValue() << endl;
	EV_SUMMARY << "Server Broken connections: " << serverSocketsBroken.unsignedLongValue() << endl;

	// Record the sockets related statistics
//	recordScalar("server.sock.opened", serverSocketsOpened);
//	recordScalar("server.sock.broken", serverSocketsBroken);

//	// Report sockets related statistics.
//	EV_SUMMARY << "Client Sockets opened: " << clientSocketsOpened << endl;
//	EV_SUMMARY << "Client Broken connections: " << clientSocketsBroken << endl;
//	// Record the sockets related statistics
//	recordScalar("client.sock.opened", clientSocketsOpened);
//	recordScalar("client.sock.broken", clientSocketsBroken);

	// record cache-related statistics
//	EV_SUMMARY<<"Cache Hits: "<< hits<<endl;
//	EV_SUMMARY<<"Cache Misses: "<< misses<<endl;

	// figure out the units
	uint64 totalSpace = resourceCache->getCapacity();
	uint64 freeSpace = resourceCache->getRemainingCapacity();
	uint64 usedSpace = totalSpace - freeSpace;
	ByteUnit unit = determineByteUnit(totalSpace, unittype_bibyte);
	string unitstr = getByteUnitAsString(unit);
	double convfactor = getMultiplicativeFactor(unit_B, unittype_bibyte, unit, unittype_bibyte);

	EV_SUMMARY<<"Current Used Cache Space: " << ( (double) usedSpace * convfactor ) << " " << unitstr <<endl;

	EV_SUMMARY<<"Current Unused Cache Space: " << ((double) freeSpace * convfactor ) << " " << unitstr <<endl;

	// for any outstanding requests
	URIVarientSimTimeMap::iterator utx_itr;
	for (utx_itr = upstream_txstart_map.begin(); utx_itr != upstream_txstart_map.end(); utx_itr++)
	{
		simtime_t txdelay = simTime() - utx_itr->second.time;
		upstreamRequestTxdelay.setValue(txdelay);
		emit(txdelay_signal, &upstreamRequestTxdelay);
//		upstream_txdelay_vector->record(txdelay);
//		upstream_txdelay_histogram->collect(txdelay);
	}

//	upstream_txdelay_histogram->record();
}

void WebCacheNewAPI::handleMessage(cMessage * msg) {
	// setup listener socket.
	if (msg->getKind() == START) {
		int port = par("port");
	    int fd = tcp_api->socket(this);
	    tcp_api->bind(fd,"",port);
	    tcp_api->listen(fd);

	    tcp_api->accept (fd);
	    // TODO use sockType WCST_LISTENER or remove this from the enum?
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

	// The acceptCallback shouldn't receive errors via ret_status, if it does then
	// the TCP socket API changed
	ASSERT(!TCPSocketAPI::isCallbackError(ret_status));

	// read data from new socket
	ConnInfo * ci = new ConnInfo();
	ci->sockType =  WCST_SERVER;
	//ci->ds_request = NULL;
	ci->numPendingResponses = -1; //XXX

	socketConnInfoMap[ret_status] = ci;

	tcp_api->recv(ret_status, ci);

	serverSocketsOpened.increment();
	emit(servsockev_signal, &serverSocketsOpened);
}

//// @param socket_id -- the id of the connected socket
//// @param ret_status -- the status of the previously invoked connect method
//// @param yourPtr -- the pointer to the data passed to the connect method
//void WebCacheNewAPI::connectCallback(int socket_id, int ret_status, void * myPtr){
//
//	Enter_Method_Silent();
//
//	ConnInfo * data = static_cast<ConnInfo *>(myPtr);
//
//	/* Error Checking and Handling */
//	// TODO check that socket_id is valid socket, i.e. in the set?
//	bool handleError = false;
//	if (!data)
//	{
//		LOG_DEBUG("No connection info returned!");
//		handleError = true;
//	}
//
//	if (TCPSocketAPI::isCallbackError(ret_status))
//	{
//		LOG_DEBUG("Socket error: "<<TCPSocketAPI::getCallbackErrorName(ret_status));
//		handleError = true;
//	}
//
//	if (handleError)
//	{
//		closeSocket(socket_id);
//
//		clientSocketsBroken++;
//		currentSocketsOpenToServer--;
//
//		return;
//	}
//	/* End Error Checking and Handling */
//
//	makeUpstreamRequest(socket_id, data);
//}

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

	ASSERT(myPtr != NULL);

	ConnInfo * data = static_cast<ConnInfo *>(myPtr);

	/* Error Checking and Handling */
//	if (!data)
//	{
//		LOG_DEBUG("No watch data returned!");
//		closeSocket(socket_id);
//		// no statistics can be updated
//		return;
//	}

	// Use a switch statement if different actions should be taken for a given error
	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		// msg is NULL so don't call delete
		LOG_DEBUG("Callback error is: "<<TCPSocketAPI::getCallbackErrorName(ret_status));
		closeSocket(socket_id);

		// check if it is a broken connection
		if (ret_status == TCPSocketAPI::CB_E_RESET ||
				ret_status == TCPSocketAPI::CB_E_REFUSED ||
				ret_status == TCPSocketAPI::CB_E_UNKNOWN)
		{
			if (data->sockType ==  WCST_SERVER)
			{
				serverSocketsBroken.increment();
				emit(servsockev_signal, &serverSocketsBroken);
				LOG_DEBUG("Server sockets broken so far: "<<serverSocketsBroken.unsignedLongValue());
			}
			else if (data->sockType == WCST_CLIENT)
			{
//				clientSocketsBroken++;
//				currentSocketsOpenToServer--;
//				LOG_DEBUG("Client sockets broken so far: "<<clientSocketsBroken);
			}
			// else there is no variable to update
		}
		//deleteSafe(socketConnInfoMap[socket_id]);
		return;
	}

	ASSERT(msg);
	/* End Error Checking and Handling */

	take(msg);

	if (data->sockType == WCST_CLIENT)
	{
		processUpstreamResponse(socket_id, msg, data);
//		if (0 < data->numPendingResponses)
//		{
//			data->numPendingResponses--;
//			tcp_api->recv(socket_id, data);
//		}
//		else
//		{
//			makeUpstreamRequest();
//		}
		LOG_DEBUG_FUN_END("handled upstream response")
		return;
	}

	// else data->sockType == WCST_SERVER

	// handleReceivedMessage will return an error reply if there is a problem with the
	// message, otherwise control will get passed to handleGetRequest which will return
	// NULL
	httptReplyMessage * errorReply = handleRequestMessage(msg);
	if (errorReply)
	{
		delete msg;
		tcp_api->send(socket_id, errorReply);
		tcp_api->recv(socket_id, myPtr);
	}
	else
	{
		processDownstreamRequest(socket_id, msg, data);
	}
	LOG_DEBUG_FUN_END("handled downstream request")
}

/**
 * Sends a request to an upstream cache or server.
 * @param ds_request the request received from a client downstream.
 */
void WebCacheNewAPI::makeUpstreamRequest(httptRequestMessage * ds_request_template) { //int socket_id, ConnInfo * data) {

	// if a request template is specified then duplicate it and modify it as necessary
	if (ds_request_template)
	{
		httptRequestMessage * us_request = ds_request_template->dup();
		stringstream stringbuilder;
		string uri = extractURLFromRequest(ds_request_template);
		stringbuilder << "GET "<<uri<<" HTTP/1.1";
		us_request->setName(stringbuilder.str().c_str());
		us_request->setHeading(stringbuilder.str().c_str());
		us_request->setUri(uri.c_str());
		us_request->setTargetUrl(upstream_server.c_str());
		us_request->setOriginatorUrl(wwwName.c_str());
		us_request->setFirstBytePos(BRS_UNSPECIFIED);
		us_request->setLastBytePos(BRS_UNSPECIFIED);// just to be safe

		//pendingUpstreamRequests.insert(us_request);

		if (TRACK_HTTP_MESSAGES)
		{
			http_msg_ev_datagram.setMessage(us_request);
			http_msg_ev_datagram.setInterfaceID(-1);
			emit(http_msg_ev_signal, &http_msg_ev_datagram);
		}

		upstream_txstart_map[URIVarientKey(us_request->uri(), DEFAULT_URI_VARIENT)] = MsgIdTimestamp(DEFAULT_MSG_ID, simTime());
		upstreamSocketPool->submitRequest(us_request);
	}

//	// open a connection if necessary and find a connected socket
//	if (pendingUpstreamRequests.isEmpty())
//	{ // then there are no requests to send
//		return;
//	}
//
//	int selected_fd = US_SOCK_NONE;
//	ConnInfo * selected_cinfo = NULL;
//	int current_fd = US_SOCK_NONE;
//	int fd_index = 0;
//	while (fd_index < socket_cap)
//	{
//		current_fd = upstreamSocketDescriptors[fd_index];
//		if ( current_fd == US_SOCK_NONE )
//		{
//			ConnInfo * us_cinfo = new ConnInfo;
//			us_cinfo->sockType = WCST_CLIENT;
//			us_cinfo->numPendingResponses = 0;
//			openUpstreamSocket(us_cinfo);
//			upstreamSocketDescriptors[fd_index] == US_SOCK_CONNECTING;
//		}
//		else if ( current_fd != US_SOCK_CONNECTING )
//		{
//			ConnInfo * cur_info = static_cast<ConnInfo *>(tcp_api->getMyPtr(current_fd));
//			if (cur_info->numPendingResponses < target_load_for_us_socket)
//			{
//				selected_fd = current_fd;
//				selected_cinfo = cur_info;
//				break;
//			}
//			else if (fd_index == socket_cap - 1
//				/* (implicitly) && target_load_for_us_socket <= cur_info->numPendingResponses */)
//			{
//				selected_fd = current_fd;
//				selected_cinfo = cur_info;
//				target_load_for_us_socket++;
//				break;
//			}
//		}
//
//		fd_index++;
//	}
//
//	if (selected_fd == US_SOCK_NONE)
//	{ // then no upstream sockets are connected yet
//		return;
//	}
//
//	// send a request on the socket
//	httptRequestMessage * us_request = check_and_cast<httptRequestMessage *>(pendingUpstreamRequests.pop());
//	selected_cinfo->numPendingResponses++;
//	LOG_DEBUG("Requesting from server: "<<us_request->heading()<<" on socket: "<<socket_id);
//	tcp_api->send(socket_id, us_request);
//
//	// if this is the first request sent on the socket then signal it to recv
//	if (selected_cinfo->numPendingResponses == 1)
//	{
//		tcp_api->recv(socket_id, selected_cinfo);
//	}
}

// Receive a response containing move data from an upstream cache or server.
void WebCacheNewAPI::processUpstreamResponse(int socket_id, cPacket * msg, /* TODO make cosnt ? */ ConnInfo * data) {

	httptReplyMessage * reply = check_and_cast<httptReplyMessage *>(msg); // used to be only a dynamic_cast

	if (TRACK_HTTP_MESSAGES)
	{
		http_msg_ev_datagram.setMessage(msg);
		http_msg_ev_datagram.setInterfaceID(socket_id);
		emit(http_msg_ev_signal, &http_msg_ev_datagram);
	}

//	if (!reply) {
//		LOG_DEBUG("Message is not an httptReply!");
//		closeSocket(socket_id);
//		currentSocketsOpenToServer--;
//		return;
//	}
	logResponse(reply);

	// calculate the txdelay
	URIVarientSimTimeMap::iterator utx_itr = upstream_txstart_map.find(URIVarientKey(reply->relatedUri(), DEFAULT_URI_VARIENT));
	if (utx_itr != upstream_txstart_map.end())
	{
		simtime_t txdelay = simTime() - utx_itr->second.time;
		upstreamRequestTxdelay.setValue(txdelay);
		emit(txdelay_signal, &upstreamRequestTxdelay);
//		upstream_txdelay_vector->record(txdelay);
//		upstream_txdelay_histogram->collect(txdelay);
		upstream_txstart_map.erase(utx_itr);
	}

	if (!isErrorMessage(reply)) {
		// determine whether the resource should be/can be added to the cache
		string uri = extractURLFromResponse(reply);
		Resource * wr = new WebResource(uri,reply->getByteLength(), reply->contentType(), reply->payload());

		bool added = false;
		string ext = parseResourceName(uri)[2];
		if (!shouldFilter || /* implicit shouldFilter && */ contentFilter.containsExtension(ext))
		{
			// verify that there is enough space to store the object
			if (wr->getSize() <= resourceCache->getCapacity()) {
			  resourceCache->add(wr);
			  added = true;
			  LOG_DEBUG("added: "<<wr->getID());
			}
		}
		// else just forward it to waiting clients

		// send a response to each waiting client.
		list<RequestRecord> requests_to_service = pendingDownstreamRequests.getRequestsForResource(wr->getID());
		list<RequestRecord>::iterator it;
		for (it = requests_to_service.begin(); it != requests_to_service.end(); it++)
		{
			respondToClientRequest((*it).interface_id, (*it).request_msg_ptr, wr);
		}

		pendingDownstreamRequests.removeAndDeleteRequestsForResource(wr->getID());
		deleteSafeIf(wr, !added);
	}
	updateDisplay();

	//closeSocket(socket_id); // close the socket to upstream server.
			//Don't close here in case it can be used again right away to request something else from the server
	//currentSocketsOpenToServer--;
	delete reply;
	//DO NOT delete data;
}

// TODO make pointers const?
void WebCacheNewAPI::respondToClientRequest(int socket_id, httptRequestMessage * request, Resource * resource)
{
	ASSERT(request != NULL);
	ASSERT(resource != NULL);

	// generateByteRangeReply handles regular requests as well byte range requests
	httptReplyMessage * reply = generateByteRangeReply(request, resource->getID(), resource->getSize(), resource->getType());
	reply->setPayload(resource->getContent().c_str());
	LOG_DEBUG("sent to client: "<<reply->heading()<<" for resource: "<<reply->relatedUri());

	if (TRACK_HTTP_MESSAGES)
	{
		http_msg_ev_datagram.setMessage(reply);
		http_msg_ev_datagram.setInterfaceID(socket_id);
		emit(http_msg_ev_signal, &http_msg_ev_datagram);
	}

	tcp_api->send(socket_id, reply);
}

bool WebCacheNewAPI::isErrorMessage(httptReplyMessage *msg)
{
	// TODO accept 206 responses?  the cache never makes a request for partial content to its upstream server
	return msg->result() != 200;
}

httptReplyMessage * WebCacheNewAPI::handleGetRequest(httptRequestMessage * msg, string resource_url)
{
	// we may not be able to return a response immediately if it is a cache miss
	return NULL;
}

// A client (or cache) requests a file from me.
// If I have it, send it along.  Otherwise, initiate a request from an upstream host.
void WebCacheNewAPI::processDownstreamRequest(int socket_id, cPacket * msg, ConnInfo * data) {

	httptRequestMessage * request = check_and_cast<httptRequestMessage *>(msg);

	downstreamRequestsReceived.increment();
	emit(reqev_signal, &downstreamRequestsReceived);

	if (TRACK_HTTP_MESSAGES)
	{
		http_msg_ev_datagram.setMessage(msg);
		http_msg_ev_datagram.setInterfaceID(socket_id);
		emit(http_msg_ev_signal, &http_msg_ev_datagram);
	}

	LOG_DEBUG("received request for: "<<request->heading());

	string url = extractURLFromRequest(request);
	Resource * wr_temp = new WebResource(url, 0); // works because the comparator used only looks at the ID not the size
	Resource * wr_incache = resourceCache->has(wr_temp);

	if (wr_incache) {
		resourceCache->renew(wr_incache); // update timestamp on LRU cache.
		hits.increment();
		emit(reqev_signal, &hits);

		respondToClientRequest(socket_id, request, wr_incache);
		delete request;
	} else {
		misses.increment();
		emit(reqev_signal, &misses);
		// request resource, only if it is the first request of its type
		bool isNew = pendingDownstreamRequests.addRequest(socket_id, url, request);
		if (isNew || resend_request_threshold < pendingDownstreamRequests.numberOfClientsAskingForResource(url)) {
			makeUpstreamRequest(request);//ANY_US_SOCKET, request);
			/*
			ConnInfo * us_cinfo = new ConnInfo;
			us_cinfo->sockType = WCST_CLIENT;
//			us_cinfo->ds_request = request->dup();
			us_cinfo->numPendingResponses = 0;

			//Rotate through a specified num of sockets. If not active(the socket has been closed) then open a new socket to take its place.
			int active_fd = activeSockets[cur_socket];
			if (active_fd == US_SOCK_NONE)
			{
				openUpstreamSocket(us_cinfo);
				activeSockets[cur_socket] = US_SOCK_CONNECTING;
			}
			else if (active_fd == US_SOCK_CONNECTING)
			{
				// add to an upstream request queue? or just call send on the socket?
			}
			else
			{
				makeUpstreamRequest(active_fd, us_cinfo);
			}

			if (cur_socket+1 < socket_cap)
				cur_socket++;
			else
				cur_socket=0;
			*/
		}
	}

	// ask downstream client for anything else it might send:
	tcp_api->recv(socket_id,data);

	updateDisplay();
	delete wr_temp;

	// DO NOT delete 'request' or 'data', request may be stored in pendingDownstreamRequests,
	// and/or it was deleted in the if clause.  'data' is being used to recv on the downstream socket
}
/**
 * takes a URL from a request message.  for now, this must be a correctly formatted one.
 */
string WebCacheNewAPI::extractURLFromRequest(httptRequestMessage * request) {
	string r_msg = request->uri();
	if (!r_msg.empty())
	{
		int erase_start_index = r_msg.find("#", 0);
		if (erase_start_index != (int) string::npos)
		{
			try
			{
				r_msg = r_msg.erase(erase_start_index);
			}
			catch(exception & e)
			{
				cout << "Problem in extractURLFromRequest: "<<e.what()<<endl;
				throw e;
			}
		}
		if (r_msg == "/")
		{
			r_msg = "root";
		}
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
		int erase_start_index = r_msg.find("#", 0);
		if (erase_start_index != (int) string::npos)
		{
			try
			{
				r_msg = r_msg.erase(erase_start_index);
			}
			catch(exception & e)
			{
				cout << "Problem in extractURLFromResponse: "<<e.what()<<endl;
				throw e;
			}
		}
		if (r_msg == "/")
		{
			r_msg = "root";
		}
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

//int WebCacheNewAPI::openUpstreamSocket(ConnInfo * data) {
//	int fd = tcp_api->socket(this);
//	tcp_api->setTimeout(fd, request_timeout);
//
//	//sockets.insert(fd);
//
//	// find module for server.
//	int connect_port;
//	char szModuleName[127];
//
//	controller->getServerInfo(upstream_server.c_str(),szModuleName,connect_port);
//	LOG_DEBUG("detain it right here");
//	tcp_api->connect(fd , szModuleName, connect_port, (void *) data);
//
//	clientSocketsOpened++;
//	currentSocketsOpenToServer++;
//
//	return fd;
//}

void WebCacheNewAPI::closeSocket(int socket_id) {
	//std::set<int>::iterator i = sockets.find(socket_id);
	ConnInfo * data = static_cast<ConnInfo *>(tcp_api->getMyPtr(socket_id));
	if (data) {
		delete data; // TODO remove it from the map as well
	}
	pendingDownstreamRequests.removeAndDeleteAllRequestsOnInterface(socket_id);
	tcp_api->close(socket_id);
	//sockets.erase(i);
	// POINT OF INTEREST
//	for(int i=0; i<socket_cap; i++){
//		if(upstreamSocketDescriptors[i]==socket_id){
//			upstreamSocketDescriptors[i] = -1;
//			return;
//		}
//	}
}

void WebCacheNewAPI::updateDisplay() {
	if ( ev.isGUI() && resourceCache)
	{
		//httptServerBase::updateDisplay(); // DON'T do this if you don't want ugly arrows
		char buf[1024];
		float h = 0;
		unsigned long dsReqsReceived = downstreamRequestsReceived.unsignedLongValue();
		if (dsReqsReceived > 0)
			h = 100.0 * hits.unsignedLongValue() / dsReqsReceived;
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
		sprintf( buf, "Req: %ld\nHit: %.1f \% \nCap: %.1f %s \nFull: %.1f \%", downstreamRequestsReceived.unsignedLongValue(),h,
				capacity, unitstr.c_str(), full);
		getParentModule()->getDisplayString().setTagArg("t",0,buf);
	} /*else if (ev.isGUI() ){
		httptServerBase::updateDisplay();
	}*/
}
