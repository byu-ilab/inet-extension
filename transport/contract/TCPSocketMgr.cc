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
#include "TCPSocketExtension.h"
#include "DuplicateHttpMessageNameObserver.h"
#include "TCPConnInfoMappingObserver.h"

// From omnetpp extension
#include <omnetppextension.h>

// From standard C++ libraries
#include <iostream>
#include <sstream>

#define DEBUG_CLASS false

Define_Module(TCPSocketMgr);

//==============================================================================
// Constructor and Destructor

TCPSocketMgr::TCPSocketMgr()
	: _socket_pool(), _app_cb_handler_map(), _bound_ports(),
	  _passive_socket_map(), _pending_socket_pool()
{
	LOG_DEBUG_FUN_BEGIN(getFullPath());

	LOG_DEBUG_FUN_END("");
}

TCPSocketMgr::~TCPSocketMgr()
{
	LOG_DEBUG_FUN_BEGIN(getFullPath());

	_socket_pool.deleteSockets();
	_pending_socket_pool.deleteSockets();

	LOG_DEBUG_FUN_END("");
}

//==============================================================================
// cSimpleModule functions

void TCPSocketMgr::initialize()
{
	cSimpleModule::initialize();

	LOG_DEBUG_FUN_BEGIN(getFullPath());

	// other variables, scalars/vectors WATCH calls

	_should_map_tcp_connections = par("shouldMapTCPConnections");
	LOG_DEBUG_APPEND_LN("should map tcp connections: "<<
			(_should_map_tcp_connections ? "true":"false"));

	if (_should_map_tcp_connections)
	{
		TCPConnInfoMappingObserver::getInstance()->subscribeOnDefaultSignal(this);
	}

	_should_track_dup_msg_names = par("shouldTrackDuplicateMessageNames");
	LOG_DEBUG_APPEND_LN("should track duplicate message names: "<<
			(_should_track_dup_msg_names ? "true":"false"));

	if (_should_track_dup_msg_names)
	{
		DuplicateHttpMessageNameObserver::getInstance()->subscribeOnDefaultSignal(this);
	}

	LOG_DEBUG_FUN_END("");
}

void TCPSocketMgr::handleMessage(cMessage *msg)
{
	ASSERT(msg != NULL);

	LOG_DEBUG_FUN_BEGIN(getFullPath());

	socket_ptr_t socket = NULL;

	LOG_DEBUG_LN("Handle inbound message " << msg->getName()
					<< " of kind " << msg->getKind());

	RecvCallbackMsg * rmsg = NULL;
	if (rmsg = (dynamic_cast<RecvCallbackMsg *>(msg))) {
		handleRecvCallback(rmsg->getSocketId());
		delete rmsg;
		return;
	}

	// if self message then it is a timeout
	if (msg->isSelfMessage())
	{
		handleTimeoutMessage(msg); // takes responsibility for the message
		cleanupClosedSockets();
		LOG_DEBUG_FUN_END("");
		return;
	}
	// else if socket is current process the message on it
	socket = findSocketFor(_socket_pool, msg);
	if (socket != NULL)
	{
		LOG_DEBUG_APPEND_LN("Socket found for the message.  Processing message.");
		socket->processMessage(msg); // takes responsibility for the message
		cleanupClosedSockets();
		LOG_DEBUG_FUN_END("");
		return;
	}
	// else if msg is TCP_I_ESTABLISHED must be to accept a new socket
	if (msg->getKind() == TCP_I_ESTABLISHED)
	{
		handleAcceptedMessage(msg); // takes responsibility for the message
		cleanupClosedSockets();
		LOG_DEBUG_FUN_END("accepted new socket");
		return;
	}
	// else if msg is TCP_I_CLOSED or TCP_I_PEER_CLOSED absorb the message since
	// the socket was closed
	if (msg->getKind() == TCP_I_CLOSED || msg->getKind() == TCP_I_PEER_CLOSED)
	{

		DEBUG_BLOCK(
			// extract the connection id
			TCPCommand * cmd = check_and_cast<TCPCommand *>(msg->getControlInfo());
			LOG_DEBUG_LN("received message for a closed socket "<<cmd->getConnId());
					);
		// absorb it
		delete msg;
		LOG_DEBUG_FUN_END("absorbed close message");
		return;
	}
	// else is an erroneous message
	LOG_DEBUG_FUN_END("absorbed erroneous message: "<<msg->getName());
	delete msg;
}

