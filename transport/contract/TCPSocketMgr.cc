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

//==============================================================================
// Constructor and Destructor

TCPSocketMgr::TCPSocketMgr()
	: _socket_pool(), _app_cb_handler_map(), _bound_ports(),
	  _passive_socket_map(), _pending_socket_pool()
{
}

TCPSocketMgr::~TCPSocketMgr()
{
	_socket_pool.deleteSockets();
	_pending_socket_pool.deleteSockets();
}

//==============================================================================
// cSimpleModule functions

void TCPSocketMgr::initialize()
{
	cSimpleModule::initialize();

	LOG_DEBUG_FUN_BEGIN("");

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
}

void TCPSocketMgr::handleMessage(cMessage *msg)
{
	ASSERT(msg != NULL);

	LOG_DEBUG_FUN_BEGIN("");

	socket_ptr_t socket = NULL;

	if (msg->isSelfMessage())
	{
		SocketTimeoutMsg * timer = check_and_cast<SocketTimeoutMsg *>(msg);
		ASSERT(timer != NULL); // if not, unknown self message received

		socket_id_t socket_id = timer->getSocketId();
		socket = getSocket(_socket_pool, socket_id);

		ASSERT(socket != NULL); // if not, timer doesn't belong to a current socket

		LOG_DEBUG_LN("On the socket: " << socket->toString());
		LOG_DEBUG_LN("Process the message " << msg->getName());
		socket->processMessage(msg);
	}
	else
	{
		LOG_DEBUG_LN("Handle inbound message " << msg->getName()
				<< " of kind " << msg->getKind());

		socket = findSocketFor(_socket_pool, msg);

		if (socket == NULL)
		{
			// Accept a new connection
			LOG_DEBUG_LN("No socket found for the message. Create a new one");

			socket = new TCPSocketExtension(msg);
			socket->setOutputGate(gate("tcpOut"));

			socket_ptr_t passive_socket = _passive_socket_map[socket->getLocalPort()];

			if (passive_socket == NULL)
			{
				LOG_DEBUG_LN("received message for connection "
						<< socket->getConnectionId() << " on non-listening port"
						<< " OR received a message for a closed socket");
				delete socket;
				delete msg;
				return;
			}
			// else
			ASSERT(msg->getKind() == TCP_I_ESTABLISHED); // if not, shouldn't be accepting
			_pending_socket_pool.addSocket(socket);
			passive_socket->appendAcceptedSocket(socket);
		}
	}

	// update display?
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

int TCPSocketMgr::socket (cb_inet_handler_ptr_t cb_handler)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN("");

	ASSERT(cb_handler != NULL);

	socket_ptr_t socket = new TCPSocketExtension(this,this);
	socket->setOutputGate(gate("tcpOut"));

	int id = socket->getConnectionId();
	_app_cb_handler_map[id] = cb_handler;

	_socket_pool.addSocket(socket);

	LOG_DEBUG_FUN_END(socket->toString());

	return id;
}


void TCPSocketMgr::bind (socket_id_t id, address_cref_t local_address,
		port_t local_port)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN("");

	// verify that socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	// check if port is available
	if (local_port != PORT_NULL)
	{
		Port_IdSetMap::iterator bprt_itr = _bound_ports.find(local_port);
		if (bprt_itr != _bound_ports.end())
		{
			throw cRuntimeError(this, "%s: %s", __FUNCTION__, "port is being used");
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

	LOG_DEBUG_FUN_BEGIN("");

	// verify that socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	socket->connect(remote_address, remote_port, context);

	LOG_DEBUG_FUN_END(socket->toString());
}


void TCPSocketMgr::listen (socket_id_t id, cb_inet_handler_ptr_t cbobj_for_accepted)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN("");

	// verify that the socket exists
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	// creates a forking socket
	socket->listen(cbobj_for_accepted);

	// add to map of passive sockets
	_passive_socket_map[socket->getLocalPort()] = socket;

	LOG_DEBUG_FUN_END(socket->toString());

	emitTCPConnInfo(socket);
}

socket_id_t TCPSocketMgr::makeActiveSocket (cb_base_handler_ptr_t cbobj,
	address_cref_t local_address, port_t local_port,
	address_cref_t remote_address, port_t remote_port)
{
	return this->makeActiveSocket(cbobj, local_address,local_port,
			remote_address,remote_port,NULL);
}

socket_id_t TCPSocketMgr::makeActiveSocket (cb_base_handler_ptr_t cb_handler,
	address_cref_t local_address, port_t local_port,
	address_cref_t remote_address, port_t remote_port, user_data_ptr_t context)
{
	Enter_Method_Silent();

	int id = TCPSocketAPI_Inet::socket(cb_handler);
	// check if bind should be skipped
	if (!local_address.empty() || 0 <= local_port)
	{
		bind(id, local_address, local_port);
	}
	connect(id, remote_address, remote_port, context);

	return id;
}

socket_id_t TCPSocketMgr::makePassiveSocket (cb_base_handler_ptr_t cb_handler,
	address_cref_t local_address, port_t local_port,
	cb_base_handler_ptr_t cb_handler_for_accepted)
{
	Enter_Method_Silent();

	int id = TCPSocketAPI_Inet::socket(cb_handler);

	bind(id, local_address, local_port);

	TCPSocketAPI_Inet::listen(id, cb_handler_for_accepted);

	return id;
}

void TCPSocketMgr::accept (socket_id_t id)
{
	accept(id, NULL);
}

void TCPSocketMgr::accept (socket_id_t id, void * context)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN("");

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

	LOG_DEBUG_FUN_BEGIN("");

	// verify that the socket exits
	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	// take ownership of the message
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

	LOG_DEBUG_FUN_BEGIN("");

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

	LOG_DEBUG_FUN_BEGIN("");

	socket_ptr_t socket = findAndCheckSocket(id, __FUNCTION__);

	freePort(id);

	user_data_ptr_t user_context = socket->getUserContext();

	LOG_DEBUG_FUN_END(socket->toString());

	// - removes any timeouts
	// - throws error if close() has already been called
	// - may invoke closeCallback immediately which will delete the socket
	// so all interaction with the socket must be done previously
	socket->close();

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
	// forward callback
	_app_cb_handler_map[id]->recvCallback(id, result, msg, context);
}

void TCPSocketMgr::closeCallback (socket_id_t id, cb_status_t result,
					user_data_ptr_t context)
{
	cleanupSocket(id);

	// forward callback
	_app_cb_handler_map[id]->closeCallback(id, result, context);
}

//==============================================================================
// Utilities

void TCPSocketMgr::cleanupSocket(socket_id_t id)
{
	freePort(id);
	removeTimeout(id);

	// delete the socket
	socket_ptr_t socket = removeSocket(_socket_pool, id);
	if (socket != NULL)
	{
		delete socket;
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

	IdSet::iterator id_itr = bprt_itr->second.find(id);
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
