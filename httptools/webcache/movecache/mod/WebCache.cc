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

/**
 * The web cache behaves as a server to move clients and acts as a client to the
 * Video servers.
 */

#include "WebCache.h"
Define_Module(WebCache);

WebCache::WebCache() {

}
WebCache::~WebCache() {
  delete resourceCache;
  ClientMap::iterator it;
  for (it = pendingRequests.begin(); it != pendingRequests.end(); it++) {
	  delete (*it).second;
  }
}

void WebCache::initialize() {
	EV_DEBUG << "WEBCACHE: initialize begin"<< endl;
	httptServerBase::initialize();
	EV_DEBUG << "Initializing cache" << endl;
    int port = par("port");
    TCPSocket * listensocket = new TCPSocket();
    CACHE_SOCK_DS *csd = new CACHE_SOCK_DS;
    csd->sockType = LISTENER;
    csd->socket = listensocket;
    listensocket->setOutputGate(gate("tcpOut"));
    listensocket->bind(port);
	listensocket->setCallbackObject(this,csd);
    listensocket->listen();

	serverSocketsBroken=0;
	serverSocketsOpened=0;

	clientSocketsBroken = 0;
	clientSocketsOpened = 0;
	hits = 0;
	misses = 0;
	WATCH(numBroken);
	WATCH(socketsOpened);

	resourceCache = new LRUCache(par("cacheSize"));
	updateDisplay();
	//EV_INFO << "INITIALIZING LRUCache OF SIZE "<<par("cacheSize")<<endl;
	controller = dynamic_cast<httptController*>(getParentModule()->getParentModule()->getSubmodule("controller")); // not sure if this is it
  if (controller == NULL) {
    error("Controller module not found");
  }
}
void WebCache::finish() {
	EV_DEBUG << "WEBCACHE: finish begin"<< endl;
	// Call the parent class finish. Takes care of most of the reporting.
	//httptServerBase::finish();

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

	// @todo Delete socket data structures........
	sockCollection.deleteSockets();
}

void WebCache::handleMessage(cMessage * msg) {
	EV_DEBUG << "WEBCACHE: handleMessage begin"<< endl;
    if (msg->isSelfMessage())
	{
		// Self messages not used at the moment
	}
//	else if (meantForServer(msg))
//	{
		EV_DEBUG << "Handle inbound (SERVER) message " << msg->getName() << " of kind " << msg->getKind() << endl;
		TCPSocket *socket = sockCollection.findSocketFor(msg);
		 /// Warning: if socket is null, we're assuming that it cannot be of type client.
		if (!socket && strcmp(msg->getFullName(),"PEER_CLOSED"))
		{
			EV_DEBUG << "No socket found for the message. Create a new one, assuming it is supposed to be of type server." << endl;
			// new connection -- create new socket object and server process
			socket = new TCPSocket(msg);
			serverConnections.insert(socket->getConnectionId());
			socket->setOutputGate(gate("tcpOut"));
			CACHE_SOCK_DS *csd = new CACHE_SOCK_DS;
			csd->sockType = SERVER;
			csd->socket = socket;
			socket->setCallbackObject(this,csd);
			sockCollection.addSocket(socket);
		}
		if (socket == NULL) {
			delete msg;
			return;
		}
		EV_DEBUG << "Process the message " << msg->getName() << endl;
		socket->processMessage(msg);
/*	}
	else // meant for client
	{
		EV_DEBUG << "Handle message for (CLIENT) received: " << msg->getName() << endl;

		// Locate the socket for the incoming message. One should definitely exist.
		TCPSocket *socket = sockCollection.findSocketFor(msg);
		if ( socket==NULL )
		{
			// Handle errors. @todo error instead of warning?
			EV_WARNING << "No socket found for message " << msg->getName() << endl;
			delete msg;
			return;
		}
		// Submit to the socket handler. Calls the TCPSocket::CallbackInterface methods.
		// Message is deleted in the socket handler
		socket->processMessage(msg);
	}
	*/
	httptServerBase::handleMessage(msg);
}

/**
 * This makes an attempt at guessing whether an incoming message is to be handled as a
 * client message or a server message.  Our assumption is that any client message will have
 * control info in the message, and if so, its connection id will not be in the set of serverConnections.
 * It could be that a client message does not have control info.  If so, this should be identified as
 * a possible reason for a bug.
 */