void TCPSocketMgr::handleTimeoutMessage(cMessage *msg)
{
	LOG_DEBUG_FUN_BEGIN(getFullPath());

	LOG_DEBUG_APPEND_LN("Checking if timer.");

	// if not, unknown self message received
	SocketTimeoutMsg * timer = check_and_cast<SocketTimeoutMsg *>(msg);

	LOG_DEBUG_APPEND_LN("Timer received.")

	socket_id_t socket_id = timer->getSocketId();

	LOG_DEBUG_APPEND_LN("Timer received for socket "<<socket_id);

	socket_ptr_t socket = getSocket(_socket_pool, socket_id);

	ASSERT(socket != NULL); // if not, timer doesn't belong to a current socket

	LOG_DEBUG_APPEND_LN("On the socket: " << socket->toString());
	LOG_DEBUG_APPEND_LN("Process the message " << msg->getName());
	socket->processMessage(msg); // takes responsibility for the message

	LOG_DEBUG_FUN_END(socket->toString());
}

void TCPSocketMgr::handleAcceptedMessage(cMessage *msg)
{
	LOG_DEBUG_FUN_BEGIN("Accepting new socket...");

	ASSERT(msg->getKind() == TCP_I_ESTABLISHED);

	TCPConnectInfo * info = check_and_cast<TCPConnectInfo *>(msg->getControlInfo());

	Port_SocketMap::iterator psitr = _passive_socket_map.find(info->getLocalPort());

	if (psitr != _passive_socket_map.end())
	{
		socket_ptr_t socket = new TCPSocketExtension(msg);
		socket->setOutputGate(gate("tcpOut"));
		_pending_socket_pool.addSocket(socket);
		_app_cb_handler_map[socket->getConnectionId()] =
				_accepted_cb_handler_map[psitr->second->getLocalPort()];

		LOG_DEBUG_LN("accepted: "<<socket->toString());

		psitr->second->appendAcceptedSocket(socket);
	}
	else
	{
		LOG_DEBUG_LN("received message for connection " << info->getConnId()
			<< " on non-listening port "<< info->getLocalPort() );
	}

	delete msg;

	LOG_DEBUG_FUN_END("");
}

void TCPSocketMgr::finish()
{
	// record scalars if any
}

//==============================================================================
// TCP Socket Port, Address, and String Accessors

port_t TCPSocketMgr::getLocalPort(socket_id_t id)
{
//	Enter_Method_Silent(); // TODO necessary?
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->getLocalPort();
}

port_t TCPSocketMgr::getRemotePort(socket_id_t id)
{
//	Enter_Method_Silent(); // TODO necessary?
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->getRemotePort();
}

address_t TCPSocketMgr::getLocalAddress(socket_id_t id)
{
//	Enter_Method_Silent(); // TODO necessary?
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->getLocalAddress().str();
}

address_t TCPSocketMgr::getRemoteAddress(socket_id_t id)
{
//	Enter_Method_Silent(); // TODO necessary?
	TCPSocket * socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->getRemoteAddress().str();
}

str_t TCPSocketMgr::socketToString(socket_id_t id)
{
//	Enter_Method_Silent(); // TODO necessary?
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	return socket->toString();
}

//==============================================================================
// BDS-like Socket Operations

socket_id_t TCPSocketMgr::socket (cb_base_handler_ptr_t cb_handler)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN(getFullPath());

	// checks that the pointer is non-NULL
	cb_inet_handler_ptr_t cb_handler_mod = verifyCallbackHandlerType(cb_handler);

	socket_ptr_t socket = new TCPSocketExtension(this,this);
	socket->setOutputGate(gate("tcpOut"));

	socket_id_t id = socket->getConnectionId();
	_app_cb_handler_map[id] = cb_handler_mod;

	_socket_pool.addSocket(socket);

	LOG_DEBUG_FUN_END(socket->toString());

	return id;
}


