// Author: Kevin Black
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

#include "TCPSocketAPI.h"

Define_Module(TCPSocketAPI);

TCPSocketAPI::TCPSocketAPI() : _socket_map(), _accept_callbacks(),
	_registered_callbacks(), _resolver() {
//	_default_recv_callback = NULL;
//	_default_accept_callback = NULL;
}

TCPSocketAPI::~TCPSocketAPI() {
	// TODO delete CallbackData objects
	_socket_map.deleteSockets();
}

//==============================================================================
// cSimpleModule functions

void TCPSocketAPI::initialize()
{
	cSimpleModule::initialize();
	EV_DEBUG << "initializing tcp socket api" << endl;

	// other variables, scalars/vectors WATCH calls
}

void TCPSocketAPI::handleMessage(cMessage *msg)
{
	// based off of code in httptServer.cc

	if (msg->isSelfMessage()) {
		// if this class eventually extends EmulationInterface then pass processing to that code
		return;
	}

	EV_DEBUG << "Handle inbound message " << msg->getName() << " of kind " << msg->getKind() << endl;

	TCPSocket *socket = _socket_map.findSocketFor(msg);

	if (!socket)
	{
		EV_DEBUG << "No socket found for the message. Create a new one" << endl;
		// this is to "accept" new connections
		// new connection -- create new socket object and server process
		socket = new TCPSocket(msg);
		socket->setOutputGate(gate("tcpOut"));

		std::map<int, CallbackData *>::iterator i = _accept_callbacks.find(socket->getLocalPort());

		if (i == _accept_callbacks.end())
		{
			EV_WARNING << "TCPSocketAPI::handleMessage(): received message for new connection on non-listening port";
			delete socket; // ignore it
			return;
		}
		socket->setCallbackObject(this, i->second); // assume that the message type is TCP_I_ESTABLISHED
		_socket_map.addSocket(socket);
	}
	EV_DEBUG << "Process the message " << msg->getName() << endl;
	socket->processMessage(msg);

	// update display?
}

void TCPSocketAPI::finish() {
	// record scalars
}

//==============================================================================
// TCP Socket API functions

int TCPSocketAPI::socket (CallbackInterface * cbobj) {
	Enter_Method_Silent();

	if (!cbobj) {
		opp_error("TCPSocketAPI::socket(): no callback object specified");
	}

	TCPSocket * socket = new TCPSocket();
	socket->setOutputGate(gate("tcpOut"));

	int id = socket->getConnectionId();
	_registered_callbacks[id] = makeCallbackData(id, cbobj, NULL, NONE);
	socket->setCallbackObject(this, _registered_callbacks[id]);

	_socket_map.addSocket(socket);
	return id;
}


void TCPSocketAPI::bind (int socket_id, std::string local_address,
		int local_port) {

	Enter_Method_Silent();

	// verifies that socket exists
	TCPSocket * socket = findAndCheckSocket(socket_id, "bind()");

	if (local_address.empty()) {
		//checks socket state and port number
		socket->bind(local_port);
	}
	else {
		//checks socket state and port number
		socket->bind(_resolver.resolve(local_address.c_str(),
				IPAddressResolver::ADDR_PREFER_IPv4), local_port);
	}
}

//void TCPSocketAPI::connect (int socket_id, std::string remote_address,
//		int remote_port, void * yourPtr, CallbackInterface * cbobj ) {
//
//	Enter_Method_Silent();
//
//	// verifies that socket exists
//	TCPSocket * socket = findAndCheckSocket(socket_id, "connect()");
//
//	socket->setCallbackObject(this, makeCallbackData(socket_id, cbobj, yourPtr, CONNECT));
//
//	socket->connect(_resolver.resolve(remote_address.c_str(),
//			IPAddressResolver::ADDR_PREFER_IPv4), remote_port);
//}