bool WebCache::meantForServer(cMessage * msg) {
	TCPCommand *ind = dynamic_cast<TCPCommand *>(msg->getControlInfo());
	if (!ind)
	{
		//EV_DEBUG << "No control info for the message" << endl;
		return true;
	}
	int connId = ind->getConnId();
	EV_DEBUG << "Connection ID: " << connId << endl;
	TSet::iterator it = serverConnections.find(connId);
	if (it != serverConnections.end()) {
		return true;
	}
	return false;
}
void WebCache::socketEstablished(int connId, void *yourPtr)
{
	EV_DEBUG << "WEBCACHE: socketEstablished begin"<< endl;
    EV_INFO << "connected socket with id=" << connId << endl;
    if ( yourPtr==NULL )
    {
      EV_ERROR << "SocketEstablished failure. Null pointer" << endl;
      return;
    }
    CACHE_SOCK_DS * csd = (CACHE_SOCK_DS *)yourPtr;
    if (csd->sockType == SERVER) {
      serverSocketsOpened++;
    } else if (csd->sockType == CLIENT) {
      clientSocketsOpened++;
      TCPSocket * socket = csd->socket;
      if ( csd->messageQueue.size()==0 )
      {
        EV_INFO << "No data to send on client socket with connection id " << connId << ". Closing" << endl;
       	socket->close();
        return;
      }
      // Send pending messages on the established socket.
      cMessage *msg;
      EV_DEBUG << "Proceeding to send messages on socket " << connId << endl;
      while( csd->messageQueue.size()!=0 )
      {
        msg = csd->messageQueue.back();
        cPacket *pckt = check_and_cast<cPacket *>(msg);
        csd->messageQueue.pop_back();
        EV_DEBUG << "Submitting request " << msg->getName() << " to socket " << connId << ". size is " << pckt->getByteLength() << " bytes" << endl;
        socket->send(msg);
        csd->pendingReplies++;
      }
    }

}
void WebCache::socketDataArrived(int connId, void * yourPtr, cPacket * msg, bool urgent) {
	EV_DEBUG << "WEBCACHE: socketDataArrived begin"<< endl;
	if ( yourPtr==NULL )
	{
		EV_ERROR << "Socket establish failure. Null pointer" << endl;
		return;
	}
	CACHE_SOCK_DS * sockdata = (CACHE_SOCK_DS *)yourPtr;
	TCPSocket *socket = sockdata->socket;
  if (sockdata->sockType == SERVER) {
    // Should be a httptReplyMessage
    EV_DEBUG << "Socket data arrived on connection " << connId << ". Message=" << msg->getName() << ", kind=" << msg->getKind() << endl;
    requestsReceived++;
    updateDisplay();
    if (serverHasResource(msg) == true) {
      // call the message handler to process the message.
      cMessage *reply = handleReceivedMessage(msg);
      if ( reply!=NULL )
      {
        socket->send(reply); // Send to socket if the reply is non-zero.
      }
      delete msg; // Delete the received message here. Must not be deleted in the handler!
    } else {
      // request resource
      httptRequestMessage * m = generateServerRequest(msg);
      TCPSocket * srv_socket = sendRequest(m);
      // log sockets;
      // TODO: when move client uses same socket for subseq. requests,
      // don't create new server socket for it.
      pendingRequests.insert(pair<TCPSocket *, cPacket *>(srv_socket, msg));
    }
  } else if (sockdata->sockType == CLIENT) {
    //handleDataMessage(msg);
    receiveResource(msg); // only one type of resource: move content.
    updateDisplay();

    if ( --sockdata->pendingReplies==0 )
    {
      EV_DEBUG << "Received last expected reply on this socket. Issing a close" << endl;
     	  socket->close();
	}
    // Message deleted in handler - do not delete here!
  }
}
void WebCache::socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status)
{
  // This is obviously not used at the present time.
  EV_INFO << "SOCKET STATUS ARRIVED. Socket: " << connId << endl;
}


void WebCache::socketPeerClosed(int connId, void *yourPtr)
{
	EV_DEBUG << "WEBCACHE: socketPeerClosed begin"<< endl;
	if ( yourPtr==NULL )
	{
		EV_ERROR << "Socket establish failure. Null pointer" << endl;
		return;
	}
	CACHE_SOCK_DS * csd = (CACHE_SOCK_DS *)yourPtr;
	TCPSocket *socket = csd->socket;

    // close the connection (if not already closed)
    if ( socket->getState()==TCPSocket::PEER_CLOSED)

    {
        EV_INFO << "remote TCP closed, closing here as well. Connection id is " << connId << endl;
        socket->close();  // Call the close method to properly dispose of the socket.
    }
}

void WebCache::socketClosed(int connId, void *yourPtr)
{
	EV_DEBUG << "WEBCACHE: socketClosed begin"<< endl;
    EV_INFO << "connection closed. Connection id " << connId << endl;

	if ( yourPtr==NULL )
	{
		EV_ERROR << "Socket establish OR socket closed failure. Null pointer" << endl;
		return;
	}
	// Cleanup
	CACHE_SOCK_DS * csd = (CACHE_SOCK_DS *)yourPtr;
	TCPSocket *socket = csd->socket;

	sockCollection.removeSocket(socket);
	delete socket;
	delete csd;
}