void TCPSocketMgr::bind (socket_id_t id, address_cref_t local_address,
		port_t local_port)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN(getFullPath());

	// verify that socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	// check if port is available
	if (local_port != PORT_NULL)
	{
		Port_IdSetMap::iterator bprt_itr = _bound_ports.find(local_port);
		if (bprt_itr != _bound_ports.end())
		{
			DEBUG_BLOCK(
					SocketIDSet::iterator isitr = bprt_itr->second.begin();
					LOG_DEBUG_APPEND("sockets on port "<<local_port<<":")
					for ( ; isitr != bprt_itr->second.end(); isitr++ )
					{
						LOG_DEBUG_APPEND(" "<<*isitr);
					}
					LOG_DEBUG_APPEND_LN("");
						);
			throw cRuntimeError(this, "%s: port %d is being used by %d other sockets", __FUNCTION__, local_port, bprt_itr->second.size());
		}
	}

	socket->bind(local_address, local_port);

	// mark the port as claimed
	if (local_port != PORT_NULL)
	{
		_bound_ports[local_port].insert(id);
	}

	LOG_DEBUG_FUN_END(socket->toString());
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

	LOG_DEBUG_FUN_BEGIN(getFullPath());

	// verify that socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	socket->connect(remote_address, remote_port, context);

	LOG_DEBUG_FUN_END(socket->toString());
}


void TCPSocketMgr::listen (socket_id_t id, cb_base_handler_ptr_t cbobj_for_accepted)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN(getFullPath());

	// verify that the socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	// creates a forking socket
	socket->listen(NULL);

	// add to map of passive sockets
	_passive_socket_map[socket->getLocalPort()] = socket;

	// register the cbobj_for_accepted handler with this passive socket's port
	if (cbobj_for_accepted != NULL)
	{
		_accepted_cb_handler_map[socket->getLocalPort()] =
				verifyCallbackHandlerType(cbobj_for_accepted);
	}
	else
	{
		_accepted_cb_handler_map[socket->getLocalPort()] = _app_cb_handler_map[id];
	}

	LOG_DEBUG_FUN_END(socket->toString());

	emitTCPConnInfo(socket);
}

void TCPSocketMgr::accept (socket_id_t id)
{
	accept(id, NULL);
}

void TCPSocketMgr::accept (socket_id_t id, void * context)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN(getFullPath());

	// verify that the socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	// call accept on it
	socket->accept(context);

	LOG_DEBUG_FUN_END(socket->toString());
}

// currently designed to allow sending even while the socket is
// still connecting or the other end of the connection closed
void TCPSocketMgr::send (socket_id_t id, cPacket * msg)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN(getFullPath());

	// verify that the socket exits
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	// take ownership of the message
	if (msg == NULL)
	{
		throw cRuntimeError(this, "Message must not be NULL.");
	}
	take(msg);

	// emit on the message event signal
	emitMessageEvent(msg, id);

	// send the message
	socket->send(msg);

	LOG_DEBUG_FUN_END(socket->toString() + " sent message "+msg->getName());
}

void TCPSocketMgr::recv (socket_id_t id, bytecount_t byte_mode)
{
	recv(id, byte_mode, NULL);
}

void TCPSocketMgr::recv (socket_id_t id, bytecount_t byte_mode, user_data_ptr_t context)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN(getFullPath());

	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	socket->recv(byte_mode, context);

	LOG_DEBUG_FUN_END(socket->toString());
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

user_data_ptr_t TCPSocketMgr::close (socket_id_t id)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN(getFullPath());

	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	LOG_DEBUG_APPEND_LN("closing socket: "<<socket->toString());

	freePort(id);

	user_data_ptr_t user_context = socket->getUserContext();

	//LOG_DEBUG_FUN_END(socket->toString());

	// add the socket to the set of sockets to be deleted
	_socket_pool.removeSocket(socket);
	_closed_sockets.push_back(socket);

	// if the socket is a passive socket then remove it from
	// the passive socket map
	if (socket->getState() == TCPSocket::LISTENING || socket->getState() == TCPSocket::ACCEPTING)
	{
		Port_SocketMap::iterator psitr = _passive_socket_map.find(socket->getLocalPort());
		ASSERT( psitr != _passive_socket_map.end() );
		_passive_socket_map.erase(psitr);
	}

	// - removes any timeouts
	// - throws error if close() has already been called
	socket->close();

	LOG_DEBUG_FUN_END(socket->toString());

	return user_context;
}

//==============================================================================
// User Context Data Access

void * TCPSocketMgr::getMyPtr(socket_id_t id)
{
	socket_ptr_t socket = getSocket(_socket_pool, id);

	return (socket!=NULL) ? socket->getUserContext() : NULL;
}

//==============================================================================
// TCPSocketAPI_Inet::CallbackHandler functions

