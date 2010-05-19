// Author: Kevin Black
// based off of code in httptServer.cc
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

#include <iostream>

Define_Module(TCPSocketAPI);

TCPSocketAPI::TCPSocketAPI() : _socket_map(), _accept_callbacks(),
	_registered_callbacks(), _timeout_timers(), _resolver() {

}

TCPSocketAPI::~TCPSocketAPI() {
//	std::cout << "~TCPSocketAPI()..."<<endl;
	_socket_map.deleteSockets();

	std::map<int, CallbackData *>::iterator cb_itr = _registered_callbacks.begin();
	while (cb_itr != _registered_callbacks.end())
	{
		delete cb_itr->second;
		cb_itr++;
	}

	std::map<int, SocketTimeoutMsg *>::iterator tmr_itr = _timeout_timers.begin();
	while (tmr_itr != _timeout_timers.end()) {
		if (tmr_itr->second) {
			cancelAndDelete(tmr_itr->second);
		}
		tmr_itr++;
	}
//	std::cout << "~TCPSocketAPI()!"<<endl;
}

//==============================================================================
// cSimpleModule functions

void TCPSocketAPI::initialize()
{
	cSimpleModule::initialize();

	std::string __function_name = "initialize()";
	EV_DEBUG << "initializing TCP socket API" << endl;

	// other variables, scalars/vectors WATCH calls
}

void TCPSocketAPI::handleMessage(cMessage *msg)
{
	std::string __fname = "handleMessage";

	if (msg->isSelfMessage()) {
		SocketTimeoutMsg * timer = static_cast<SocketTimeoutMsg *>(msg);
		if (!timer) {
			signalFunctionError(__fname, "unknown self message received");
		}

		int socket_id = timer->getSocketId();
		CallbackData * cbdata = _registered_callbacks[socket_id];

		if (!cbdata) {
			signalFunctionError(__fname, "invalid socket id in timeout message");
		}
		EV_DEBUG << "Handle timeout on socket "<< socket_id << endl;
		socketTimeout(socket_id, cbdata);
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
			EV_WARNING << __fname << " received message for new connection " <<
				socket->getConnectionId() << " on non-listening port" <<
				" OR received a message for a closed socket" <<endl;
			// ignore it
			delete socket;
			delete msg;
			return;
		}
		 // assume that the message type is TCP_I_ESTABLISHED
		socket->setCallbackObject(this, i->second);
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

	std::string __fname = "socket";

	if (!cbobj) {
		signalFunctionError(__fname, "no callback object specified");
	}

	TCPSocket * socket = new TCPSocket();
	socket->setOutputGate(gate("tcpOut"));

	int id = socket->getConnectionId();
	CallbackData * cbdata = makeCallbackData(id, cbobj, NULL, CB_S_NONE);
	_registered_callbacks[id] = cbdata;
	socket->setCallbackObject(this, _registered_callbacks[id]);

	_socket_map.addSocket(socket);
	EV_DEBUG << "socket(): new socket " << socket->getConnectionId() << " created" << endl;
	return id;
}


void TCPSocketAPI::bind (int socket_id, std::string local_address,
		int local_port) {

	Enter_Method_Silent();

	std::string __fname = "bind";

	// verifies that socket exists
	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	if (local_address.empty()) {
		//checks socket state and port number
		socket->bind(local_port);
	}
	else {
		//checks socket state and port number
		socket->bind(_resolver.resolve(local_address.c_str(),
				IPAddressResolver::ADDR_PREFER_IPv4), local_port);
	}
	EV_DEBUG << "bind(): socket " << socket->getConnectionId() << " bound on " <<
		socket->getLocalAddress() << ":" << socket->getLocalPort() << endl;
}

void TCPSocketAPI::connect (int socket_id, std::string remote_address,
		int remote_port, void * yourPtr) {

	Enter_Method_Silent();

	std::string __fname = "connect";

	// verifies that socket exists
	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	CallbackData * cbdata = _registered_callbacks[socket_id];
	cbdata->function_data = yourPtr;
	cbdata->state = CB_S_CONNECT;

	socket->connect(_resolver.resolve(remote_address.c_str(),
			IPAddressResolver::ADDR_PREFER_IPv4), remote_port);

	EV_DEBUG << "connect(): socket " << socket->getConnectionId() << " connecting to " <<
		socket->getRemoteAddress() << ":" << socket->getRemotePort() << "..." << endl;
}