void WebCache::socketFailure(int connId, void *yourPtr, int code)
{
	EV_DEBUG << "WEBCACHE: socketFailure begin"<< endl;
    EV_WARNING << "connection broken. Conneciton id " << connId << endl;
    if ( yourPtr==NULL )
    {
      EV_ERROR << "Socket establish or other failure. Null pointer" << endl;
      return;
    }
    CACHE_SOCK_DS * csd = (CACHE_SOCK_DS *)yourPtr;
    TCPSocket *socket = csd->socket;
    if (csd->sockType == SERVER) {
      serverSocketsBroken++;
    }
    else if (csd->sockType == CLIENT) {
      clientSocketsBroken++;
    }

    EV_INFO << "connection closed. Connection id " << connId << endl;


	if (code==TCP_I_CONNECTION_RESET)
		EV_WARNING << "Connection reset!\\n";
	else if (code==TCP_I_CONNECTION_REFUSED)
		EV_WARNING << "Connection refused!\\n";

	// Cleanup
	sockCollection.removeSocket(socket);
	delete socket;
	delete csd;
}
/**
 * Indicates whether a cacheable resource is stored in the server.
 * Returns true if the resource is non-cacheable or if the resource is
 * in the cache, and false otherwise.
 */
bool WebCache::serverHasResource(cPacket * msg) {
	EV_DEBUG << "WEBCACHE: serverHasResource begin"<< endl;
  // check if file is in cache.
  httptRequestMessage *request = check_and_cast<httptRequestMessage *>(msg);
  if (request==NULL) error("Message (%s)%s is not a valid request", msg->getClassName(), msg->getName());
  string url = extractURLFromRequest(request);
  Resource * wr = new WebResource(url, 0);
  Resource * existing = NULL;
  if ((existing = resourceCache->has(wr) )!= NULL) {
	    EV_INFO <<"CACHE HIT FOR RESOURCE: "<<wr->getID()<<endl;
	    //cout <<"CACHE HIT FOR RESOURCE: "<<wr->getID()<<endl;
	    resourceCache->add(existing); // changes timestamp on it.
	    hits++;
	    delete wr;
	    return true;
  }
  else {
	  EV_INFO <<"CACHE MISS FOR RESOURCE: "<<wr->getID()<<endl;
	  //cout <<"CACHE MISS FOR RESOURCE: "<<wr->getID()<<endl;
	  misses++;
	  delete wr;
	  return false;
  }
}
/**
 * takes a URL from a request message.  for now, this must be a correctly formatted one.
 */
string WebCache::extractURLFromRequest(httptRequestMessage * request) {
	cStringTokenizer tokenizer = cStringTokenizer(request->heading()," ");
	 vector<string> res = tokenizer.asVector();
	 string r_msg = res[1];
	 if (!strcmp(r_msg.c_str(), "/")) {
		 r_msg = "root";
	 }
	 r_msg = trimLeft(r_msg, "/");
	//cout<<"EXTRACTED From request: "<<r_msg<<endl;
	 return r_msg;
}
string WebCache::extractURLFromResponse(httptReplyMessage * response) {
	cStringTokenizer tokenizer = cStringTokenizer(response->getFullName(),"()");
	 vector<string> res = tokenizer.asVector();
	 //cout<<"RESULT LENGTH: "<<res.size()<<endl;
	 //cout<<"Imported: "<<res[1]<<endl;

	 string resp = res[1];

	 return resp;
}
/**
 * Sends a request to a move server.  The request is based on the
 * request it got from a move client.
 */
TCPSocket * WebCache::sendRequest(httptRequestMessage * request) {
	EV_DEBUG << "WEBCACHE: sendRequest begin"<< endl;
  int connectPort;
  char szModuleName[127];

  if ( controller->getServerInfo(request->targetUrl(),szModuleName,connectPort) != 0 )
  {
    EV_ERROR << "Unable to get server info for URL " << request->targetUrl() << endl;
    return NULL;
  }
  EV_DEBUG << "Sending request to server " << request->targetUrl() << " (" << szModuleName << ") on port " << connectPort << endl;
  return submitToSocket(szModuleName,connectPort,request);
}

/**
 * based on a message containing a request from a move client, generate a
 * httptRequestMessage that will ask a move server where data is.
 */
httptRequestMessage * WebCache::generateServerRequest(cPacket * msg) {
  httptRequestMessage *request = check_and_cast<httptRequestMessage *>(msg);
  if (request==NULL) error("Message (%s)%s is not a valid request", msg->getClassName(), msg->getName());
  httptRequestMessage * request2 = new httptRequestMessage(*request);
  request2->setTargetUrl(par("serverwww"));
  request2->setOriginatorUrl(request->targetUrl());
  return request2;
}

