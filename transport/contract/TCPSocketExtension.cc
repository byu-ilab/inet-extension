//==========================================================================80>
/**
 * @file TCPSocketExtension.cc
 *
 * TCPSocketExtension class definitions.
 *
 * Created: July 28, 2010
 *
 * @todo Add GPL notice.
 */

// From inet
#include "TCPSocketExtension.h"

// From omnetpp extension
#include <omnetppextension.h>

// From standard C++ libraries
#include <sstream>

#define OPP_ERROR(DETAILS) opp_error("%s::%s: %s.",__FILE__,__FUNCTION__,DETAILS)
#define OPP_ERROR_INCONSISTENT_STATE opp_error("%s::%s inconsistent state %s",\
		__FILE__, __FUNCTION__, stateName(sockstate))

#define DEBUG_CLASS true

//==============================================================================
// Initialization

void TCPSocketExtension::initialize(cb_inet_handler_ptr_t handler,
		cSimpleModule * scheduler, IPAddressResolver::ResolutionMode mode)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	_cb_handler = handler;
	// setCallbackHandler(handler); // just in case handler is non-NULL
	_cb_handler_for_accepted = NULL;
	_user_context_data = NULL;
	_timeout_scheduler = scheduler;
	_timeout_msg = NULL;
	_resolution_mode = mode;

	LOG_DEBUG_FUN_END(toString());
}

//==============================================================================
// Constructors and Destructor

TCPSocketExtension::TCPSocketExtension() : TCPSocket()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	initialize(NULL, NULL, IPAddressResolver::ADDR_PREFER_IPv4);

	LOG_DEBUG_FUN_END(toString());
}

TCPSocketExtension::TCPSocketExtension(cMessage * msg) : TCPSocket(msg)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	initialize(NULL, NULL, IPAddressResolver::ADDR_PREFER_IPv4);

	LOG_DEBUG_FUN_END(toString());
}

TCPSocketExtension::TCPSocketExtension(TCPSocketAPI_Inet::CallbackHandler * handler,
		cSimpleModule * scheduler, IPAddressResolver::ResolutionMode mode)
	: TCPSocket()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	initialize(handler, scheduler, mode);

	LOG_DEBUG_FUN_END(toString());
}

TCPSocketExtension::~TCPSocketExtension()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	// DO NOT delete _cb_handler or _timeout_scheduler

	LOG_DEBUG_FUN_END(toString());
}

//==============================================================================
// BDS-like Socket Operations

void TCPSocketExtension::bind(address_cref_t local_address, port_t local_port)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	ASSERT(_cb_handler != NULL);

	if (local_address.empty())
	{
		// Checks socket state and port number
		TCPSocket::bind(local_port);
	}
	else
	{
		// Checks socket state and port number, allows for only the
		// address to be specified if port = PORT_NULL, but address must
		// thus be valid.
		TCPSocket::bind(_resolver.resolve(local_address.c_str(), _resolution_mode), local_port);
	}
	// fact: sockstate == BOUND

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::listen (cb_inet_handler_ptr_t cb_handler_for_accepted)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	ASSERT(_cb_handler != NULL);

	// Verify that the port has been specified
	if (localPrt == PORT_NULL)
	{
		OPP_ERROR("local port must be specified");
	}

	// Checks the state
	TCPSocket::listen();

	// fact: sockstate == LISTENING
	if (cb_handler_for_accepted != NULL)
	{
		_cb_handler_for_accepted = cb_handler_for_accepted;
	}
	else
	{
		_cb_handler_for_accepted = _cb_handler;
	}

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::listenOnce(cb_inet_handler_ptr_t cb_handler_for_accepted)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	ASSERT(_cb_handler != NULL);

	// Verify that the port has been specified
	if (localPrt == PORT_NULL)
	{
		OPP_ERROR("local port must be specified");
	}

	// Checks the state
	TCPSocket::listenOnce();

	// fact: sockstate == LISTENING
	if (cb_handler_for_accepted != NULL)
	{
		_cb_handler_for_accepted = cb_handler_for_accepted;
	}
	else
	{
		_cb_handler_for_accepted = _cb_handler;
	}

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::accept ()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	ASSERT(_cb_handler != NULL);

	if (sockstate != LISTENING && sockstate != ACCEPTING)
	{
		OPP_ERROR("socket is not listening");
	}
	// fact: else sockstate == LISTENING || sockstate == ACCEPTING
	if (sockstate == ACCEPTING)
	{
		OPP_ERROR("socket is already set to accept");
	}
	// fact: else sockstate == LISTENING and it can be scheduled to accept
	sockstate = ACCEPTING;

	if (!_sockets_pending_acceptance.empty())
	{
		// notify the user of acceptance immediately
		processEstablished();
	}

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::accept (user_data_ptr_t context)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	// context data must be set first in the event that there is a connection
	// in the pending connections queue
	_user_context_data = context;
	this->accept();

	LOG_DEBUG_FUN_END(toString());
}