void TCPSocketMgr::acceptCallback  (socket_id_t id, cb_status_t result,
		user_data_ptr_t context)
{
	ASSERT(findAndCheckSocket(id, __FUNCTION__) != NULL);

	if (!isCallbackError(result))
	{
		// move accepted socket from pending pool to active/current pool
		socket_ptr_t accepted_socket = removeSocket(_pending_socket_pool, result);
		ASSERT(accepted_socket != NULL); // if not, was not in the pending socket pool
		_socket_pool.addSocket(accepted_socket);

		// add entry to bound ports
		_bound_ports[accepted_socket->getLocalPort()].insert(result);
	}

	// forward callback
	_app_cb_handler_map[id]->acceptCallback(id, result, context);
}

void TCPSocketMgr::connectCallback (socket_id_t id, cb_status_t result,
		user_data_ptr_t context)
{
	ASSERT(findAndCheckSocket(id, __FUNCTION__) != NULL);

	if (!isCallbackError(result))
	{
		// add entry to bound ports
		socket_ptr_t socket = getSocket(_socket_pool, id);
		_bound_ports[socket->getLocalPort()].insert(id);
	}

	// forward callback
	_app_cb_handler_map[id]->connectCallback(id, result, context);
}

void TCPSocketMgr::recvCallback(socket_id_t id, cb_status_t result,
		cPacket * msg, user_data_ptr_t context)
{
	ASSERT(findAndCheckSocket(id, __FUNCTION__) != NULL);

	// forward callback
	_app_cb_handler_map[id]->recvCallback(id, result, msg, context);
}
void TCPSocketMgr::scheduleRecvCallback(TCPSocketExtension * sock) {
	int conn = sock->getConnectionId();

	RecvCallbackMsg * rc = new RecvCallbackMsg();
	rc->setSocketId(conn);
	scheduleAt(simTime(),rc);
	//_app_cb_handler_map[conn]->scheduleRecvCallback(sock);
}
void TCPSocketMgr::handleRecvCallback(int socket_id) {
	TCPSocketExtension * socket = getSocket(socket_id);
	socket->doRecvCallback();
}

//void TCPSocketMgr::closeCallback (socket_id_t id, cb_status_t result,
//					user_data_ptr_t context)
//{
//	cleanupSocket(id);
//
//	// forward callback
//	_app_cb_handler_map[id]->closeCallback(id, result, context);
//}

//==============================================================================
// Utilities

//void TCPSocketMgr::cleanupSocket(socket_id_t id)
//{
//	freePort(id);
//	removeTimeout(id);
//
//	// delete the socket
//	socket_ptr_t socket = removeSocket(_socket_pool, id);
//	if (socket != NULL)
//	{
//		delete socket;
//	}
//}

void TCPSocketMgr::cleanupClosedSockets()
{
	// the port and timeout message should have been previously deallocated
	SocketList::iterator csitr = _closed_sockets.begin();
	while (csitr != _closed_sockets.end())
	{
		delete (*csitr);
		_closed_sockets.erase(csitr);
		csitr = _closed_sockets.begin();
	}
}

void TCPSocketMgr::freePort(socket_id_t id)
{
	socket_ptr_t socket = getSocket(_socket_pool, id);

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

	SocketIDSet::iterator id_itr = bprt_itr->second.find(id);
	if (id_itr != bprt_itr->second.end())
	{
		bprt_itr->second.erase(id_itr);
	}
}

socket_ptr_t TCPSocketMgr::findAndCheckSocket(socket_id_t id, str_cref_t fname)
{
	socket_ptr_t socket = getSocket(_socket_pool, id);
	if (socket == NULL)
	{
		throw cRuntimeError(this, "%s: invalid socket id %d", fname.c_str(), id);
	}
	// else
	return socket;
}

socket_ptr_t TCPSocketMgr::getSocket(socket_id_t id) {
	return dynamic_cast<socket_ptr_t>(_socket_pool.getSocket(id));
}
socket_ptr_t TCPSocketMgr::getSocket(TCPSocketMap & pool, socket_id_t id)
{
	return dynamic_cast<socket_ptr_t>(pool.getSocket(id));
}

socket_ptr_t TCPSocketMgr::removeSocket(TCPSocketMap & pool, socket_id_t id)
{
	return dynamic_cast<socket_ptr_t>(pool.removeSocket(id));
}

socket_ptr_t TCPSocketMgr::findSocketFor(TCPSocketMap & pool, cMessage * msg)
{
	return dynamic_cast<socket_ptr_t>(pool.findSocketFor(msg));
}

void TCPSocketMgr::emitTCPConnInfo(socket_ptr_t socket)
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