/**
 * Received a response from a move server; obtain the data,
 * update cache, and then send it to the move client.
 */
void WebCache::receiveResource(cPacket * msg) {

	EV_DEBUG << "WEBCACHE: receiveResource begin"<< endl;
  httptReplyMessage *appmsg = check_and_cast<httptReplyMessage*>(msg);
  if (appmsg==NULL) error("Message (%s)%s is not a valid reply message", msg->getClassName(), msg->getName());
  logResponse(appmsg);

  // update cache
  if (!isErrorMessage(appmsg)) {
	  Resource * wr = new WebResource(extractURLFromResponse(appmsg),appmsg->getByteLength());

	  if (wr->getSize() <= ((int)par("cacheSize"))) {
		  EV_INFO << "CACHE ADDING RESOURCE: "<<wr->getID() <<".  REMAINING CAPACITY: "<<((LRUCache *)resourceCache)->getRemainingCapacity()<<endl;
		  resourceCache->add(wr);
	  }
	  else {
		  EV_DEBUG<<"Resource not added to cache because bigger than cache capacity"<<endl;
	  }
  }
  else {
	  EV_DEBUG<<"Resource not added to cache it is an error message."<<endl;
  }
  // send message to approp. client
  httptReplyMessage *appmsg2 = new httptReplyMessage(*appmsg); // use dup() here?

  TCPSocket * srvr = sockCollection.findSocketFor(msg);
  ClientMap::iterator i = pendingRequests.find(srvr);
  cPacket * m = i->second;
  TCPSocket * cli = sockCollection.findSocketFor(m);
  if (!cli || cli->getState() == 7) { // client socket was closed or f ailed earlier
	  EV_INFO<<"Cache can not send to client, socket failed or was closed"<<endl;

  }
  else{
	  httptRequestMessage * rm= check_and_cast<httptRequestMessage *>(m);
	  appmsg2->setTargetUrl(rm->originatorUrl());  // maybe not what httptserver does.
	  appmsg2->setOriginatorUrl(par("www"));
	  cli->send(appmsg2);
  }
  delete msg;
}

bool WebCache::isErrorMessage(httptReplyMessage *msg)
{
	return msg->result() != 200;
}


TCPSocket * WebCache::submitToSocket( const char* moduleName, int connectPort, cMessage *msg ) {
	// Create a queue and push the single message
	MESSAGE_QUEUE_TYPE queue;
	queue.push_back(msg);
	// Call the overloaded version with the queue as parameter
	return submitToSocket(moduleName,connectPort,queue);
}
TCPSocket * WebCache::submitToSocket( const char* moduleName, int connectPort, MESSAGE_QUEUE_TYPE &queue ) {
	// Dont do anything if the queue is empty.s
	if ( queue.size()==0 )
	{
		EV_INFO << "Submitting to socket. No data to send to " << moduleName << ". Skipping connection." << endl;
		return NULL;
	}
	EV_DEBUG << "Submitting to socket. Module: " << moduleName << ", port: " << connectPort << ". Total messages: " << queue.size() << endl;

	// Create and initialize the socket
	TCPSocket *socket = new TCPSocket();
	socket->setOutputGate(gate("tcpOut"));
	sockCollection.addSocket(socket);

	// Initialize the associated datastructure
	CACHE_SOCK_DS * sockdata = new CACHE_SOCK_DS;
	sockdata->sockType = CLIENT;
	sockdata->messageQueue = MESSAGE_QUEUE_TYPE(queue);
	sockdata->socket=socket;
	sockdata->pendingReplies=0;
	socket->setCallbackObject(this,sockdata);

	// Issue a connect to the socket for the specified module and port.
  socket->connect(IPAddressResolver().resolve(moduleName), connectPort);
  return socket;
}
void WebCache::updateDisplay() {
	if ( ev.isGUI() && resourceCache)
	{
		httptServerBase::updateDisplay();
		char buf[1024];
		float h = 0;
		if (requestsReceived > 0)
			h = 100.0 * hits / requestsReceived;
		int cacheSize = (int)par("cacheSize");
		int remaining = ((LRUCache *)resourceCache)->getRemainingCapacity();
		float full =0;
		if (cacheSize > 0)
			full = 100.0 * (cacheSize-remaining) /cacheSize;
		sprintf( buf, "Req: %ld\nHit: %.1f\%\nCap: %.1fKB\nFull: %.1f\%", requestsReceived,h,cacheSize/1000.0, full);
		getParentModule()->getDisplayString().setTagArg("t",0,buf);
	} else if (ev.isGUI() ){
		httptServer::updateDisplay();
	}

}

