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
#include <sstream>

Define_Module(TCPSocketAPI);

TCPSocketAPI::TCPSocketAPI() : _socket_map(), /* _rejected_sockets_map(), */
	_timeout_timers(), _resolver(), _bound_ports(),
	_passive_callbacks(), _registered_callbacks(),
	_pending_sockets_map(), _pending_connections(), _reception_buffers() {

}

TCPSocketAPI::~TCPSocketAPI() {
	_socket_map.deleteSockets();
	_pending_sockets_map.deleteSockets();

	// delete the registered callbacks
	std::map<int, CallbackData *>::iterator rcb_itr = _registered_callbacks.begin();
	while (rcb_itr != _registered_callbacks.end())
	{
		delete rcb_itr->second;
		rcb_itr++;
	}

	// delete the timers
	std::map<int, SocketTimeoutMsg *>::iterator tmr_itr = _timeout_timers.begin();
	while (tmr_itr != _timeout_timers.end()) {
		if (tmr_itr->second) {
			cancelAndDelete(tmr_itr->second);
		}
		tmr_itr++;
	}

	// delete messages sitting in the reception buffers
	std::map<int, std::deque<cPacket *> >::iterator rb_itr = _reception_buffers.begin();
	std::deque<cPacket *>::iterator q_itr;
	std::deque<cPacket *>::iterator qend_itr;
	while (rb_itr != _reception_buffers.end())
	{
		q_itr = rb_itr->second.begin();
		qend_itr = rb_itr->second.end();
		while (q_itr != qend_itr)
		{
			deleteSafe((*q_itr));
			q_itr++;
		}
		rb_itr++;
	}
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

//		CallbackData * cbdata = getAcceptCallback(socket->getLocalPort());
		CallbackData * cbdata = getPassiveCallback(socket->getLocalPort());

		if (!cbdata)
		{
			EV_WARNING << __fname << " received message for connection " <<
				socket->getConnectionId() << " on non-listening port" <<
				" OR received a message for a closed socket" <<endl;
//			if (msg->getKind() == TCP_I_ESTABLISHED)
//			{
//				// close the socket so the other end will close too
//				//_socket_map.addSocket(socket);
//				_rejected_sockets_map.addSocket(socket);
//				socket->setCallbackObject(this, NULL);
//				socket->close();
//			}
//			else
//			{
				delete socket;
//			}
			delete msg;
			return;
		}
		// assert that the message type is TCP_I_ESTABLISHED
		if (msg->getKind() != TCP_I_ESTABLISHED)
		{
			signalFunctionError(__fname, "message is not TCP_I_ESTABLISHED");
		}

		socket->setCallbackObject(this, cbdata);
		_pending_sockets_map.addSocket(socket);
	}
	EV_DEBUG << "on the socket: "<<socket->toString()<<endl;
	EV_DEBUG << "Process the message " << msg->getName() << endl;
	socket->processMessage(msg);

	// update display?
}

void TCPSocketAPI::finish() {
	// record scalars
}

//==============================================================================
// TCP Socket API functions

bool TCPSocketAPI::isCallbackError(int error)
{
	switch(error)
	{
	case CB_E_UNKNOWN:
	case CB_E_CLOSED:
	case CB_E_TIMEOUT:
	case CB_E_RESET:
	case CB_E_REFUSED:
		return true;
		break;
	default:
		return false;
	}
}

std::string TCPSocketAPI::getCallbackErrorName(int error)
{
	switch(error)
	{
	case CB_E_UNKNOWN:
		return "UNKNOWN";
		break;
	case CB_E_CLOSED:
		return "CLOSED";
		break;
	case CB_E_TIMEOUT:
		return "TIMEOUT";
		break;
	case CB_E_RESET:
		return "RESET";
		break;
	case CB_E_REFUSED:
		return "REFUSED";
		break;
	default:
		return "UNDEFINED";
	}
}

bool TCPSocketAPI::isCallbackType(int type)
{

	switch(type)
	{
	case CB_T_CONNECT:
	case CB_T_ACCEPT:
	case CB_T_RECV:
		return true;
		break;
	default:
		return false;
	}
}

std::string TCPSocketAPI::getCallbackTypeName(int type)
{
	switch(type)
	{
	case CB_T_CONNECT:
		return "CONNECT";
		break;
	case CB_T_ACCEPT:
		return "ACCPET";
		break;
	case CB_T_RECV:
		return "RECV";
		break;
	default:
		return "UNDEFINED";
	}
}

int TCPSocketAPI::getLocalPort(int socket_id)
{
	Enter_Method_Silent();
	std::string __fname = "getLocalPort";
	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	return socket->getLocalPort();
}