void TCPSocketAPI::listen (int socket_id, CallbackInterface * cbobj_for_accepted) {
	Enter_Method_Silent();

	std::string __fname = "listen";

	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	if (cbobj_for_accepted) {
		CallbackData * cbdata = _registered_callbacks[socket_id];
		cbdata->cbobj_for_accepted = cbobj_for_accepted;
	}

	// creates a forking socket
	socket->listen();

	EV_DEBUG << "listen(): socket " << socket->getConnectionId() << " listening on " <<
		socket->getLocalAddress() << ":" << socket->getLocalPort() << endl;
}

int TCPSocketAPI::makeActiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, std::string remote_address, int remote_port, void * yourPtr) {
	Enter_Method_Silent();
	int id = socket(cbobj);
	if (local_port >= 0) {
		bind(id, local_address, local_port);
	}
	connect(id, remote_address, remote_port, yourPtr);
	return id;
}

int TCPSocketAPI::makePassiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, CallbackInterface * cbobj_for_accepted) {
	Enter_Method_Silent();
	int id = socket(cbobj);
	bind(id, local_address, local_port);
	listen(id, cbobj_for_accepted);
	return id;
}

void TCPSocketAPI::accept (int socket_id, void * yourPtr) {

	Enter_Method_Silent();
	std::string __fname = "accept";

	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	if (socket->getState() != TCPSocket::LISTENING)
	{
		signalFunctionError(__fname, "socket is not a passive socket");
	}

	CallbackData * cbdata = _registered_callbacks[socket_id];
	cbdata->function_data = yourPtr;
	cbdata->state = CB_S_ACCEPT;

	_accept_callbacks[socket->getLocalPort()] = cbdata;
	EV_DEBUG << "accept(): socket "<< socket->getConnectionId() << " accepting on " <<
		socket->getLocalAddress() << ":" << socket->getLocalPort() << endl;
}

void TCPSocketAPI::send (int socket_id, cMessage * msg) {

	Enter_Method_Silent();

	std::string __fname = "send";

	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	take(msg); // take ownership of the message

	// remove any control info with the message
	cObject * ctrl_info = msg->removeControlInfo();
	if (ctrl_info)
		delete ctrl_info;

	socket->send(msg);
	EV_DEBUG << "send(): socket " << socket->getConnectionId() << " sent message " <<
		msg->getName() << endl;
}

void TCPSocketAPI::recv (int socket_id, void * yourPtr) {

	Enter_Method_Silent();

	std::string __fname = "recv";

	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);
	if (socket->getState() != TCPSocket::CONNECTED) {
		signalFunctionError(__fname, "socket is not connected");
	}
	CallbackData * cbdata = _registered_callbacks[socket_id];
	cbdata->function_data = yourPtr;
	cbdata->state = CB_S_RECV;

	// schedule a timeout if set
	SocketTimeoutMsg * timer = _timeout_timers[socket_id];
	if (timer) {
		if (timer->isScheduled()) {
			cancelEvent(timer);
		}
		scheduleAt(simTime()+timer->getTimeoutInterval(), timer);
	}

	EV_DEBUG << "recv(): socket " << socket->getConnectionId() << " receiving..." << endl;
}

void TCPSocketAPI::setTimeout(int socket_id, simtime_t timeout_interval) {
	Enter_Method_Silent();
	std::string __fname = "setTimeout";

	findAndCheckSocket(socket_id, __fname);

	SocketTimeoutMsg * timer = _timeout_timers[socket_id];
	if (!timer) {
		timer = new SocketTimeoutMsg("socket timeout");
		timer->setSocketId(socket_id);
		_timeout_timers[socket_id] = timer;
	}
	else if (timer->isScheduled()){
		cancelEvent(timer);
	}

	// set interval on timer
	timer->setTimeoutInterval(timeout_interval.dbl());

	// see if a timeout should be scheduled now
	CallbackData * cbdata = _registered_callbacks[socket_id];
	if (cbdata && cbdata->state == CB_S_RECV) {
		scheduleAt(simTime()+timeout_interval, timer);
	}
}

bool TCPSocketAPI::removeTimeout(int socket_id) {
	Enter_Method_Silent();
	SocketTimeoutMsg * timer = _timeout_timers[socket_id];
	if (!timer) {
		return false;
	}

	_timeout_timers[socket_id] = NULL;
	cancelAndDelete(timer);
	return true;
}