void TCPSocketExtension::appendAcceptedSocket(TCPSocketExtension * socket)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	ASSERT(_cb_handler != NULL);

	if (sockstate != LISTENING && sockstate != ACCEPTING)
	{
		OPP_ERROR("socket is not a passive socket");
	}

	ASSERT(socket->_cb_handler == NULL); // if not, not created correctly via msg constructor
	socket->setCallbackHandler(_cb_handler_for_accepted);

	_sockets_pending_acceptance.push_back(socket);

	if (sockstate == ACCEPTING)
	{
		processEstablished();
	}

	LOG_DEBUG_FUN_END(toString());
}


void TCPSocketExtension::connect(address_cref_t remote_address, port_t remote_port)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	ASSERT(_cb_handler != NULL);

	// Check here because this isn't checked until the TCP core processes the
	// OPEN_ACTIVE command.
	if (remote_address.empty())
	{
		OPP_ERROR("remote address must be specified.");
	}

	// Checks the port but not the address
	TCPSocket::connect(_resolver.resolve(remote_address.c_str(), _resolution_mode),
			remote_port);

	// fact: sockstate == CONNECTING

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::connect (address_cref_t remote_address,
		port_t remote_port, user_data_ptr_t context)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	_user_context_data = context;
	this->connect(remote_address, remote_port);

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::send(cMessage *msg)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	ASSERT(_cb_handler != NULL);

	if (msg == NULL)
	{
		OPP_ERROR("cannot send a NULL message");
	}

	// remove any control info with the message
	cObject * ctrl_info = msg->removeControlInfo();
	if (ctrl_info)
	{
		delete ctrl_info;
	}

	TCPSocket::send(msg);

	LOG_DEBUG_FUN_END(toString());
}


void TCPSocketExtension::recv (bytecount_t byte_mode)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	ASSERT(_cb_handler != NULL);

	if (sockstate != CONNECTED && sockstate != RECEIVING)
	{
		OPP_ERROR("socket is not connected");
	}
	// fact: else sockstate == CONNECTED || sockstate == RECEIVING
	if (sockstate == RECEIVING)
	{
		OPP_ERROR("socket is already set to receive");
	}

	_recv_mode = byte_mode;

	// relative to the byte_mode return something out of the buffer
	cPacket * ret_msg = _recv_buffer.extractAvailableBytes(_recv_mode);
	if (ret_msg != NULL)
	{
		// no change in state
		_cb_handler->recvCallback(connId, ret_msg->getByteLength(), ret_msg, removeUserContext());
	}
	else if (_timeout_msg != NULL)
	{
		_timeout_scheduler->scheduleAt(simTime()+_timeout_msg->getTimeoutInterval(), _timeout_msg);
	}

	LOG_DEBUG_FUN_END(toString());
}


void TCPSocketExtension::recv (bytecount_t byte_mode,
		user_data_ptr_t context)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	_user_context_data = context;
	this->recv(byte_mode);

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::setTimeout(simtime_t timeout_period)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	ASSERT(timeout_period >= 0);
	ASSERT(_timeout_scheduler != NULL);

	if (!canModifyTimeout())
	{
		OPP_ERROR("socket state won't allow timeout to be set");
	}

	if (_timeout_msg == NULL)
	{
		_timeout_msg = new SocketTimeoutMsg("socket timeout");
		_timeout_msg->setSocketId(connId);
	}

	ASSERT(!_timeout_msg->isScheduled());

	// set period on timer
	_timeout_msg->setTimeoutInterval(timeout_period.dbl());

	LOG_DEBUG_FUN_END(toString());
}


bool TCPSocketExtension::removeTimeout ()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	if (_timeout_msg == NULL)
	{
		LOG_DEBUG_FUN_END(toString());
		return false;
	}

	ASSERT(_timeout_scheduler != NULL);
	if (!canModifyTimeout())
	{
		OPP_ERROR("socket state won't allow timeout to be removed");
	}

	_timeout_scheduler->cancelAndDelete(_timeout_msg);
	_timeout_msg = NULL;

	LOG_DEBUG_FUN_END(toString());

	return true;
}