int TCPSocketAPI::getRemotePort(int socket_id)
{
	Enter_Method_Silent();
	std::string __fname = "getRemotePort";
	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	return socket->getRemotePort();
}

IPvXAddress TCPSocketAPI::getLocalAddress(int socket_id)
{
	Enter_Method_Silent();
	std::string __fname = "getLocalAddress";
	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	return socket->getLocalAddress();
}

IPvXAddress TCPSocketAPI::getRemoteAddres(int socket_id)
{
	Enter_Method_Silent();
	std::string __fname = "getRemoteAddress";
	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	return socket->getRemoteAddress();
}

std::string TCPSocketAPI::socketToString(int socket_id)
{
	Enter_Method_Silent();
	std::string __fname = "socketToString";
	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	return socket->toString();
}

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
	printFunctionNotice(__fname, socket->toString());
	return id;
}


void TCPSocketAPI::bind (int socket_id, std::string local_address,
		int local_port) {

	Enter_Method_Silent();

	std::string __fname = "bind";

	// verifies that socket exists
	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	// check if port is available
	if (local_port != -1)
	{
		std::map<int, std::set<int> >::iterator bprt_itr = _bound_ports.find(local_port);
		if (bprt_itr != _bound_ports.end())
		{
			signalFunctionError(__fname, "port is being used");
		}
	}

	if (local_address.empty()) {
		//checks socket state and port number
		socket->bind(local_port);
	}
	else {
		// checks socket state and port number, allows for only the
		// address to be specified if port = -1 but address must
		// thus be valid
		socket->bind(_resolver.resolve(local_address.c_str(),
				IPAddressResolver::ADDR_PREFER_IPv4), local_port);
	}

	// mark the port as claimed
	if (local_port != -1)
	{
		_bound_ports[local_port].insert(socket_id);
	}

	printFunctionNotice(__fname, socket->toString());
}

void TCPSocketAPI::connect (int socket_id, std::string remote_address,
		int remote_port, void * yourPtr) {

	Enter_Method_Silent();

	std::string __fname = "connect";

	// verifies that socket exists
	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	// check here because this isn't checked until the TCP core
	// processes the connect request
	if (remote_address.empty())
	{
		signalFunctionError(__fname, "remote address must be specified");
	}

	// checks the port but not the address
	socket->connect(_resolver.resolve(remote_address.c_str(),
				IPAddressResolver::ADDR_PREFER_IPv4), remote_port);

	CallbackData * cbdata = _registered_callbacks[socket_id];
	if (cbdata->state != CB_S_NONE)
	{ // this really shouldn't be possible
		signalCBStateInconsistentError(__fname, cbdata->state);
	}
	cbdata->userptr = yourPtr;
	cbdata->state = CB_S_CONNECT;

	printFunctionNotice(__fname, socket->toString());
}


void TCPSocketAPI::listen (int socket_id, CallbackInterface * cbobj_for_accepted) {
	Enter_Method_Silent();

	std::string __fname = "listen";

	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	// verify that the port has been specified
	if (socket->getLocalPort() == -1)
	{
		signalFunctionError(__fname, "port must be specified");
	}

	// creates a forking socket
	socket->listen();

	CallbackData * cbdata = _registered_callbacks[socket_id];
	cbdata->state = CB_S_WAIT;
	if (cbobj_for_accepted) {
		cbdata->cbobj_for_accepted = cbobj_for_accepted;
	}

	_passive_callbacks[socket->getLocalPort()] = cbdata;

	printFunctionNotice(__fname, socket->toString());
}

