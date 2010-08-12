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

#define OPP_ERROR(DETAILS) opp_error("%s::%s%s.",__FILE__,__FUNCTION__,DETAILS)
#define OPP_ERROR_INCONSISTENT_STATE opp_error("%s::%s inconsistent state %s",\
		__FILE__, __FUNCTION__, stateName(sockstate))

#define DEBUG_CLASS true

//==============================================================================
// Initialization

void TCPSocketExtension::initialize(cb_inet_handler_ptr_t handler,
		cSimpleModule * scheduler, IPAddressResolver::ResolutionMode mode)
{
	_cb_handler = NULL;
	setCallbackHandler(handler); // just in case handler is non-NULL
	_cb_handler_for_accepted = NULL;
	_user_context_data = NULL;
	_timeout_scheduler = scheduler;
	_timeout_msg = NULL;
	_resolution_mode = mode;
}

//==============================================================================
// Constructors and Destructor

TCPSocketExtension::TCPSocketExtension() : TCPSocket()
{
	initialize(NULL, NULL, IPAddressResolver::ADDR_PREFER_IPv4);
}

TCPSocketExtension::TCPSocketExtension(cMessage * msg) : TCPSocket()
{
	initialize(NULL, NULL, IPAddressResolver::ADDR_PREFER_IPv4);
}

TCPSocketExtension::TCPSocketExtension(TCPSocketAPI_Inet::CallbackHandler * handler,
		cSimpleModule * scheduler, IPAddressResolver::ResolutionMode mode)
	: TCPSocket()
{
	initialize(handler, scheduler, mode);
}

TCPSocketExtension::~TCPSocketExtension()
{
	// DO NOT delete _cb_handler or _timeout_scheduler
}

//==============================================================================
// BDS-like Socket Operations

void TCPSocketExtension::bind(address_cref_t local_address, port_t local_port)
{
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
}

void TCPSocketExtension::listen (cb_inet_handler_ptr_t cb_handler_for_accepted)
{
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
}

void TCPSocketExtension::listenOnce(cb_inet_handler_ptr_t cb_handler_for_accepted)
{
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
}

void TCPSocketExtension::accept ()
{
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
}

void TCPSocketExtension::accept (user_data_ptr_t context)
{
	// context data must be set first in the event that there is a connection
	// in the pending connections queue
	_user_context_data = context;
	this->accept();
}


void TCPSocketExtension::appendAcceptedSocket(TCPSocketExtension * socket)
{
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
}


void TCPSocketExtension::connect(address_cref_t remote_address, port_t remote_port)
{
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
}

void TCPSocketExtension::connect (address_cref_t remote_address,
		port_t remote_port, user_data_ptr_t context)
{
	_user_context_data = context;
	this->connect(remote_address, remote_port);
}

void TCPSocketExtension::send(cMessage *msg)
{
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
}


void TCPSocketExtension::recv (bytecount_t byte_mode)
{
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
}


void TCPSocketExtension::recv (bytecount_t byte_mode,
		user_data_ptr_t context)
{
	_user_context_data = context;
	this->recv(byte_mode);
}

void TCPSocketExtension::setTimeout(simtime_t timeout_period)
{
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
}


bool TCPSocketExtension::removeTimeout ()
{
	if (_timeout_msg == NULL)
	{
		return false;
	}

	ASSERT(_timeout_scheduler != NULL);
	if (!canModifyTimeout())
	{
		OPP_ERROR("socket state won't allow timeout to be removed");
	}

	_timeout_scheduler->cancelAndDelete(_timeout_msg);
	_timeout_msg = NULL;
	return true;
}

bool TCPSocketExtension::canModifyTimeout() const
{
	if (	   sockstate == NOT_BOUND
			|| sockstate == BOUND
			|| sockstate == CONNECTING
			|| sockstate == CONNECTED)
	{
		return true;
	}
	// else
	return false;
}