bool TCPSocketExtension::canModifyTimeout() const
{
	LOG_DEBUG_FUN_BEGIN(toString());

	if (	   sockstate == NOT_BOUND
			|| sockstate == BOUND
			|| sockstate == CONNECTING
			|| sockstate == CONNECTED)
	{
		LOG_DEBUG_FUN_END(toString());
		return true;
	}
	// else
	LOG_DEBUG_FUN_END(toString());
	return false;
}

void TCPSocketExtension::close ()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	ASSERT(_cb_handler != NULL);

	if (sockstate == CLOSED || sockstate == LOCALLY_CLOSED)
	{
		LOG_DEBUG_LN("Already called on this socket.");
		return;
	}
	// else
	removeTimeout();

	if (sockstate != NOT_BOUND && sockstate != BOUND && sockstate != SOCKERROR)
	{
		// send off close command to other end of the connection
		TCPSocket::close();
	}
	else
	{
		processClosed();
	}

	LOG_DEBUG_FUN_END(toString());
}

user_data_ptr_t TCPSocketExtension::getUserContext ()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	LOG_DEBUG_FUN_END(toString());
	return _user_context_data;
}

user_data_ptr_t TCPSocketExtension::removeUserContext ()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	user_data_ptr_t context = _user_context_data;
	_user_context_data = NULL;

	LOG_DEBUG_FUN_END(toString());
	return context;
}

bool TCPSocketExtension::setTimeoutScheduler(cSimpleModule * scheduler)
{
	LOG_DEBUG_FUN_BEGIN(toString());
	if (_timeout_scheduler == NULL)
	{
		_timeout_scheduler = scheduler;
		LOG_DEBUG_FUN_END(toString());
		return true;
	}
	LOG_DEBUG_FUN_END(toString());
	return false;
}

void TCPSocketExtension::setAddressResolutionMode(IPAddressResolver::ResolutionMode mode)
{
	LOG_DEBUG_FUN_BEGIN(toString());
	_resolution_mode = mode;
	LOG_DEBUG_FUN_END(toString());
}

IPAddressResolver::ResolutionMode TCPSocketExtension::getAddressResolutionMode()
{
	LOG_DEBUG_FUN_BEGIN(toString());
	LOG_DEBUG_FUN_END(toString());
	return _resolution_mode;
}

void TCPSocketExtension::setCallbackObject(TCPSocket::CallbackInterface *cb, void *yourPtr)
{
	OPP_ERROR("not supported.  See setCallbackHandler()");
}

