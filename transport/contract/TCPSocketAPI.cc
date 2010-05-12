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

TCPSocketAPI::TCPSocketAPI() : _socket_map(), _accept_callbacks() {

}

TCPSocketAPI::~TCPSocketAPI() {

}

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
		// this is to "accept" new connections
		EV_DEBUG << "No socket found for the message. Create a new one" << endl;
		// new connection -- create new socket object and server process
		socket = new TCPSocket(msg);
		socket->setOutputGate(gate("tcpOut"));
		//socket->setCallbackObject(this,NULL);
		//_socket_map.addSocket(socket);

		std::map<int, CallbackData *>::iterator i = _accept_callbacks.find(socket->getLocalPort());
		if (i == _accept_callbacks.end())
		{
			EV_WARNING << "TCPSocketAPI::handleMessage(): received message for new connection on non-listening port";
			// ignore it
			return;
		}
		socket->setCallbackObject(this, i->second); //assume that processMessage will call TCP_I_ESTABLISHED
		_socket_map.addSocket(socket);
	}
	EV_DEBUG << "Process the message " << msg->getName() << endl;
	socket->processMessage(msg);

	// update display?
}

void TCPSocketAPI::finish() {
	// record scalars
}


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
		socket->bind(IPAddressResolver.resolve(local_address), local_port);
	}
}

void TCPSocketAPI::connect (int socket_id, std::string remote_address,
		int remote_port, void * yourPtr, SOCK_CB ) {

	// verifies that socket exists
	TCPSocket * socket = findAndCheckSocket(socket_id, "connect()");

	registerCallbackData(socket, callback_function, yourPtr, CONNECT);

	socket->connect(IPAddressResolver(remove_address), remote_port);
}

void TCPSocketAPI::listen (int socket_id) {
	TCPSocket * socket = findAndCheckSocket(socket_id, "listen()");

	// create a forking socket
	socket->listen(true);
}

void TCPSocketAPI::accept (int socket_id, void * yourPtr, SOCK_CB) {
	TCPSocket * socket = findAndCheckSocket(socket_id, "accept()");

	if (socket->getState() != LISTENING)
	{
		opp_error("TCPSocketAPI::accpet() : socket is not a passive socket");
	}

	// registerCallbackData(socket, callback_function, yourPtr, ACCEPT);
	CallbackData * cbdata = new CallbackData();
	cbdata->callback_function = callback_function;
	cbdata->function_data = function_data;
	cbdata->type = ACCEPT;

	_accept_callbacks.add(socket->getLocalPort(), cbdata);
}

void TCPSocketAPI::send (int socket_id, std::string data) {
	TCPSocket * socket = findAndCheckSocket(socket_id, "send()");
	// construct a cMessage with the data -- use a translator plugin?
	socket->send();
}

void TCPSocketAPI::recv (int socket_id, void * yourPtr, SOCK_CB) {
	TCPSocket * socket = findAndCheckSocket(socket_id, "recv()");

	registerCallbackData(socket, callback_function, yourPtr, RECV);
}

void TCPSocketAPI::close (int socket_id) {
	TCPSocket * socket = findAndCheckSocket(socket_id, "close()");

	_socket_map.removeSocket(socket);
	socket->close();
	// delete the socket in the socketClosed callback
}

void TCPSocketAPI::socketEstablished(int connId, void *yourPtr)
{
	EV_INFO << "connected socket with id=" << connId << endl;
	// update scalar variables socketsOpened++;

	// invoke the "connect" or "accept" callback
}

void TCPSocketAPI::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
	if ( yourPtr==NULL )
	{
		EV_ERROR << "Socket establish failure. Null pointer" << endl;
		return;
	}
	TCPSocket *socket = (TCPSocket*)yourPtr;

	// invoke the recv callback

	// delete the message?
}

void TCPSocketAPI::socketPeerClosed(int connId, void *yourPtr)
{
	if ( yourPtr==NULL )
	{
		EV_ERROR << "Socket establish failure. Null pointer" << endl;
		return;
	}
	TCPSocket *socket = (TCPSocket*)yourPtr;

	// invoke the recv callback? or just handle the close operation?

	// close the connection (if not already closed)
	if (socket->getState()==TCPSocket::PEER_CLOSED)
	{
		EV_INFO << "remote TCP closed, closing here as well. Connection id is " << connId << endl;
		socket->close();  // Call the close method to properly dispose of the socket.
	}
}

void TCPSocketAPI::socketClosed(int connId, void *yourPtr)
{
	EV_INFO << "connection closed. Connection id " << connId << endl;

	if ( yourPtr==NULL )
	{
		EV_ERROR << "Socket establish failure. Null pointer" << endl;
		return;
	}
	// Cleanup
	TCPSocket *socket = (TCPSocket*)yourPtr;
	sockCollection.removeSocket(socket);
	delete socket;
}

void TCPSocketAPI::socketFailure(int connId, void *yourPtr, int code)
{
	EV_WARNING << "connection broken. Connection id " << connId << endl;
	numBroken++;

	EV_INFO << "connection closed. Connection id " << connId << endl;

	if ( yourPtr==NULL )
	{
		EV_ERROR << "Socket establish failure. Null pointer" << endl;
		return;
	}
	TCPSocket *socket = (TCPSocket*)yourPtr;

	if (code==TCP_I_CONNECTION_RESET)
		EV_WARNING << "Connection reset!\\n";
	else if (code==TCP_I_CONNECTION_REFUSED)
		EV_WARNING << "Connection refused!\\n";

	// Cleanup
	sockCollection.removeSocket(socket);
	delete socket;
}

TCPSocket * TCPSocketAPI::findAndCheckSocket(int socket_id, std::string method) {
	TCPSocket * socket = _socket_map.getSocket(socket_id);
	if (!socket) {
		opp_error("TCPSocketAPI::"+method+" : invalid socket id");
	}
	return socket;
}

void TCPSocketAPI::registerCallbackData (TCPSocket * socket, SOCK_CB,
		void * function_data, CALLBACK_TYPE type) {

	CallbackData * cbdata = new CallbackData();
	cbdata->callback_function = callback_function;
	cbdata->function_data = function_data;
	cbdata->type = type;

	socket->setCallbackObject(this, cbdata);
}