void TCPSocketAPI::close (int socket_id) {

	Enter_Method_Silent();

	std::string __fname = "close";

	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	CallbackData * cbdata = _registered_callbacks[socket_id];
	// DO NOT set cbdata->function_data to NULL because getMyPtr needs
	// to be valid at any time, even after the socket is closed
	cbdata->state = CB_S_CLOSE;

	socket->close();
	EV_DEBUG << "close(): socket " << socket->getConnectionId() << " closing..." << endl;
}

void * TCPSocketAPI::getMyPtr(int socket_id) {
	if (_registered_callbacks[socket_id]) {
		return _registered_callbacks[socket_id]->function_data;
	}
	return NULL;
}

//==============================================================================
// TCPSocket::CallbackInterface functions

void TCPSocketAPI::socketEstablished(int connId, void *yourPtr)
{
	// update scalar variables
	std::string  __fname = "socketEstablished";

	if ( yourPtr == NULL ) {
		signalCBNullError(__fname);
	}

	// invoke the "connect" or "accept" callback
	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);

	CallbackData * new_cbdata = NULL;

	switch (cbdata->state){
	case CB_S_CONNECT:
		EV_DEBUG << "connected socket " << connId << endl;
		cbdata->state = CB_S_NONE;
		cbdata->cbobj->connectCallback(connId, 0, cbdata->function_data);
		break;
	case CB_S_ACCEPT:
		// set the spawned socket's callback data to match itself now
		new_cbdata = makeCallbackData(connId, cbdata->cbobj_for_accepted, NULL, CB_S_NONE);
		_registered_callbacks[connId] = new_cbdata;
		_socket_map.getSocket(connId)->setCallbackObject(this, new_cbdata);

		EV_DEBUG << "accepted socket with id=" << connId << endl;

		// notify the passive socket of an accept
		cbdata->cbobj->acceptCallback(cbdata->socket_id, connId, cbdata->function_data);
		break;
	case CB_S_CLOSE:
		EV_DEBUG << "socketEstablished(): CLOSED callback received" << endl;
		break;
	default: // i.e. CB_S_RECV, CB_S_TIMEOUT, or CB_S_NONE
		signalCBStateError(__fname, cbdata->state);
	}
}

void TCPSocketAPI::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
	std::string __fname = "socketDataArrived";

	if ( yourPtr==NULL )
	{
		signalCBNullError(__fname);
	}

	// invoke the recv callback
	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);
	SocketTimeoutMsg * timer = NULL;
	switch (cbdata->state)
	{
	case CB_S_RECV:
		timer = _timeout_timers[connId];
		if (timer) {
			cancelEvent(timer);
			scheduleAt(simTime()+timer->getTimeoutInterval(), timer);
		}
		cbdata->cbobj->recvCallback(connId, msg->getByteLength(), msg, cbdata->function_data);
		break;
	case CB_S_CLOSE:
		EV_DEBUG << "socketDataArrived(): CLOSED callback received" << endl;
		delete msg;
		break;
	case CB_S_TIMEOUT:
		EV_DEBUG << "socketDataArrived(): TIMEOUT callback recieved" << endl;
		delete msg;
		break;
	default: // i.e. CB_S_CONNECT, CB_S_ACCEPT, or CB_S_NONE
		signalCBStateError(__fname, cbdata->state);
	}
}