void TCPSocketExtension::setCallbackHandler(cb_inet_handler_ptr_t handler)
{
	LOG_DEBUG_FUN_BEGIN(toString());
	if (handler == NULL)
	{
		OPP_ERROR("callback handler must not be NULL");
	}
	_cb_handler = handler;
	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::processMessage (cMessage * msg)
{
	LOG_DEBUG_FUN_BEGIN(toString());
	ASSERT(_cb_handler != NULL);
	ASSERT(msg != NULL);

	if (dynamic_cast<SocketTimeoutMsg *>(msg) != NULL)
	{
		delete msg;
		processFailure(TCPSocketAPI_Base::CB_E_TIMEOUT);
		LOG_DEBUG_FUN_END(toString());
		return;
	}

	// only works if the message has TCPCommandInfo
	ASSERT(belongsToSocket(msg));

	TCPStatusInfo *status;
	TCPConnectInfo *connectInfo;
	switch (msg->getKind())
	{
	case TCP_I_DATA:
		processDataArrived(PK(msg), false);
		break;

	case TCP_I_URGENT_DATA:
		processDataArrived(PK(msg), true);
		break;

	case TCP_I_ESTABLISHED:
		// Note: this code is only for sockets doing active open, and nonforking
		// listening sockets. For a forking listening sockets, TCP_I_ESTABLISHED
		// carries a new connId which won't match the connId of this
		// TCPSocketExtension, so you won't get here. Rather, when you see
		// TCP_I_ESTABLISHED, you'll want to create a new TCPSocketExtension
		// object via new TCPSocketExtension(msg).

		connectInfo = dynamic_cast<TCPConnectInfo *>(msg->getControlInfo());
		localAddr = connectInfo->getLocalAddr();
		remoteAddr = connectInfo->getRemoteAddr();
		localPrt = connectInfo->getLocalPort();
		remotePrt = connectInfo->getRemotePort();
		// DO NOT delete connectInfo it was not removed from msg so msg will delete it
		delete msg;
		processEstablished();
		break;

	case TCP_I_PEER_CLOSED:
		delete msg;
		processPeerClosed();
		break;

	case TCP_I_CLOSED:
		delete msg;
		processClosed();
		break;

	case TCP_I_CONNECTION_REFUSED:
		delete msg;
		processFailure(TCPSocketAPI_Base::CB_E_REFUSED);
		break;

	case TCP_I_CONNECTION_RESET:
		delete msg;
		processFailure(TCPSocketAPI_Base::CB_E_RESET);
		break;

	case TCP_I_TIMED_OUT:
		delete msg;
		processFailure(TCPSocketAPI_Base::CB_E_TIMEOUT);
		break;

	case TCP_I_STATUS:
		status = check_and_cast<TCPStatusInfo *>(msg->removeControlInfo());
		delete msg;
		processStatusArrived(status);
		break;

	default:
		std::stringstream details;
		details << "invalid msg kind "<<msg->getKind()
					<<" one of the TCP_I_xxx constants expected";
		delete msg;
		OPP_ERROR(details.str().c_str());
	}
	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::processDataArrived(cPacket *msg, bool urgent)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	// depends on receiving mode
	if (sockstate == RECEIVING)
	{
		sockstate = CONNECTED;

		// cancel timeout if any
		if (_timeout_msg != NULL)
		{
			_timeout_scheduler->cancelEvent(_timeout_msg);
		}

		// add message to receive buffer
		_recv_buffer.insertData(msg);

		// get bytes to return
		cPacket * ret_msg = _recv_buffer.extractAvailableBytes(_recv_mode);

		if (ret_msg != NULL)
		{
			_cb_handler->recvCallback(connId, ret_msg->getByteLength(), ret_msg, removeUserContext());
		}
	}
	else if (sockstate == CONNECTED)
	{
		// add message to receive buffer
		_recv_buffer.insertData(msg);

		// no change in socket state
	}
	else
	{
		OPP_ERROR_INCONSISTENT_STATE;
	}

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::processEstablished()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	// invoke the "connect" or "accept" callback
	if (sockstate == CONNECTING)
	{
		sockstate = CONNECTED;
		_cb_handler->connectCallback(connId, 0, removeUserContext());
	}
	else if (sockstate == ACCEPTING)
	{
		ASSERT(!_sockets_pending_acceptance.empty());

		TCPSocketExtension * socket = _sockets_pending_acceptance.front();
		_sockets_pending_acceptance.pop_front();
		// DO NOT delete the socket, it should be being stored by the application
		sockstate = LISTENING;

		_cb_handler->acceptCallback(connId, socket->getConnectionId(), removeUserContext());
	}
	else if (sockstate == LISTENING)
	{
		// Socket already added to the pending connections in appendAcceptedSocket()
	}
	//	else if (sockstate == CLOSED)
	//	{
	//		// remove the socket identified by connId from the pending connections pool?
	//	}
	else
	{
		OPP_ERROR_INCONSISTENT_STATE;
	}

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::processPeerClosed()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	if (sockstate == RECEIVING)
	{
		sockstate = PEER_CLOSED;
		_cb_handler->recvCallback(connId, TCPSocketAPI_Base::CB_E_CLOSED,
				NULL, removeUserContext());
	}
	else if (sockstate == CONNECTED)
	{
		sockstate = PEER_CLOSED;
		this->close(); // when closed the close callback is invoked
	}
	else if (sockstate == LOCALLY_CLOSED)
	{
		sockstate = CLOSED;
	}
	else
	{
		// fact: sockstate == NOT_BOUND | BOUND | CONNECTING? | LISTENING |
		//					  ACCEPTING | PEER_CLOSED | CLOSED | SOCKERROR
		// the TCPSocket::processMessage would simple set the state to CLOSED
		OPP_ERROR_INCONSISTENT_STATE;
	}

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::processClosed()
{
	LOG_DEBUG_FUN_BEGIN(toString());

	sockstate = CLOSED; // TODO when should it be LOCALLY_CLOSED?

	LOG_DEBUG_FUN_END(toString());

	//_cb_handler->closeCallback(connId, 0, removeUserContext());
}

void TCPSocketExtension::processFailure(int code)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	if (sockstate == CONNECTING)
	{
		sockstate = SOCKERROR;
		_cb_handler->connectCallback(connId, code, removeUserContext());
	}
	else if (sockstate == RECEIVING)
	{
		sockstate = SOCKERROR;
		_cb_handler->recvCallback(connId, code, NULL, removeUserContext());
	}
	else
	{
		sockstate = SOCKERROR;
	}

	LOG_DEBUG_FUN_END(toString());
}

void TCPSocketExtension::processStatusArrived(TCPStatusInfo *status)
{
	LOG_DEBUG_FUN_BEGIN(toString());

	// nothing special for now
	ASSERT(status != NULL);
	delete status;

	LOG_DEBUG_FUN_END(toString());
}