int TCPSocketAPI::makeActiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, std::string remote_address, int remote_port, void * yourPtr) {
	Enter_Method_Silent();
	int id = socket(cbobj);
	// check if bind should be skipped
	if (!local_address.empty() || local_port >= 0)
	{
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

	CallbackData * cbdata = _passive_callbacks[socket->getLocalPort()];//_registered_callbacks[socket_id];
	if (cbdata->state != CB_S_WAIT)
	{
		signalCBStateInconsistentError(__fname, cbdata->state);
	}
	cbdata->userptr = yourPtr;
	cbdata->state = CB_S_ACCEPT;

	if (!_pending_connections[socket_id].empty())
	{
		int pending_socket_fd = _pending_connections[socket_id].front();
		_pending_connections[socket_id].pop_front();
		//TCPSocket * pending_socket = _pending_sockets_map.getSocket(pending_socket_fd);
		socketEstablished(pending_socket_fd, cbdata);
	}

	//_passive_callbacks[socket->getLocalPort()] = cbdata;

	printFunctionNotice(__fname, socket->toString());
}

// currently designed to allow sending even while the socket is
// still connecting or the other end of the connection closed
void TCPSocketAPI::send (int socket_id, cMessage * msg) {

	Enter_Method_Silent();

	std::string __fname = "send";

	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);

	// check the socket's state before taking the message
	if (socket->getState() != TCPSocket::CONNECTED &&
			socket->getState() != TCPSocket::CONNECTING &&
			socket->getState() != TCPSocket::PEER_CLOSED)
	{
		signalFunctionError(__fname, "the socket is not connected or connecting");
	}

	if (!msg)
	{
		signalFunctionError(__fname, "cannot send a NULL message");
	}

	// take ownership of the message
	take(msg);
	// remove any control info with the message
	cObject * ctrl_info = msg->removeControlInfo();
	if (ctrl_info) {
		delete ctrl_info;
	}

	CallbackData * cbdata = _registered_callbacks[socket_id];
	if (cbdata->state != CB_S_WAIT &&
			cbdata->state != CB_S_RECV &&
			cbdata->state != CB_S_CONNECT)
	{
		signalCBStateInconsistentError(__fname, cbdata->state);
	}

	socket->send(msg);
	printFunctionNotice(__fname, socket->toString() + " sent message "+msg->getName());
}

void TCPSocketAPI::recv (int socket_id, void * yourPtr) {

	Enter_Method_Silent();

	std::string __fname = "recv";

	TCPSocket * socket = findAndCheckSocket(socket_id, __fname);
	if (socket->getState() != TCPSocket::CONNECTED) {
		signalFunctionError(__fname, "socket is not connected");
	}

	CallbackData * cbdata = _registered_callbacks[socket_id];
	if (cbdata->state != CB_S_WAIT && cbdata->state != CB_S_RECV)
	{
		signalCBStateInconsistentError(__fname, cbdata->state);
	}
	cbdata->userptr = yourPtr;
	cbdata->state = CB_S_RECV;

	if (!_reception_buffers[socket_id].empty())
	{
		cPacket * msg = _reception_buffers[socket_id].front();
		_reception_buffers[socket_id].pop_front();
		socketDataArrived(socket_id, cbdata, msg, false);
	}
	else
	{
		// schedule a timeout if set
		SocketTimeoutMsg * timer = _timeout_timers[socket_id];
		if (timer) {
			if (timer->isScheduled()) {
				cancelEvent(timer);
			}
			scheduleAt(simTime()+timer->getTimeoutInterval(), timer);
		}
	}

	printFunctionNotice(__fname, socket->toString());
}