void TCPSocketExtension::close ()
{
	ASSERT(_cb_handler != NULL);

	if (sockstate == CLOSED || sockstate == LOCALLY_CLOSED)
	{
		OPP_ERROR("already called on this socket");
	}
	// else
	removeTimeout();

	if (sockstate != NOT_BOUND && sockstate != BOUND && sockstate != SOCKERROR)
	{
		TCPSocket::close();
	}
	else
	{
		processClosed();
	}
}

user_data_ptr_t TCPSocketExtension::getUserContext ()
{
	return _user_context_data;
}

user_data_ptr_t TCPSocketExtension::removeUserContext ()
{
	user_data_ptr_t context = _user_context_data;
	_user_context_data = NULL;
	return context;
}

bool TCPSocketExtension::setTimeoutScheduler(cSimpleModule * scheduler)
{
	if (_timeout_scheduler == NULL)
	{
		_timeout_scheduler = scheduler;
		return true;
	}
	return false;
}

void TCPSocketExtension::setAddressResolutionMode(IPAddressResolver::ResolutionMode mode)
{
	_resolution_mode = mode;
}

IPAddressResolver::ResolutionMode TCPSocketExtension::getAddressResolutionMode()
{
	return _resolution_mode;
}

void TCPSocketExtension::setCallbackObject(TCPSocket::CallbackInterface *cb, void *yourPtr)
{
	OPP_ERROR("not supported.  See setCallbackHandler()");
}

void TCPSocketExtension::setCallbackHandler(cb_inet_handler_ptr_t handler)
{
	if (handler == NULL)
	{
		OPP_ERROR("callback handler may not be NULL");
	}
	_cb_handler = handler;
}

void TCPSocketExtension::processMessage (cMessage * msg)
{
	ASSERT(_cb_handler != NULL);
	ASSERT(msg != NULL);

	if (dynamic_cast<SocketTimeoutMsg *>(msg) != NULL)
	{
		processFailure(TCPSocketAPI_Base::CB_E_TIMEOUT);
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
		sockstate = CONNECTED;
		connectInfo = dynamic_cast<TCPConnectInfo *>(msg->getControlInfo());
		localAddr = connectInfo->getLocalAddr();
		remoteAddr = connectInfo->getRemoteAddr();
		localPrt = connectInfo->getLocalPort();
		remotePrt = connectInfo->getRemotePort();
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
}

void TCPSocketExtension::processDataArrived(cPacket *msg, bool urgent)
{
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
}

void TCPSocketExtension::processEstablished()
{
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
}

void TCPSocketExtension::processPeerClosed()
{
	if (sockstate == RECEIVING)
	{
		sockstate = PEER_CLOSED;
		_cb_handler->recvCallback(connId, TCPSocketAPI_Base::CB_E_CLOSED,
				NULL, removeUserContext());
	}
	else if (sockstate == CONNECTED)
	{
		sockstate == PEER_CLOSED;
		this->close(); // when closed the close callback is invoked
	}
	else
	{
		// the TCPSocket::processMessage would set the state to CLOSED
		OPP_ERROR_INCONSISTENT_STATE;
	}
}

void TCPSocketExtension::processClosed()
{
	sockstate = CLOSED; // TODO when should it be LOCALLY_CLOSED?
	_cb_handler->closeCallback(connId, 0, removeUserContext());
}

void TCPSocketExtension::processFailure(int code)
{
	sockstate = SOCKERROR;

	if (sockstate == CONNECTING)
	{
		_cb_handler->connectCallback(connId, code, removeUserContext());
	}
	else if (sockstate == RECEIVING)
	{
		_cb_handler->recvCallback(connId, code, NULL, removeUserContext());
	}
	else
	{
		LOG_DEBUG_LN("state: "<<stateName(sockstate));
	}
}

void TCPSocketExtension::processStatusArrived(TCPStatusInfo *status)
{
	// nothing special for now
	ASSERT(status != NULL);
	delete status;
}