void TCPSocketAPI::socketPeerClosed(int connId, void *yourPtr)
{
	std::string __fname = "socketPeerClosed";
	if ( yourPtr==NULL )
	{
		signalCBNullError(__fname);
	}
	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);

	// invoke the recv callback or just handle the close operation?
	// assume that the application will close the socket
	switch (cbdata->state)
	{
	case CB_S_RECV:
		cbdata->cbobj->recvCallback(connId, CB_E_CLOSED, NULL, cbdata->function_data);
		break;
	case CB_S_CLOSE:
		EV_DEBUG << "socketPeerClosed(): CLOSED callback received" << endl;
		break;
	default: // i.e. CB_S_CONNECT, CB_S_ACCEPT, CB_S_TIMEOUT, or CB_S_NONE
		signalCBStateError(__fname, cbdata->state);
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
	std::string __fname = "socketFailure";

	if (code == TCP_I_TIMED_OUT) {
		socketTimeout(connId, yourPtr);
		return;
	}

	EV_WARNING << "connection broken. Connection id " << connId;

	EV_INFO << "connection closed. Connection id " << connId;

	if ( yourPtr==NULL )
	{
		signalCBNullError(__fname);
	}

	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);

	CALLBACK_ERROR ecode = CB_E_UNKNOWN;

	if (code==TCP_I_CONNECTION_RESET) {
		EV_WARNING << "Connection reset!" << endl;
		ecode = CB_E_RESET;
	} 
	else if (code==TCP_I_CONNECTION_REFUSED) {
		EV_WARNING << "Connection refused!" << endl;
		ecode = CB_E_REFUSED;
	}
	else {
		EV_WARNING << "Unknown TCP socket failure code!"<<endl;
	}

	// invoke the recv callback or just handle the close operation?
	switch (cbdata->state)
	{
	case CB_S_RECV:
		cbdata->cbobj->recvCallback(connId, ecode, NULL, cbdata->function_data);
		break;
	case CB_S_CONNECT:
		cbdata->cbobj->connectCallback(connId, ecode, cbdata->function_data);
		break;
	case CB_S_CLOSE:
		EV_DEBUG << "socketFailure(): CLOSE callback received" << endl;
		break;
	case CB_S_TIMEOUT:
		EV_DEBUG << "socketFailure(): TIMEOUT callback received" << endl;
		break;
	default: // i.e. CB_S_ACCEPT, CB_S_NONE
		signalCBStateError(__fname, cbdata->state);
	}

	// Cleanup
	TCPSocket * socket = _socket_map.removeSocket(connId);
	delete socket;
}

void TCPSocketAPI::socketTimeout(int connId, void * yourPtr) {

	std::string __fname = "socketTimeout";

	if ( yourPtr==NULL )
	{
		signalCBNullError(__fname);
	}

	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);

	switch (cbdata->state)
	{
	case CB_S_RECV:
		cbdata->state = CB_S_TIMEOUT;
		cbdata->cbobj->recvCallback(connId, CB_E_TIMEOUT, NULL, cbdata->function_data);
		break;
	case CB_S_CLOSE:
		EV_DEBUG << "socketTimeout(): CLOSE callback received" << endl;
		break;
	case CB_S_TIMEOUT:
		EV_WARNING << "socketTimeout(): multiple TIMEOUT occurred";
		break;
	default: // i.e. CB_S_NONE
		signalCBStateError(__fname, cbdata->state);
	}
}

//==============================================================================
// UTILITY FUNCTIONS

TCPSocket * TCPSocketAPI::findAndCheckSocket(int socket_id, const std::string & fname) {
	TCPSocket * socket = _socket_map.getSocket(socket_id);
	if (!socket) {
		throw cRuntimeError(this, "%s: invalid socket id %d", fname.c_str(), socket_id);
	}
	return socket;
}

TCPSocketAPI::CallbackData * TCPSocketAPI::makeCallbackData(int socket_id,
		CallbackInterface * cbobj, void * function_data, CALLBACK_STATE state) {

	CallbackData * cbdata = new CallbackData();
	cbdata->socket_id = socket_id;
	cbdata->cbobj = cbobj;
	cbdata->function_data = function_data;
	cbdata->state = state;
	cbdata->cbobj_for_accepted = cbobj;
	return cbdata;
}

void TCPSocketAPI::signalFunctionError(const std::string & fname, const std::string & details) {
	std::string msg = fname + ": "+details;
	throw cRuntimeError(this, msg.c_str());
}

void TCPSocketAPI::signalCBStateError(const std::string & fname, CALLBACK_STATE state) {
	std::string details = "";

	switch (state)
	{
	case CB_S_RECV:
		details = "RECV";
		break;
	case CB_S_CONNECT:
		details = "CONNECT";
		break;
	case CB_S_ACCEPT:
		details = "ACCEPT";
		break;
	case CB_S_CLOSE:
		details = "CLOSE";
		break;
	case CB_S_TIMEOUT:
		details = "TIMEOUT";
		break;
	case CB_S_NONE:
		details = "NONE";
		break;
	default:
		details = "unknown";
	}
	details += " callback received";
	signalFunctionError(fname, details);
}

void TCPSocketAPI::signalCBNullError(const std::string & fname) {
	signalFunctionError(fname, "no callback data with socket");
}