void TCPSocketAPI::setTimeout(int socket_id, simtime_t timeout_interval) {
	Enter_Method_Silent();
	std::string __fname = "setTimeout";

	findAndCheckSocket(socket_id, __fname);

	if (timeout_interval < 0)
	{
		signalFunctionError(__fname, "negative timeout interval not allowed");
	}

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

	switch(socket->getState())
	{
	case TCPSocket::NOT_BOUND:
	case TCPSocket::BOUND:
	case TCPSocket::SOCKERROR:
		// then remove the socket from the Socket API
		printFunctionNotice(__fname, socket->toString());
		cleanupSocket(socket_id);
		break;
	case TCPSocket::CLOSED:
	case TCPSocket::LOCALLY_CLOSED:
		signalFunctionError(__fname, "close() already called on this socket");
		break;
	default: // including TCPSocket::PEER_CLOSED, TCPSocket::CONNECTED, TCPSocket::LISTENING,
		// TCPSocket::CONNECTING
		// cancel any callbacks
		removeTimeout(socket_id);
		// free the port
		freePort(socket_id);
		// then initiate close messages on the connection
		socket->close();
		printFunctionNotice(__fname, socket->toString());
	}

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

	CallbackData * cbdata = static_cast<CallbackData *>(yourPtr);
	void * userPtr = cbdata->userptr;
	cbdata->userptr = NULL;

	CallbackData * new_cbdata = NULL;
	TCPSocket * socket = NULL;

	// invoke the "connect" or "accept" callback
	switch (cbdata->state){
	case CB_S_CONNECT:
		socket = _socket_map.getSocket(connId);
		_bound_ports[socket->getLocalPort()].insert(connId); // inserting into a SET
		EV_DEBUG << "connected socket " << connId << endl;
		cbdata->state = CB_S_WAIT;
		cbdata->cbobj->connectCallback(connId, 0, userPtr);
		break;
	case CB_S_ACCEPT:
		// set the spawned socket's callback data to match itself now
		new_cbdata = makeCallbackData(connId, cbdata->cbobj_for_accepted, NULL, CB_S_WAIT);
		_registered_callbacks[connId] = new_cbdata;
		socket = _pending_sockets_map.removeSocket(connId);
		socket->setCallbackObject(this, new_cbdata);
		_socket_map.addSocket(socket);
		_bound_ports[socket->getLocalPort()].insert(connId);

		EV_DEBUG << "accepted socket with id=" << connId << endl;

		cbdata->state = CB_S_WAIT;

		// notify the passive socket of an accept
		cbdata->cbobj->acceptCallback(cbdata->socket_id, connId, userPtr);
		break;
	case CB_S_CLOSE:
		_pending_sockets_map.removeSocket(connId);
		printCBStateReceptionNotice(__fname, cbdata->state);
		break;
	case CB_S_WAIT:
		_pending_connections[cbdata->socket_id].push_back(connId);
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
	case CB_S_WAIT:
		_reception_buffers[connId].push_back(msg);
		break;
	case CB_S_CLOSE:
	case CB_S_TIMEOUT:
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
	//cbdata->state = CB_S_CLOSE;

	// invoke the recv callback or just handle the close operation?
	// assume that the application will close the socket
	switch (cbdata->state)
	{
	case CB_S_RECV:
		cbdata->state = CB_S_CLOSE;
		cbdata->cbobj->recvCallback(connId, CB_E_CLOSED, NULL, userPtr);
		break;
//	default:
//		cbdata->cbobj->closeNotice(connId, userPtr);

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
	case CB_S_CONNECT:
		cbdata->state = CB_S_TIMEOUT;
		cbdata->cbobj->connectCallback(connId, CB_E_TIMEOUT, userPtr);
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
	freePort(socket_id);
	removeTimeout(socket_id);
	// delete the socket
	TCPSocket * socket = _socket_map.removeSocket(socket_id);
	if (socket)
	{
		delete socket;
	}
//	else
//	{
//		socket = _rejected_sockets_map.removeSocket(socket_id);
//		if (socket)
//		{
//			delete socket;
//		}
//	}

	// delete callback data
	std::map<int, CallbackData *>::iterator rcb_itr = _registered_callbacks.find(socket_id);
	if (rcb_itr != _registered_callbacks.end()) {
		if (rcb_itr->second) {
			delete rcb_itr->second;
		}
		_registered_callbacks.erase(rcb_itr);

	}
}

void TCPSocketAPI::freePort(int socket_id)
{
	TCPSocket * socket = _socket_map.getSocket(socket_id);
	if (!socket) {
		return;
	}

	std::map<int, std::set<int> >::iterator bprt_itr = _bound_ports.find(socket->getLocalPort());
	if (bprt_itr == _bound_ports.end()) {
		return;
	}

	if (bprt_itr->second.size() <= 1) // shouldn't be less than 1, but to be safe ...
	{
		_bound_ports.erase(bprt_itr);
		return;
	}

	std::set<int>::iterator id_itr = bprt_itr->second.find(socket_id);
	if (id_itr != bprt_itr->second.end())
	{
		bprt_itr->second.erase(id_itr);
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
		CallbackInterface * cbobj, void * userptr, CALLBACK_STATE state) {

	CallbackData * cbdata = new CallbackData();
	cbdata->socket_id = socket_id;
	cbdata->cbobj = cbobj;
	cbdata->userptr = userptr;
	cbdata->state = state;
	cbdata->cbobj_for_accepted = cbobj;
	return cbdata;
}

TCPSocketAPI::CallbackData * TCPSocketAPI::getAcceptCallback(int port)
{
	// IMPORTANT can't do it through _bound_ports since the accepted
	// sockets have the same port as the passive listening socket

	std::map<int, CallbackData *>::iterator pcb_itr = _passive_callbacks.find(port);

	if (pcb_itr == _passive_callbacks.end())
	{
		return NULL;
	}

	if (pcb_itr->second->state != CB_S_ACCEPT)
	{
		return NULL;
	}

	return pcb_itr->second;
}

TCPSocketAPI::CallbackData * TCPSocketAPI::getPassiveCallback(int port)
{
	// IMPORTANT can't do it through _bound_ports since the accepted
	// sockets have the same port as the passive listening socket

	std::map<int, CallbackData *>::iterator pcb_itr = _passive_callbacks.find(port);

	if (pcb_itr == _passive_callbacks.end())
	{
		return NULL;
	}

	return pcb_itr->second;
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
	EV_DEBUG << fname << ": " << notice << endl;
}

void TCPSocketAPI::printCBStateReceptionNotice(const std::string & fname, CALLBACK_STATE state) {
	std::string notice = getStateName(state) + " callback received";
	printFunctionNotice(fname, notice);
}
