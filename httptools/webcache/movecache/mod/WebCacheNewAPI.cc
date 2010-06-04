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

Define_Module(WebCacheNewAPI);

WebCacheNewAPI::WebCacheNewAPI() {
	resourceCache = NULL;
}

WebCacheNewAPI::~WebCacheNewAPI() {
	  delete resourceCache;
}
void WebCacheNewAPI::initialize() {
	httptServerBase::initialize();

    controller = dynamic_cast<httptController*>(getParentModule()->getParentModule()->getSubmodule("controller")); // not sure if this is it
	if (controller == NULL) {
		error("Controller module not found");
	}
	upstream_cache = par("serverwww").stringValue();
	request_timeout = par("request_timeout");

	resourceCache = new LRUCache(par("cacheSize"));
	updateDisplay();
	serverSocketsBroken=0;
	serverSocketsOpened=0;
	clientSocketsBroken = 0;
	clientSocketsOpened = 0;
	hits = 0;
	misses = 0;
	WATCH(numBroken);
	WATCH(socketsOpened);

	// get socket api
	std::string api_obj_name = par("socketapi").stringValue();
	if (api_obj_name.empty()) {
		opp_error("TCPSAPIGenericSrvApp::initialize(): no tcp socket api specified!");
	}
	tcp_api = check_and_cast<TCPSocketAPI *>(getParentModule()->getSubmodule(api_obj_name.c_str()));
    cMessage * start = new cMessage("START",START);
    scheduleAt(simTime(),start);
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
	if (msg->isSelfMessage()){
    	//opp_error("WebCacheNewAPI::handleMessage(): received a non self message");
    }
	httptServerBase::handleMessage(msg);
}

bool WebCacheNewAPI::hasCallback(TCPSocketAPI::CALLBACK_TYPE type){
	return (type == TCPSocketAPI::CB_T_CONNECT ||
			type == TCPSocketAPI::CB_T_RECV ||
			type == TCPSocketAPI::CB_T_ACCEPT);
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

	// signal next accept
	tcp_api->accept(socket_id);

	switch(ret_status) {
		case TCPSocketAPI::CB_E_UNKNOWN:
			opp_error("WebCacheNewAPI::acceptCallback: unknown error.");
			break;
		default: // read data from new socket
			ConnInfo * ci = new ConnInfo();
			ci->sockType = SERVER;
			tcp_api->recv(ret_status, ci);
			break;
	}
}

// @param socket_id -- the id of the connected socket
// @param ret_status -- the status of the previously invoked connect method
// @param yourPtr -- the pointer to the data passed to the connect method
void WebCacheNewAPI::connectCallback(int socket_id, int ret_status, void * myPtr){

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

	// recv is called on downstream connections to receive requests,
	// and it is called on upstream connections to receive the replies for requests.


	ConnInfo * data = static_cast<ConnInfo *>(myPtr);
	if (!data) {
		cout << "WebCacheNewAPI::recvCallback: No watch data returned!";
		closeSocket(socket_id);
		return;
	}

	bool actAsClient = data->sockType == CLIENT;

	switch(ret_status) {
	case TCPSocketAPI::CB_E_TIMEOUT:
		handleTimeout(socket_id);
		break;
	case TCPSocketAPI::CB_E_UNKNOWN:
		// do nothing special
		break;
	case TCPSocketAPI::CB_E_CLOSED:
		closeSocket(socket_id);
		break;
	default: // positive # of bytes received
		if (actAsClient)
			processUpstreamResponse(socket_id, msg, data);
		else
			processDownstreamRequest(socket_id, msg, data);
		break;
	}
}

/**
 * Sends a request to an upstream cache or server.
 * @param ds_request the request received from a client downstream.
 */
void WebCacheNewAPI::makeUpstreamRequest(int socket_id, ConnInfo * data) {

	httptRequestMessage * ds_request = data->ds_request;
	httptRequestMessage * us_request = ds_request->dup();
	us_request->setTargetUrl(par("serverwww"));
	us_request->setOriginatorUrl(ds_request->targetUrl());

	ConnInfo * ci =new ConnInfo;
	ci->sockType = CLIENT;

	tcp_api->send(socket_id,us_request);
	tcp_api->recv(socket_id,ci);
	delete data;
}

// Receive a response containing move data from an upstream cache or server.
void WebCacheNewAPI::processUpstreamResponse(int socket_id, cPacket * msg, ConnInfo * data) {

	httptReplyMessage * reply = dynamic_cast<httptReplyMessage *>(msg);

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
		list<int> recipients = pendingRequests.clientsAskingForResource(wr->getID());
		list<int>::iterator it;
		for (it = recipients.begin(); it != recipients.end(); it++) {
			httptReplyMessage *cliReply = reply->dup();
			//cliReply->setTargetUrl(rm->originatorUrl());  // maybe not what httptserver does.
			cliReply->setOriginatorUrl(par("www"));
			tcp_api->send(*it, cliReply);
		}
		pendingRequests.removeRequestsForResource(wr->getID());
	}
	updateDisplay();
	closeSocket(socket_id); // close the socket to upstream server.
	delete reply;
	delete data;
}
bool WebCacheNewAPI::isErrorMessage(httptReplyMessage *msg)
{
	return msg->result() != 200;
}
// A client (or cache) requests a file from me.
// If I have it, send it along.  Otherwise, initiate a request from an upstream host.
void WebCacheNewAPI::processDownstreamRequest(int socket_id, cPacket * msg, ConnInfo * data) {

	httptRequestMessage * request = dynamic_cast<httptRequestMessage *>(msg);
	if (!request) {
		opp_error("WebCacheNewAPI::processDownstreamRequest: msg not a httptRequest.");
	}
	requestsReceived++;

	string url = extractURLFromRequest(request);
	Resource * wr = new WebResource(url, 0);
	if (resourceCache->has(wr)) {
		resourceCache->renew(wr); // update timestamp on LRU cache.
		hits++;

		// call the message handler to process the message.
		cMessage *reply = handleReceivedMessage(msg);
		if (reply)
		{
			tcp_api->send(socket_id, reply);
		} else {
			opp_error("WebCacheNewAPI::processDownstreamRequest: handleReceivedMessage returns NULL");
		}
	} else {
		misses++;
		// request resource, only if it is the first request of its type
		bool isNew = pendingRequests.addRequest(socket_id, url);
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
	delete request;
	delete wr;
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
		httptServerBase::updateDisplay();
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
	} else if (ev.isGUI() ){
		httptServerBase::updateDisplay();
	}
}
