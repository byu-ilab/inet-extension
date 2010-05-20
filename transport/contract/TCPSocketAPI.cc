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

TCPSocketAPI::TCPSocketAPI() : _socket_map(), _timeout_timers(), _resolver(),
	_accept_callbacks(), _registered_callbacks() {

}

TCPSocketAPI::~TCPSocketAPI() {
//	std::cout << "~TCPSocketAPI()..."<<endl;
	_socket_map.deleteSockets();

	std::map<int, CallbackData *>::iterator rcb_itr = _registered_callbacks.begin();
	while (rcb_itr != _registered_callbacks.end())
	{
		delete rcb_itr->second;
		rcb_itr++;
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

	std::string __fname = "initialize()";
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

	socket->connect(_resolver.resolve(remote_address.c_str(),
				IPAddressResolver::ADDR_PREFER_IPv4), remote_port);

	CallbackData * cbdata = _registered_callbacks[socket_id];
	if (cbdata->state != CB_S_NONE) {
		signalCBStateInconsistentError(__fname, cbdata->state);
	}
	cbdata->userptr = yourPtr;
	cbdata->state = CB_S_CONNECT;

	EV_DEBUG << __fname << ": socket " << socket->getConnectionId() << " connecting to " <<
		socket->getRemoteAddress() << ":" << socket->getRemotePort() << "..." << endl;
}


void TCPSocketAPI::listen (int socket_id, CallbackInterface * cbobj_for_accepted) {
	Enter_Method_Silent();

	std::string __fname = "listen";

	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	// creates a forking socket
	socket->listen();

	CallbackData * cbdata = _registered_callbacks[socket_id];
	cbdata->state = CB_S_WAIT;
	if (cbobj_for_accepted) {
		cbdata->cbobj_for_accepted = cbobj_for_accepted;
	}

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
	if (cbdata->state != CB_S_WAIT) {
		signalCBStateInconsistentError(__fname, cbdata->state);
	}
	cbdata->userptr = yourPtr;
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
	if (cbdata->state != CB_S_WAIT) {
		signalCBStateInconsistentError(__fname, cbdata->state);
	}
	cbdata->userptr = yourPtr;
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

void * TCPSocketAPI::close (int socket_id) {

	Enter_Method_Silent();

	std::string __fname = "close";

	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	CallbackData * cbdata = _registered_callbacks[socket_id];
	void * userPtr = cbdata->userptr;
	cbdata->userptr = NULL;
	cbdata->state = CB_S_CLOSE;

	socket->close();
	EV_DEBUG << "close(): socket " << socket->getConnectionId() << " closing..." << endl;
	return userPtr;
}

void * TCPSocketAPI::getMyPtr(int socket_id) {
	std::map<int, CallbackData *>::iterator rcb_itr = _registered_callbacks.find(socket_id);
	if (rcb_itr != _registered_callbacks.end()) {
		return rcb_itr->second->userptr;
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
	void * userPtr = cbdata->userptr;
	cbdata->userptr = NULL;

	CallbackData * new_cbdata = NULL;

	switch (cbdata->state){
	case CB_S_CONNECT:
		EV_DEBUG << "connected socket " << connId << endl;
		cbdata->state = CB_S_WAIT;
		cbdata->cbobj->connectCallback(connId, 0, userPtr);
		break;
	case CB_S_ACCEPT:
		// set the spawned socket's callback data to match itself now
		new_cbdata = makeCallbackData(connId, cbdata->cbobj_for_accepted, NULL, CB_S_WAIT);
		_registered_callbacks[connId] = new_cbdata;
		_socket_map.getSocket(connId)->setCallbackObject(this, new_cbdata);

		EV_DEBUG << "accepted socket with id=" << connId << endl;

		cbdata->state = CB_S_WAIT;

		// notify the passive socket of an accept
		cbdata->cbobj->acceptCallback(cbdata->socket_id, connId, userPtr);
		break;
	case CB_S_CLOSE:
	case CB_S_WAIT:
		printCBStateReceptionNotice(__fname, cbdata->state);
		break;
	default:
		signalCBStateReceptionError(__fname, cbdata->state);
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
	void * userPtr = cbdata->userptr;
	cbdata->userptr = NULL;

	switch (cbdata->state)
	{
	case CB_S_RECV:
		cbdata->state = CB_S_WAIT;
		cbdata->cbobj->recvCallback(connId, msg->getByteLength(), msg, userPtr);
		break;
	case CB_S_CLOSE:
	case CB_S_TIMEOUT:
	case CB_S_WAIT:
		delete msg;
		printCBStateReceptionNotice(__fname, cbdata->state);
		break;
	default:
		signalCBStateReceptionError(__fname, cbdata->state);
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
	void * userPtr = cbdata->userptr;
	cbdata->userptr = NULL;

	// invoke the recv callback or just handle the close operation?
	// assume that the application will close the socket
	switch (cbdata->state)
	{
	case CB_S_RECV:
		cbdata->state = CB_S_CLOSE;
		cbdata->cbobj->recvCallback(connId, CB_E_CLOSED, NULL, userPtr);
		break;
	case CB_S_CLOSE:
	case CB_S_WAIT:
		printCBStateReceptionNotice(__fname, cbdata->state);
		break;
	default:
		signalCBStateReceptionError(__fname, cbdata->state);
	}
}

void TCPSocketAPI::socketClosed(int connId, void *yourPtr)
{
	EV_INFO << "connection closed. Connection id " << connId << endl;

	// okay if yourPtr == NULL
	cleanupSocket(connId);
}

void TCPSocketAPI::socketFailure(int connId, void *yourPtr, int code)
{
	std::string __fname = "socketFailure";

	// todo branch or no?
	if (code == TCP_I_TIMED_OUT) {
		socketTimeout(connId, yourPtr);
		return;
	}

	EV_WARNING << "connection broken. Connection id " << connId;

	if ( yourPtr==NULL )
	{
		signalCBNullError(__fname);
	}

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

	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);
	void * userPtr = cbdata->userptr;
	cbdata->userptr = NULL;

	// invoke the recv callback or just handle the close operation?
	switch (cbdata->state)
	{
	case CB_S_RECV:
		cbdata->state = CB_S_CLOSE;
		cbdata->cbobj->recvCallback(connId, ecode, NULL, userPtr);
		break;
	case CB_S_CONNECT:
		cbdata->state = CB_S_CLOSE;
		cbdata->cbobj->connectCallback(connId, ecode, userPtr);
		break;
	case CB_S_CLOSE:
	case CB_S_TIMEOUT:
	case CB_S_WAIT:
		printCBStateReceptionNotice(__fname, cbdata->state);
		break;
	default:
		signalCBStateReceptionError(__fname, cbdata->state);
	}
}

//==============================================================================
// UTILITY FUNCTIONS

void TCPSocketAPI::socketTimeout(int connId, void * yourPtr) {

	std::string __fname = "socketTimeout";

	if ( yourPtr==NULL )
	{
		signalCBNullError(__fname);
	}

	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);
	void * userPtr = cbdata->userptr;
	cbdata->userptr = NULL;

	switch (cbdata->state)
	{
	case CB_S_RECV:
		cbdata->state = CB_S_TIMEOUT;
		cbdata->cbobj->recvCallback(connId, CB_E_TIMEOUT, NULL, userPtr);
		break;
	case CB_S_CLOSE:
	case CB_S_WAIT:
		printCBStateReceptionNotice(__fname, cbdata->state);
		break;
	case CB_S_TIMEOUT:
		printFunctionNotice(__fname, "multiple TIMEOUT occurred");
		break;
	default:
		signalCBStateReceptionError(__fname, cbdata->state);
	}
}

void TCPSocketAPI::cleanupSocket(int socket_id) {
	// delete socket
	TCPSocket * socket = _socket_map.removeSocket(socket_id);
	if (socket) {
		delete socket;
	}

	// delete callback data
	std::map<int, CallbackData *>::iterator rcb_itr = _registered_callbacks.find(socket_id);
	if (rcb_itr != _registered_callbacks.end()) {
		if (rcb_itr->second) {
			delete rcb_itr->second;
		}
		_registered_callbacks.erase(rcb_itr);
	}
}

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
	cbdata->userptr = function_data;
	cbdata->state = state;
	cbdata->cbobj_for_accepted = cbobj;
	return cbdata;
}

std::string TCPSocketAPI::getStateName(CALLBACK_STATE state) {
	std::string name = "";
	switch (state)
	{
	case CB_S_RECV:
		name = "RECV";
		break;
	case CB_S_CONNECT:
		name = "CONNECT";
		break;
	case CB_S_ACCEPT:
		name = "ACCEPT";
		break;
	case CB_S_CLOSE:
		name = "CLOSE";
		break;
	case CB_S_TIMEOUT:
		name = "TIMEOUT";
		break;
	case CB_S_NONE:
		name = "NONE";
		break;
	case CB_S_WAIT:
		name = "WAIT";
		break;
	default:
		name = "unknown";
	}
	return name;
}

void TCPSocketAPI::signalFunctionError(const std::string & fname, const std::string & details) {
	std::string msg = fname + ": "+details;
	throw cRuntimeError(this, msg.c_str());
}

void TCPSocketAPI::signalCBStateReceptionError(const std::string & fname, CALLBACK_STATE state) {
	std::string details = getStateName(state) + " callback received";
	signalFunctionError(fname, details);
}

void TCPSocketAPI::signalCBStateInconsistentError(const std::string & fname, CALLBACK_STATE state) {
	std::string details = "inconsistent state: "+getStateName(state);
	signalFunctionError(fname, details);
}

void TCPSocketAPI::signalCBNullError(const std::string & fname) {
	signalFunctionError(fname, "no callback data with socket");
}

void TCPSocketAPI::printFunctionNotice(const std::string & fname, const std::string & notice){
	EV_DEBUG << fname << ": " << notice;
}

void TCPSocketAPI::printCBStateReceptionNotice(const std::string & fname, CALLBACK_STATE state) {
	std::string notice = getStateName(state) + " callback received";
	printFunctionNotice(fname, notice);
}