void TCPSocketAPI::connect (int socket_id, std::string remote_address,
		int remote_port, void * yourPtr) {

	Enter_Method_Silent();

	// verifies that socket exists
	TCPSocket * socket = findAndCheckSocket(socket_id, "connect()");

	CallbackData * cbdata = _registered_callbacks[socket_id];
	// should not be NULL
	cbdata->function_data = yourPtr;
	cbdata->type = CONNECT;
	//socket->setCallbackObject(this, cbdata);

	socket->connect(_resolver.resolve(remote_address.c_str(),
			IPAddressResolver::ADDR_PREFER_IPv4), remote_port);
}


void TCPSocketAPI::listen (int socket_id, CallbackInterface * cbobj_for_accepted) {
	Enter_Method_Silent();

	TCPSocket * socket = findAndCheckSocket(socket_id, "listen()");

	if (cbobj_for_accepted) {
		CallbackData * cbdata = _registered_callbacks[socket_id];
		cbdata->cbobj_for_accepted = cbobj_for_accepted;
	}

	// creates a forking socket
	socket->listen();
}

int TCPSocketAPI::makeActiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, std::string remote_address, int remote_port, void * yourPtr) {
	Enter_Method_Silent();
	int id = socket(cbobj);
	bind(id, local_address, local_port);
	connect(id, remote_address, remote_port, yourPtr);
	return id;
}

int TCPSocketAPI::makePassiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, CallbackInterface * cbobj_for_accepted=NULL) {
	Enter_Method_Silent();
	int id = socket(cbobj);
	bind(id, local_address, local_port);
	listen(id, cbobj_for_accepted);
	return id;
}

void TCPSocketAPI::accept (int socket_id, void * yourPtr) {

	Enter_Method_Silent();

	TCPSocket * socket = findAndCheckSocket(socket_id, "accept()");

	if (socket->getState() != TCPSocket::LISTENING)
	{
		opp_error("TCPSocketAPI::accept() : socket is not a passive socket");
	}

	CallbackData * cbdata = _registered_callbacks[socket_id];
	cbdata->function_data = yourPtr;
	cbdata->type = ACCEPT;

	_accept_callbacks[socket->getLocalPort()] = cbdata;
}

void TCPSocketAPI::send (int socket_id, cMessage * msg) {

	Enter_Method_Silent();

	TCPSocket * socket = findAndCheckSocket(socket_id, "send()");

	// remove any control info with the message
	cObject * ctrl_info = msg->removeControlInfo();
	if (ctrl_info)
		delete ctrl_info;

	socket->send(msg);
}

void TCPSocketAPI::recv (int socket_id, void * yourPtr) {

	Enter_Method_Silent();

	TCPSocket * socket = findAndCheckSocket(socket_id, "recv()");
	CallbackData * cbdata = _registered_callbacks[socket_id];
	cbdata->function_data = yourPtr;
	cbdata->type = RECV;
}

void TCPSocketAPI::close (int socket_id) {

	Enter_Method_Silent();

	TCPSocket * socket = findAndCheckSocket(socket_id, "close()");

	socket->close();
}

//==============================================================================
// TCPSocket::CallbackInterface functions

void TCPSocketAPI::socketEstablished(int connId, void *yourPtr)
{
	EV_INFO << "connected socket with id=" << connId << endl;
	// update scalar variables

	if ( yourPtr == NULL )
	{
		EV_ERROR << "TCPSocketAPI::socketEstablished(): No callback data with the socket";
		return;
	}

	// invoke the "connect" or "accept" callback
	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);

	CallbackData * new_cbdata = NULL;

	switch (cbdata->type){
	case CONNECT:
		cbdata->cbobj->connectCallback(connId, 0, cbdata->function_data);
		break;
	case ACCEPT:
		// set the spawned socket's callback data to match itself now
		new_cbdata = makeCallbackData(connId, cbdata->cbobj_for_accepted, NULL, NONE);
		_registered_callbacks[connId] = new_cbdata;
		_socket_map.getSocket(connId)->setCallbackObject(this, new_cbdata);

		// notify the passive socket of an accept
		cbdata->cbobj->acceptCallback(cbdata->socket_id, connId, cbdata->function_data);
		break;
	case RECV:
	default:
		EV_WARNING << "TCPSocketAPI::socketEstablished(): RECV callback received";
	}
}

