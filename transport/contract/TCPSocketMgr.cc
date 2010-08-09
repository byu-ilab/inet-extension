//==========================================================================80>
/**
 * @file TCPSocketMgr.cc
 *
 * TCPSocketMgr class definitions.
 *
 * Created: August 2, 2010
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
#include "httptLogdefs.h" //? user EV_DEBUG?
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
	ASSERT(msg != NULL);

	LOG_DEBUG_FUN_BEGIN("");

	TCPSocketExtension * socket = NULL;

	if (msg->isSelfMessage())
	{
		SocketTimeoutMsg * timer = check_and_cast<SocketTimeoutMsg *>(msg);
		ASSERT(timer != NULL); // unknown self message received

		socket_id_t socket_id = timer->getSocketId();
		socket = _socket_map.getSocket(socket_id);

		ASSERT(socket != NULL); // timer doesn't belong to a current socket

		LOG_DEBUG_LN("On the socket: "<<socket->toString());
		LOG_DEBUG_LN("Process the message " << msg->getName());
		socket->processMessage(msg);
	}
	else
	{
		LOG_DEBUG_LN("Handle inbound message " << msg->getName()
				<< " of kind " << msg->getKind());

		socket = _socket_map.findSocketFor(msg);

		if (socket == NULL)
		{
			// Accept a new connection
			LOG_DEBUG_LN("No socket found for the message. Create a new one");

			socket = new TCPSocketExtension(msg);
			socket->setOutputGate(gate("tcpOut"));

			// look up the passive socket on the same port as the new socket
			// in order to get its socket id

			// if there is no passive socket
			{
				LOG_DEBUG_LN("received message for connection "
						<<socket->getConnectionId() << " on non-listening port"
						<<" OR received a message for a closed socket");
				delete socket;
				delete msg;
				return;
			}
			// else
			ASSERT(msg->getKind() == TCP_I_ESTABLISHED);
			_pending_sockets_map.addSocket(socket);
			// passive socket->appendAcceptedSocket(socket);
	}

	// update display?
}

void TCPSocketMgr::finish()
{
	// record scalars if any
}

//==============================================================================
// TCP Socket API functions

port_t TCPSocketMgr::getLocalPort(socket_id_t id)
{
	Enter_Method_Silent(); // TODO necessary?
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->getLocalPort();
}

port_t TCPSocketMgr::getRemotePort(socket_id_t id)
{
	Enter_Method_Silent(); // TODO necessary?
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->getRemotePort();
}

address_t TCPSocketMgr::getLocalAddress(socket_id_t id)
{
	Enter_Method_Silent(); // TODO necessary?
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->getLocalAddress().str();
}

address_t TCPSocketMgr::getRemoteAddres(socket_id_t id)
{
	Enter_Method_Silent(); // TODO necessary?
	TCPSocket * socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->getRemoteAddress().str();
}

std::string TCPSocketMgr::socketToString(socket_id_t id)
{
	Enter_Method_Silent(); // TODO necessary?
	TCPSocket * socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->toString();
}

int TCPSocketMgr::socket (cb_inet_handler_ptr_t cbobj)
{
	Enter_Method_Silent();

	if (cbobj == NULL)
	{
		signalFunctionError(__FUNCTION__, "no callback handler specified");
	}

	socket_ptr_t socket = new TCPSocketExtension(this,this);
	socket->setOutputGate(gate("tcpOut"));

	int id = socket->getConnectionId();
	// TODO add the given cbobj to a mapping from socket id to callback handler
	_socket_map.addSocket(socket);
	printFunctionNotice(__FUNCTION__, socket->toString());

	return id;
}


void TCPSocketMgr::bind (socket_id_t id, address_cref_t local_address,
		port_t local_port)
{
	Enter_Method_Silent();

	// verify that socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	// check if port is available
	if (local_port != PORT_NULL)
	{
		Port_IdSetMap::iterator bprt_itr = _bound_ports.find(local_port);
		if (bprt_itr != _bound_ports.end())
		{
			signalFunctionError(__FUNCTION__, "port is being used");
		}
	}

	socket->bind(local_address, local_port);

	// mark the port as claimed
	if (local_port != PORT_NULL)
	{
		_bound_ports[local_port].insert(id);
	}

	printFunctionNotice(__FUNCTION__, socket->toString());
}

void TCPSocketMgr::connect (socket_id_t id, address_cref_t remote_address,
		port_t remote_port)
{
	this->connect(id, remote_address,remote_port,NULL);
}

void TCPSocketMgr::connect (socket_id_t id, address_cref_t remote_address,
		port_t remote_port, user_data_ptr_t context)
{
	Enter_Method_Silent();

	// verify that socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	socket->connect(remote_address, remote_port, context);

	printFunctionNotice(__FUNCTION__, socket->toString());
}


void TCPSocketMgr::listen (socket_id_t id, cb_inet_handler_ptr_t cbobj_for_accepted)
{
	Enter_Method_Silent();

	// verify that the socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	// creates a forking socket
	socket->listen(cbobj_for_accepted);

	// TODO add socket to a collection of passive sockets

	printFunctionNotice(__FUNCTION__, socket->toString());

	emitTCPConnInfo(socket);
}

socket_id_t TCPSocketMgr::makeActiveSocket (cb_base_handler_ptr_t cbobj,
	address_cref_t local_address, port_t local_port,
	address_cref_t remote_address, port_t remote_port)
{
	return this->makeActiveSocket(cbobj, local_address,local_port,remote_address,remote_port,NULL);
}

socket_id_t TCPSocketMgr::makeActiveSocket (cb_base_handler_ptr_t cbobj,
	address_cref_t local_address, port_t local_port,
	address_cref_t remote_address, port_t remote_port, user_data_ptr_t context)
{
	Enter_Method_Silent();
	int id = TCPSocketAPI_Inet::socket(cbobj);
	// check if bind should be skipped
	if (!local_address.empty() || 0 <= local_port)
	{
		bind(id, local_address, local_port);
	}
	connect(id, remote_address, remote_port, context);

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

void TCPSocketMgr::accept (socket_id_t id, void * context)
{
	Enter_Method_Silent();

	// verify that the socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	socket->accept(context);

	printFunctionNotice(__FUNCTION__, socket->toString());
}

// currently designed to allow sending even while the socket is
// still connecting or the other end of the connection closed
void TCPSocketMgr::send (socket_id_t id, cPacket * msg)
{
	Enter_Method_Silent();

	// verify that the socket exits
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	// take ownership of the message
	take(msg);

	// emit on the message event signal
	emitMessageEvent(msg, id);

	socket->send(msg);

	printFunctionNotice(__FUNCTION__, socket->toString() + " sent message "+msg->getName());
}

void TCPSocketMgr::recv (socket_id_t id, bytecount_t byte_mode)
{
	recv(id, byte_mode, NULL);
}

void TCPSocketMgr::recv (socket_id_t id, bytecount_t byte_mode, user_data_ptr_t context)
{
	Enter_Method_Silent();

	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	socket->recv(byte_mode, context);

	printFunctionNotice(__FUNCTION__, socket->toString());
}

void TCPSocketMgr::setTimeout(socket_id_t id, simtime_t timeout_period)
{
	Enter_Method_Silent();

	// verify that the socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	socket->setTimeout(timeout_period);
}

bool TCPSocketMgr::removeTimeout(socket_id_t id)
{
	Enter_Method_Silent();

	// verify that the socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->removeTimeout();
}

void * TCPSocketMgr::close (socket_id_t id)
{
	Enter_Method_Silent();

	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	removeTimeout(id);
	freePort(id);

	// throws error if close() has already been called
	socket->close();

	printFunctionNotice(__FUNCTION__, socket->toString());

	return socket->getUserContext();
}

void * TCPSocketMgr::getMyPtr(socket_id_t id)
{
	socket_ptr_t socket = _socket_map.getSocket(id);

	return (socket!=NULL) ? socket->getUserContext() : NULL;
}

//==============================================================================
// TCPSocket::CallbackInterface functions
// CONTINUE: remove these functions and replace with the Inet::CallbackHandler functions
// so as to intercept socket events and shuffle around the accepted socket pointers and
// track the bound ports

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
