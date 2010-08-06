/**
 * \file TCPSocketMgr.cc
 *
 * Created: August 2, 2010
 * Author: Kevin Black
 *
 * Based off of code in httptServer.cc
 *
 * @todo Check GPL notice.
 */

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

// From inet
#include "TCPSocketMgr.h"
//#include "TCPSocketMap.h"
//#include "IPAddressResolver.h"
//#include "SocketTimeoutMsg_m.h"
#include "TCPPortRangeDefs.h"
#include "httptLogdefs.h"
#include "TCPCommand_m.h"
#include "DuplicateHttpMessageNameObserver.h"
#include "TCPConnInfoMappingObserver.h"

// From omnetpp extension
#include <omnetppextension.h>

// From standard C++ libraries
#include <iostream>
#include <sstream>

#define DEBUG_CLASS false

Define_Module(TCPSocketMgr);

TCPSocketMgr::TCPSocketMgr()
	: _socket_map(), /* _rejected_sockets_map(), */
	_timeout_timers(), _resolver(), _bound_ports(),
	_passive_callbacks(), _registered_callbacks(),
	_pending_sockets_map(), _pending_connections(), _reception_buffers()
{
}

TCPSocketMgr::~TCPSocketMgr()
{
	_socket_map.deleteSockets();
	_pending_sockets_map.deleteSockets();

	// delete the registered callbacks
	Id_CBDataMap::iterator rcb_itr = _registered_callbacks.begin();
	while (rcb_itr != _registered_callbacks.end())
	{
		delete rcb_itr->second;
		rcb_itr++;
	}

	// delete the timers
	Id_TimeoutMsgMap::iterator tmr_itr = _timeout_timers.begin();
	while (tmr_itr != _timeout_timers.end())
	{
		if (tmr_itr->second)
		{
			cancelAndDelete(tmr_itr->second);
		}
		tmr_itr++;
	}

	// delete messages sitting in the reception buffers
	Id_PacketDequeMap::iterator rb_itr = _reception_buffers.begin();
	PacketDeque::iterator q_itr;
	PacketDeque::iterator qend_itr;
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

void TCPSocketMgr::initialize()
{
	cSimpleModule::initialize();

	std::string __fname = "initialize()";
	LOG_DEBUG_LN("initializing TCP socket API");

	// other variables, scalars/vectors WATCH calls

	_should_map_tcp_connections = par("shouldMapTCPConnections");
	LOG_DEBUG_LN("should map tcp connections: "<<(_should_map_tcp_connections ? "true":"false"));

	if (_should_map_tcp_connections)
	{
		TCPConnInfoMappingObserver::getInstance()->subscribeOnDefaultSignal(this);
	}

	_should_track_dup_msg_names = par("shouldTrackDuplicateMessageNames");
	LOG_DEBUG_LN("should track duplicate message names: "<<(_should_track_dup_msg_names ? "true":"false"));

	if (_should_track_dup_msg_names)
	{
		DuplicateHttpMessageNameObserver::getInstance()->subscribeOnDefaultSignal(this);
	}
}

void TCPSocketMgr::handleMessage(cMessage *msg)
{
	std::string __fname = "handleMessage";

	if (msg->isSelfMessage()) {
		SocketTimeoutMsg * timer = static_cast<SocketTimeoutMsg *>(msg);
		if (!timer)
		{
			signalFunctionError(__fname, "unknown self message received");
		}

		socket_id_t socket_id = timer->getSocketId();
		cb_data_ptr_t cbdata = _registered_callbacks[socket_id];

		if (!cbdata)
		{
			signalFunctionError(__fname, "invalid socket id in timeout message");
		}
		LOG_DEBUG_LN("Handle timeout on socket "<< socket_id);
		socketTimeout(socket_id, cbdata);
		return;
	}

	LOG_DEBUG_LN("Handle inbound message " << msg->getName() << " of kind " << msg->getKind());

	TCPSocket * socket = _socket_map.findSocketFor(msg);

	if (!socket)
	{
		LOG_DEBUG_LN("No socket found for the message. Create a new one");
		// this is to "accept" new connections
		// new connection -- create new socket object and server process
		socket = new TCPSocket(msg);
		socket->setOutputGate(gate("tcpOut"));

//		cb_data_ptr_t cbdata = getAcceptCallback(socket->getLocalPort());
		cb_data_ptr_t cbdata = getPassiveCallback(socket->getLocalPort());

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
	LOG_DEBUG_LN("on the socket: "<<socket->toString());
	LOG_DEBUG_LN("Process the message " << msg->getName());
	socket->processMessage(msg);

	// update display?
}

void TCPSocketMgr::finish()
{
	// record scalars
}

//==============================================================================
// TCP Socket API functions

//bool TCPSocketMgr::isCallbackError(error_id_t error)
//{
//	return TCPSocketAPI_Inet::isCallbackError(error);
//}

//std::string TCPSocketMgr::getCallbackErrorName(error_id_t error)
//{
//	return TCPSocketAPI_Inet::getCallbackErrorName(error);
//}

port_t TCPSocketMgr::getLocalPort(socket_id_t id)
{
	Enter_Method_Silent();
	std::string __fname = "getLocalPort";
	TCPSocket * socket = findAndCheckSocket(id, __fname);

	return socket->getLocalPort();
}

port_t TCPSocketMgr::getRemotePort(socket_id_t id)
{
	Enter_Method_Silent();
	std::string __fname = "getRemotePort";
	TCPSocket * socket = findAndCheckSocket(id, __fname);

	return socket->getRemotePort();
}

address_t TCPSocketMgr::getLocalAddress(socket_id_t id)
{
	Enter_Method_Silent();
	std::string __fname = "getLocalAddress";
	TCPSocket * socket = findAndCheckSocket(id, __fname);

	return socket->getLocalAddress().str();
}

address_t TCPSocketMgr::getRemoteAddres(socket_id_t id)
{
	Enter_Method_Silent();
	std::string __fname = "getRemoteAddress";
	TCPSocket * socket = findAndCheckSocket(id, __fname);

	return socket->getRemoteAddress().str();
}

std::string TCPSocketMgr::socketToString(socket_id_t id)
{
	Enter_Method_Silent();
	std::string __fname = "socketToString";
	TCPSocket * socket = findAndCheckSocket(id, __fname);

	return socket->toString();
}

int TCPSocketMgr::socket (cb_inet_handler_ptr_t cbobj)
{
	Enter_Method_Silent();

	std::string __fname = "socket";

	if (cbobj == NULL)
	{
		signalFunctionError(__fname, "no callback object specified");
	}

	TCPSocket * socket = new TCPSocket();
	socket->setOutputGate(gate("tcpOut"));

	int id = socket->getConnectionId();
	cb_data_ptr_t cbdata = makeCallbackData(id, cbobj, NULL, CB_S_NONE);
	_registered_callbacks[id] = cbdata;
	socket->setCallbackObject(this, _registered_callbacks[id]);

	_socket_map.addSocket(socket);
	printFunctionNotice(__fname, socket->toString());
	return id;
}


void TCPSocketMgr::bind (socket_id_t id, address_cref_t local_address,
		port_t local_port)
{

	Enter_Method_Silent();

	std::string __fname = "bind";

	// verifies that socket exists
	TCPSocket * socket = findAndCheckSocket(id, __fname);

	// check if port is available
	if (local_port != -1)
	{
		Port_IdSetMap::iterator bprt_itr = _bound_ports.find(local_port);
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
		_bound_ports[local_port].insert(id);
	}

	printFunctionNotice(__fname, socket->toString());
}

void TCPSocketMgr::connect (socket_id_t id, address_cref_t remote_address,
		port_t remote_port)
{
	connect(id, remote_address,remote_port,NULL);
}

void TCPSocketMgr::connect (socket_id_t id, address_cref_t remote_address,
		port_t remote_port, user_data_ptr_t yourPtr)
{

	Enter_Method_Silent();

	std::string __fname = "connect";

	// verifies that socket exists
	TCPSocket * socket = findAndCheckSocket(id, __fname);

	// check here because this isn't checked until the TCP core
	// processes the connect request
	if (remote_address.empty())
	{
		signalFunctionError(__fname, "remote address must be specified");
	}

	// checks the port but not the address
	socket->connect(_resolver.resolve(remote_address.c_str(),
				IPAddressResolver::ADDR_PREFER_IPv4), remote_port);

	cb_data_ptr_t cbdata = _registered_callbacks[id];
	if (cbdata->state != CB_S_NONE)
	{ // this really shouldn't be possible
		signalCBStateInconsistentError(__fname, cbdata->state);
	}
	cbdata->userptr = yourPtr;
	cbdata->state = CB_S_CONNECT;

	printFunctionNotice(__fname, socket->toString());
}


void TCPSocketMgr::listen (socket_id_t id, cb_inet_handler_ptr_t cbobj_for_accepted)
{
	Enter_Method_Silent();

	std::string __fname = "listen";

	TCPSocket * socket = findAndCheckSocket(id, __fname);

	// verify that the port has been specified
	if (socket->getLocalPort() == -1)
	{
		signalFunctionError(__fname, "port must be specified");
	}

	// creates a forking socket
	socket->listen();

	cb_data_ptr_t cbdata = _registered_callbacks[id];
	cbdata->state = CB_S_WAIT;
	if (cbobj_for_accepted)
	{
		cbdata->cbobj_for_accepted = cbobj_for_accepted;
	}

	_passive_callbacks[socket->getLocalPort()] = cbdata;

	printFunctionNotice(__fname, socket->toString());

	emitTCPConnInfo(socket);
}

socket_id_t TCPSocketMgr::makeActiveSocket (cb_base_handler_ptr_t cbobj,
	address_cref_t local_address, port_t local_port,
	address_cref_t remote_address, port_t remote_port)
{
	return makeActiveSocket(cbobj, local_address,local_port,remote_address,remote_port,NULL);
}

socket_id_t TCPSocketMgr::makeActiveSocket (cb_base_handler_ptr_t cbobj,
	address_cref_t local_address, port_t local_port,
	address_cref_t remote_address, port_t remote_port, user_data_ptr_t yourPtr)
{
	Enter_Method_Silent();
	int id = TCPSocketAPI_Inet::socket(cbobj);
	// check if bind should be skipped
	if (!local_address.empty() || local_port >= 0)
	{
		bind(id, local_address, local_port);
	}
	connect(id, remote_address, remote_port, yourPtr);
	return id;
}

socket_id_t TCPSocketMgr::makePassiveSocket (cb_base_handler_ptr_t cbobj,
	address_cref_t local_address, port_t local_port, cb_base_handler_ptr_t cbobj_for_accepted)
{
	Enter_Method_Silent();
	int id = TCPSocketAPI_Inet::socket(cbobj);
	bind(id, local_address, local_port);
	TCPSocketAPI_Inet::listen(id, cbobj_for_accepted);
	return id;
}

void TCPSocketMgr::accept (socket_id_t id)
{
	accept(id, NULL);
}

void TCPSocketMgr::accept (socket_id_t id, void * yourPtr)
{

	Enter_Method_Silent();
	std::string __fname = "accept";

	TCPSocket * socket = findAndCheckSocket(id, __fname);

	if (socket->getState() != TCPSocket::LISTENING)
	{
		signalFunctionError(__fname, "socket is not a passive socket");
	}

	cb_data_ptr_t cbdata = _passive_callbacks[socket->getLocalPort()];//_registered_callbacks[socket_id];
	if (cbdata->state != CB_S_WAIT)
	{
		signalCBStateInconsistentError(__fname, cbdata->state);
	}
	cbdata->userptr = yourPtr;
	cbdata->state = CB_S_ACCEPT;

	if (!_pending_connections[id].empty())
	{
		socket_id_t pending_socket_fd = _pending_connections[id].front();
		_pending_connections[id].pop_front();
		//TCPSocket * pending_socket = _pending_sockets_map.getSocket(pending_socket_fd);
		socketEstablished(pending_socket_fd, cbdata);
	}

	//_passive_callbacks[socket->getLocalPort()] = cbdata;

	printFunctionNotice(__fname, socket->toString());
}

// currently designed to allow sending even while the socket is
// still connecting or the other end of the connection closed
void TCPSocketMgr::send (socket_id_t id, cPacket * msg)
{

	Enter_Method_Silent();

	std::string __fname = "send";

	TCPSocket * socket = findAndCheckSocket(id, __fname);

	// check the socket's state before taking the message
	if (socket->getState() != TCPSocket::CONNECTED &&
			socket->getState() != TCPSocket::CONNECTING &&
			socket->getState() != TCPSocket::PEER_CLOSED)
	{
		signalFunctionError(__fname, "the socket is not connected or connecting");
	}

	if (msg == NULL)
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

	cb_data_ptr_t cbdata = _registered_callbacks[id];
	if (cbdata->state != CB_S_WAIT &&
			cbdata->state != CB_S_RECV &&
			cbdata->state != CB_S_CONNECT)
	{
		signalCBStateInconsistentError(__fname, cbdata->state);
	}

	//emit on the message event signal
	emitMessageEvent(msg, id);

	socket->send(msg);
	printFunctionNotice(__fname, socket->toString() + " sent message "+msg->getName());
}

void TCPSocketMgr::recv (socket_id_t id, bytecount_t byte_mode)
{
	recv(id, byte_mode, NULL);
}

void TCPSocketMgr::recv (socket_id_t id, bytecount_t byte_mode, user_data_ptr_t yourPtr)
{

	Enter_Method_Silent();

	std::string __fname = "recv";

	TCPSocket * socket = findAndCheckSocket(id, __fname);
	if (socket->getState() != TCPSocket::CONNECTED) {
		signalFunctionError(__fname, "socket is not connected");
	}

	cb_data_ptr_t cbdata = _registered_callbacks[id];
	if (cbdata->state != CB_S_WAIT)// && cbdata->state != CB_S_RECV)
	{
		signalCBStateInconsistentError(__fname, cbdata->state);
	}
	cbdata->userptr = yourPtr;
	cbdata->state = CB_S_RECV;

	if (!_reception_buffers[id].empty())
	{
		cPacket * msg = _reception_buffers[id].front();
		_reception_buffers[id].pop_front();
		socketDataArrived(id, cbdata, msg, false);
	}
	else
	{
		// schedule a timeout if set
		SocketTimeoutMsg * timer = _timeout_timers[id];
		if (timer) {
			if (timer->isScheduled()) {
				cancelEvent(timer);
			}
			scheduleAt(simTime()+timer->getTimeoutInterval(), timer);
		}
	}

	printFunctionNotice(__fname, socket->toString());
}

void TCPSocketMgr::setTimeout(socket_id_t id, simtime_t timeout_interval)
{
	Enter_Method_Silent();
	std::string __fname = "setTimeout";

	findAndCheckSocket(id, __fname);

	if (timeout_interval < 0)
	{
		signalFunctionError(__fname, "negative timeout interval not allowed");
	}

	SocketTimeoutMsg * timer = _timeout_timers[id];
	if (!timer) {
		timer = new SocketTimeoutMsg("socket timeout");
		timer->setSocketId(id);
		_timeout_timers[id] = timer;
	}
	else if (timer->isScheduled()){
		cancelEvent(timer);
	}

	// set interval on timer
	timer->setTimeoutInterval(timeout_interval.dbl());

	// see if a timeout should be scheduled now
	cb_data_ptr_t cbdata = _registered_callbacks[id];
	if (cbdata && cbdata->state == CB_S_RECV) {
		scheduleAt(simTime()+timeout_interval, timer);
	}
}

bool TCPSocketMgr::removeTimeout(socket_id_t id)
{
	Enter_Method_Silent();
	SocketTimeoutMsg * timer = _timeout_timers[id];
	if (!timer)
	{
		return false;
	}

	_timeout_timers[id] = NULL;
	cancelAndDelete(timer);
	return true;
}

void * TCPSocketMgr::close (socket_id_t id)
{

	Enter_Method_Silent();

	std::string __fname = "close";

	TCPSocket * socket = findAndCheckSocket(id, __fname);

	cb_data_ptr_t cbdata = _registered_callbacks[id];
	user_data_ptr_t userPtr = cbdata->userptr;
	cbdata->userptr = NULL;
	cbdata->state = CB_S_CLOSE;

	switch(socket->getState())
	{
	case TCPSocket::NOT_BOUND:
	case TCPSocket::BOUND:
	case TCPSocket::SOCKERROR:
		// then remove the socket from the Socket API
		printFunctionNotice(__fname, socket->toString());
		cleanupSocket(id);
		break;
	case TCPSocket::CLOSED:
	case TCPSocket::LOCALLY_CLOSED:
		signalFunctionError(__fname, "close() already called on this socket");
		break;
	default: // including TCPSocket::PEER_CLOSED, TCPSocket::CONNECTED, TCPSocket::LISTENING,
		// TCPSocket::CONNECTING
		// cancel any callbacks
		removeTimeout(id);
		// free the port
		freePort(id);
		// then initiate close messages on the connection
		socket->close();
		printFunctionNotice(__fname, socket->toString());
	}

	return userPtr;
}

void * TCPSocketMgr::getMyPtr(socket_id_t id)
{
	Id_CBDataMap::iterator rcb_itr = _registered_callbacks.find(id);
	if (rcb_itr != _registered_callbacks.end())
	{
		return rcb_itr->second->userptr;
	}
	return NULL;
}

//==============================================================================
// TCPSocket::CallbackInterface functions

void TCPSocketMgr::socketEstablished(int connId, void * yourPtr)
{
	// update scalar variables
	std::string  __fname = "socketEstablished";

	if ( yourPtr == NULL )
	{
		signalCBNullError(__fname);
	}

	cb_data_ptr_t cbdata = static_cast<cb_data_ptr_t>(yourPtr);
	void * userPtr = cbdata->userptr;
	cbdata->userptr = NULL;

	cb_data_ptr_t new_cbdata = NULL;
	TCPSocket * socket = NULL;

	// invoke the "connect" or "accept" callback
	switch (cbdata->state)
	{
	case CB_S_CONNECT:
		socket = _socket_map.getSocket(connId);
		_bound_ports[socket->getLocalPort()].insert(connId); // inserting into a SET
		LOG_DEBUG_LN("connected socket " << connId);
		emitTCPConnInfo(socket);
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

		LOG_DEBUG_LN("accepted socket with id=" << connId);
		emitTCPConnInfo(socket);

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

void TCPSocketMgr::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
{
	std::string __fname = "socketDataArrived";

	if ( yourPtr==NULL )
	{
		signalCBNullError(__fname);
	}

	// invoke the recv callback
	cb_data_ptr_t cbdata = static_cast<cb_data_ptr_t>(yourPtr);
	void * userPtr = cbdata->userptr;
	cbdata->userptr = NULL;

	// emit a message event signal
	emitMessageEvent(msg, connId);

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

void TCPSocketMgr::socketPeerClosed(int connId, void *yourPtr)
{
	std::string __fname = "socketPeerClosed";
	if ( yourPtr==NULL )
	{
		signalCBNullError(__fname);
	}
	cb_data_ptr_t cbdata = static_cast<cb_data_ptr_t>(yourPtr);
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

void TCPSocketMgr::socketClosed(int connId, void *yourPtr)
{
	EV_INFO << "connection closed. Connection id " << connId << endl;

	cleanupSocket(connId);
}

void TCPSocketMgr::socketFailure(int connId, void *yourPtr, int code)
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

	CallbackError ecode = CB_E_UNKNOWN;

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

	cb_data_ptr_t cbdata = static_cast<cb_data_ptr_t>(yourPtr);
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

void TCPSocketMgr::socketTimeout(int connId, void * yourPtr)
{

	std::string __fname = "socketTimeout";

	if ( yourPtr==NULL )
	{
		signalCBNullError(__fname);
	}

	cb_data_ptr_t cbdata = static_cast<cb_data_ptr_t>(yourPtr);
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

void TCPSocketMgr::cleanupSocket(socket_id_t id)
{
	freePort(id);
	removeTimeout(id);
	// delete the socket
	TCPSocket * socket = _socket_map.removeSocket(id);
	if (socket != NULL)
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
	Id_CBDataMap::iterator rcb_itr = _registered_callbacks.find(id);
	if (rcb_itr != _registered_callbacks.end())
	{
		if (rcb_itr->second)
		{
			delete rcb_itr->second;
		}
		_registered_callbacks.erase(rcb_itr);

	}
}

void TCPSocketMgr::freePort(socket_id_t id)
{
	TCPSocket * socket = _socket_map.getSocket(id);
	if (socket == NULL)
	{
		return;
	}

	Port_IdSetMap::iterator bprt_itr = _bound_ports.find(socket->getLocalPort());
	if (bprt_itr == _bound_ports.end())
	{
		return;
	}

	if (bprt_itr->second.size() <= 1) // shouldn't be less than 1, but to be safe ...
	{
		_bound_ports.erase(bprt_itr);
		return;
	}

	IdSet::iterator id_itr = bprt_itr->second.find(id);
	if (id_itr != bprt_itr->second.end())
	{
		bprt_itr->second.erase(id_itr);
	}
}

TCPSocket * TCPSocketMgr::findAndCheckSocket(socket_id_t id, str_cref_t fname)
{
	TCPSocket * socket = _socket_map.getSocket(id);
	if (socket == NULL)
	{
		throw cRuntimeError(this, "%s: invalid socket id %d", fname.c_str(), id);
	}
	return socket;
}

cb_data_ptr_t TCPSocketMgr::makeCallbackData(socket_id_t id,
		cb_inet_handler_ptr_t cbobj, user_data_ptr_t userptr, CALLBACK_STATE state)
{

	cb_data_ptr_t cbdata = new CallbackData();
	cbdata->socket_id = id;
	cbdata->cbobj = cbobj;
	cbdata->userptr = userptr;
	cbdata->state = state;
	cbdata->cbobj_for_accepted = cbobj;
	return cbdata;
}

cb_data_ptr_t TCPSocketMgr::getAcceptCallback(port_t port)
{
	// IMPORTANT can't do it through _bound_ports since the accepted
	// sockets have the same port as the passive listening socket

	std::map<int, cb_data_ptr_t>::iterator pcb_itr = _passive_callbacks.find(port);

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

cb_data_ptr_t TCPSocketMgr::getPassiveCallback(port_t port)
{
	// IMPORTANT can't do it through _bound_ports since the accepted
	// sockets have the same port as the passive listening socket

	std::map<int, cb_data_ptr_t>::iterator pcb_itr = _passive_callbacks.find(port);

	if (pcb_itr == _passive_callbacks.end())
	{
		return NULL;
	}

	return pcb_itr->second;
}

std::string TCPSocketMgr::getStateName(CALLBACK_STATE state)
{
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

void TCPSocketMgr::signalFunctionError(str_cref_t fname, str_cref_t details)
{
	std::string msg = fname + ": "+details;
	throw cRuntimeError(this, msg.c_str());
}

void TCPSocketMgr::signalCBStateReceptionError(str_cref_t fname, CALLBACK_STATE state)
{
	std::string details = getStateName(state) + " callback received";
	signalFunctionError(fname, details);
}

void TCPSocketMgr::signalCBStateInconsistentError(str_cref_t fname, CALLBACK_STATE state)
{
	std::string details = "inconsistent state: "+getStateName(state);
	signalFunctionError(fname, details);
}

void TCPSocketMgr::signalCBNullError(str_cref_t fname)
{
	signalFunctionError(fname, "no callback data with socket");
}

void TCPSocketMgr::printFunctionNotice(str_cref_t fname, str_cref_t notice)
{
	LOG_DEBUG_LN(fname << ": " << notice);
}

void TCPSocketMgr::printCBStateReceptionNotice(str_cref_t fname, CALLBACK_STATE state)
{
	std::string notice = getStateName(state) + " callback received";
	printFunctionNotice(fname, notice);
}

void TCPSocketMgr::emitTCPConnInfo(TCPSocket * socket)
{
	if (_should_map_tcp_connections)
	{
		simsignal_t s = TCPConnInfoMappingObserver::getInstance()->getDefaultSignalID();
		TCPConnInfoDatagram d(socket->getConnectionId(), socket->getLocalPort(),
				socket->getRemotePort(), socket->getLocalAddress().str(), socket->getRemoteAddress().str());
		emit(s, &d);
	}
}

void TCPSocketMgr::emitMessageEvent(const cMessage * msg, int interface_id)
{
	if (_should_track_dup_msg_names)
	{
		simsignal_t s = DuplicateHttpMessageNameObserver::getInstance()->getDefaultSignalID();
		cMessageEventDatagram d(msg, interface_id);
		emit(s, &d);
	}
}