void TCPSocketAPI::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
	if ( yourPtr==NULL )
	{
		EV_ERROR << "TCPSocketAPI::socketDataArrived(): "
					"No callback data with the socket!  Connection failure?";
		return;
	}

	// invoke the recv callback
	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);

	switch (cbdata->type)
	{
	case RECV:
		cbdata->cbobj->recvCallback(connId, msg->getByteLength(), msg, cbdata->function_data);
		break;
	case CONNECT:
	case ACCEPT:
	default:
		EV_WARNING << "TCPSocketAPI::socketEstablished(): CONNECT or ACCEPT callback received";
	}
}

void TCPSocketAPI::socketPeerClosed(int connId, void *yourPtr)
{
	if ( yourPtr==NULL )
	{
		EV_ERROR << "TCPSocketAPI::socketPeerClosed(): no callback data with socket";
		return;
	}
	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);

	// invoke the recv callback or just handle the close operation?
	// assume that the application will close the socket
	switch (cbdata->type)
	{
	case RECV:
		cbdata->cbobj->recvCallback(connId, 0, NULL, cbdata->function_data);
		break;
	case CONNECT:
	case ACCEPT:
	default:
		EV_WARNING << "TCPSocketAPI::socketPeerClosed(): CONNECT or ACCEPT callback received";
	}
}

void TCPSocketAPI::socketClosed(int connId, void *yourPtr)
{
	EV_INFO << "connection closed. Connection id " << connId << endl;

	// okay if yourPtr == NULL

	TCPSocket * socket = _socket_map.removeSocket(connId);
	delete socket;
}

void TCPSocketAPI::socketFailure(int connId, void *yourPtr, int code)
{
	EV_WARNING << "connection broken. Connection id " << connId;

	EV_INFO << "connection closed. Connection id " << connId;

	if ( yourPtr==NULL )
	{
		EV_ERROR << "TCPSocketAPI::socketFailure(): no callback data with socket";
		return;
	}

	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);

	if (code==TCP_I_CONNECTION_RESET)
		EV_WARNING << "Connection reset!\\n";
	else if (code==TCP_I_CONNECTION_REFUSED)
		EV_WARNING << "Connection refused!\\n";

	// invoke the recv callback or just handle the close operation?
	switch (cbdata->type)
	{
	case RECV:
		cbdata->cbobj->recvCallback(connId, -1, NULL, cbdata->function_data);
		break;
	case CONNECT:
		cbdata->cbobj->connectCallback(connId, -1, cbdata->function_data);
	case ACCEPT:
	default:
		EV_WARNING << "TCPSocketAPI::socketPeerClosed(): ACCEPT callback received";
	}

	// Cleanup
	TCPSocket * socket = _socket_map.removeSocket(connId);
	delete socket;
}

//==============================================================================
// UTILITY FUNCTIONS

TCPSocket * TCPSocketAPI::findAndCheckSocket(int socket_id, std::string method) {
	TCPSocket * socket = _socket_map.getSocket(socket_id);
	if (!socket) {
		std::string info = "TCPSocketAPI::"+method+" : invalid socket id";
		opp_error(info.c_str());
	}
	return socket;
}

TCPSocketAPI::CallbackData * TCPSocketAPI::makeCallbackData(int socket_id,
		CallbackInterface * cbobj, void * function_data, CALLBACK_TYPE type) {

	CallbackData * cbdata = new CallbackData();
	cbdata->socket_id = socket_id;
	cbdata->cbobj = cbobj;
	cbdata->function_data = function_data;
	cbdata->type = type;
	cbdata->cbobj_for_accepted = cbobj;
	return cbdata;
}
