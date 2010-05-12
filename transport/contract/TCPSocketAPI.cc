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

TCPSocketAPI::TCPSocketAPI() : _socket_map(), _accept_callbacks(), _resolver() {
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

int TCPSocketAPI::socket () {
	TCPSocket * socket = new TCPSocket();
	socket->setOutputGate(gate("tcpOut"));
	socket->setCallbackObject(this, NULL);
	_socket_map.addSocket(socket);
	return socket->getConnectionId();
}


void TCPSocketAPI::bind (int socket_id, std::string local_address,
		int local_port) {

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

void TCPSocketAPI::connect (int socket_id, std::string remote_address,
		int remote_port, void * yourPtr, SOCK_CB ) {

	// verifies that socket exists
	TCPSocket * socket = findAndCheckSocket(socket_id, "connect()");

	socket->setCallbackObject(this, makeCallbackData(socket_id, callback_function, yourPtr, CONNECT));

	socket->connect(_resolver.resolve(remote_address.c_str(),
			IPAddressResolver::ADDR_PREFER_IPv4), remote_port);
}

void TCPSocketAPI::listen (int socket_id) {
	TCPSocket * socket = findAndCheckSocket(socket_id, "listen()");

	// creates a forking socket
	socket->listen();
}

void TCPSocketAPI::accept (int socket_id, void * yourPtr, SOCK_CB) {
	TCPSocket * socket = findAndCheckSocket(socket_id, "accept()");

	if (socket->getState() != TCPSocket::LISTENING)
	{
		opp_error("TCPSocketAPI::accept() : socket is not a passive socket");
	}


	//socket->setCallbackObject(this, makeCallbackData(socket_id, callback_fucntion, yourPtr, ACCEPT));
	//_listening_sockets.add(socket->getLocalPort(), socket);

	_accept_callbacks[socket->getLocalPort()] =
			makeCallbackData(socket_id, callback_function, yourPtr, ACCEPT);
}

//void TCPSocketAPI::accept(int socket_id, void * yourPtr) {
//	TCPSocket * socket = findAndCheckSocket(socket_id, "recv()");
//	std::map<int, CallbackData *>::iterator i = _registered_callbacks.find(socket_id);
//
//	if (i == _registered_callbacks.end() || i->second->type != ACCEPT)
//	{
//		opp_error("TCPSocketAPI::accept(): no accept callback registered");
//	}
//
//	i->second->function_data = yourPtr;
//
//	socket->setCallbackObject(this, i->second);
//}
//
//void TCPSocketAPI::registerAcceptCallback(int socket_id, SOCK_CB) {
//	TCPSocket * socket = findAndCheckSocket(socket_id, "registerAcceptCallback");
//	registerCallbackData(socket, makeCallbackData(callback_function, NULL, ACCEPT));
//}
//
//void TCPSocketAPI::registerDefaultAcceptCallback(SOCK_CB) {
//	_default_accept_callback = makeCallbackData(callback_function, NULL, ACCEPT);
//}

void TCPSocketAPI::send (int socket_id, cMessage * msg) {
	TCPSocket * socket = findAndCheckSocket(socket_id, "send()");
	socket->send(msg);
}

void TCPSocketAPI::recv (int socket_id, void * yourPtr, SOCK_CB) {
	TCPSocket * socket = findAndCheckSocket(socket_id, "recv()");
	socket->setCallbackObject(this, makeCallbackData(socket_id, callback_function, yourPtr, RECV));
}

//void TCPSocketAPI::recv (int socket_id, void * yourPtr) {
//	TCPSocket * socket = findAndCheckSocket(socket_id, "recv()");
//	std::map<int, CallbackData *>::iterator i = _registered_callbacks.find(socket_id);
//
//	if (i == _registered_callbacks.end())
//	{
//		if (_default_recv_callback)
//			registerCallbackData(socket, )
//		opp_error("TCPSocketAPI::recv(): no recv callback registered");
//	}
//
//	i->second->function_data = yourPtr;
//
//	socket->setCallbackObject(this, i->second);
//}
//
//void TCPSocketAPI::registerRecvCallback (int socket_id, SOCK_CB) {
//	TCPSocket * socket = findAndCheckSocket(socket_id, "registerRecvCallback()");
//	registerCallbackData(socket, makeCallbackData(callback_function, NULL, RECV));
//}
//
//void TCPSocketAPI::registerDefaultRecvCallback(SOCK_CB) {
//	_default_recv_callback = makeCallbackData(callback_function, NULL, RECV);
//}

void TCPSocketAPI::close (int socket_id) {
	TCPSocket * socket = findAndCheckSocket(socket_id, "close()");

	socket->close();
	//_socket_map.removeSocket(socket);
	// delete the socket in the socketClosed callback
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

	switch (cbdata->type){
	case CONNECT:
		cbdata->callback_function(connId, 0, NULL, cbdata->function_data);
		break;
	case ACCEPT:
		cbdata->callback_function(cbdata->socket_id, connId, NULL, cbdata->function_data);
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
		cbdata->callback_function(connId, msg->getByteLength(), msg, cbdata->function_data);
		break;
	case CONNECT:
	case ACCEPT:
	default:
		EV_WARNING << "TCPSocketAPI::socketEstablished(): CONNECT or ACCEPT callback received";
	}

	// delete the message?
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
		cbdata->callback_function(connId, 0, NULL, cbdata->function_data);
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
//	if ( yourPtr==NULL )
//	{
//		EV_ERROR << "TCPSocketAPI::socketClosed(): no callback data";
//		return;
//	}

	// Cleanup
	TCPSocket * socket = _socket_map.removeSocket(connId);
	delete socket;
}

void TCPSocketAPI::socketFailure(int connId, void *yourPtr, int code)
{
	EV_WARNING << "connection broken. Connection id " << connId;
//	numBroken++;

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
		cbdata->callback_function(connId, -1, NULL, cbdata->function_data);
		break;
	case CONNECT:
		cbdata->callback_function(connId, -1, NULL, cbdata->function_data);
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

CallbackData * TCPSocketAPI::makeCallbackData(int socket_id, SOCK_CB,
		void * function_data, CALLBACK_TYPE type) {

	CallbackData * cbdata = new CallbackData();
	cbdata->socket_id = socket_id;
	cbdata->callback_function = callback_function;
	cbdata->function_data = function_data;
	cbdata->type = type;
	return cbdata;
}

//void TCPSocketAPI::registerCallbackData (TCPSocket * socket, CallbackData * cbdata) {
//		_registered_callbacks.add(socket->getConnectionId, cbdata);
//}
