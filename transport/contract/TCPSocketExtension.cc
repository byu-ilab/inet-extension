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

// From standard C++ libraries
#include <sstream>

#define OPP_ERROR(DETAILS) opp_error("%s::%s%s.",__FILE__,__FUNCTION__,DETAILS)
#define OPP_ERROR_INCONSISTENT_STATE opp_error("%s::%s inconsistent state %s",\
		__FILE__, __FUNCTION__, stateName(sockstate))

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

void TCPSocketExtension::bind(address_cref_t local_address, port_t local_port)
{
	if (local_address.empty())
	{
		// Checks socket state and port number
		TCPSocket::bind(local_port);
	}
	else
	{
		// Checks socket state and port number, allows for only the
		// address to be specified if port = PORT_NULL but address must
		// thus be valid.
		TCPSocket::bind(_resolver.resolve(local_address.c_str(), _resolution_mode), local_port);
	}
	// fact: sockstate == BOUND
}

void TCPSocketExtension::listen (cb_inet_handler_ptr_t cb_handler_for_accepted)
{
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
	if (sockstate != LISTENING && sockstate != ACCEPTING)
	{
		OPP_ERROR("socket is not a passive socket");
	}

	socket->setCallbackHandler(_cb_handler_for_accepted);

	_sockets_pending_acceptance.push_back(socket);

	if (sockstate == ACCEPTING)
	{
		processEstablished();
	}
}


void TCPSocketExtension::connect(address_cref_t remote_address, port_t remote_port)
{
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
	// check the socket's state before taking the message
	if (	   sockstate != CONNECTED
			&& sockstate != CONNECTING
			&& sockstate != RECEIVING
			&& sockstate != PEER_CLOSED)
	{
		OPP_ERROR("socket cannot send");
	}

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
	if (sockstate != CONNECTED && sockstate != RECEIVING)
	{
		OPP_ERROR("socket is not connected");
	}
	// fact: else sockstate == CONNECTED || sockstate == RECEIVING
	if (sockstate == RECEIVING)
	{
		OPP_ERROR("socket is already set to receive");
	}

	// relative to the byte_mode return something out of the buffer
//	if (!_reception_buffers[id].empty())
//	{
//		cPacket * msg = _reception_buffers[id].front();
//		_reception_buffers[id].pop_front();
//		socketDataArrived(id, cbdata, msg, false);
//	}
	// or set a timeout to occur if no data is received in time
//	else
//	{
//		// schedule a timeout if set
//		SocketTimeoutMsg * timer = _timeout_timers[id];
//		if (timer) {
//			if (timer->isScheduled()) {
//				cancelEvent(timer);
//			}
//			scheduleAt(simTime()+timer->getTimeoutInterval(), timer);
//		}
//	}
}


void TCPSocketExtension::recv (bytecount_t byte_mode,
		user_data_ptr_t context)
{
	_user_context_data = context;
	this->recv(byte_mode);
}

void TCPSocketExtension::setTimeout(simtime_t timeout_period)
{
	if (timeout_period < 0)
	{
		OPP_ERROR("timeout period is negative");
	}

	if (_timeout_scheduler == NULL)
	{
		OPP_ERROR("no timeout scheduler set");
	}

	if (_timeout_msg == NULL)
	{
		_timeout_msg = new SocketTimeoutMsg("socket timeout");
		_timeout_msg->setSocketId(connId);
	}
	else if (_timeout_msg->isScheduled())
	{
		// then the _timeout_scheduler had to be non-NULL to have scheduled
		// the message
		_timeout_scheduler->cancelEvent(_timeout_msg);
	}

	// set period on timer
	_timeout_msg->setTimeoutInterval(timeout_period.dbl());
}


bool TCPSocketExtension::removeTimeout ()
{
	if (_timeout_msg == NULL)
	{
		return false;
	}

	if (_timeout_scheduler == NULL)
	{
		OPP_ERROR("no timeout scheduler set");
	}

	_timeout_scheduler->cancelAndDelete(_timeout_msg);
	_timeout_msg = NULL;
	return true;
}


void TCPSocketExtension::close ()
{
	switch(sockstate)
	{
	case NOT_BOUND:
	case BOUND:
	case SOCKERROR:
		// no TCP core clean up needs to occur
		removeTimeout();
		break;
	case CLOSED:
	case LOCALLY_CLOSED:
		OPP_ERROR("already called on this socket");
		break;
	default:
		// including PEER_CLOSED, CONNECTING, CONNECTED,
		// LISTENING, ACCEPTING, RECEIVING, TIMED_OUT

		// cancel any callbacks
		removeTimeout();

		// initiate TCP core close messages
		TCPSocket::close();
	}
}

user_data_ptr_t TCPSocketExtension::getUserContext ()
{
	return _user_context_data;
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
		 // carries a new connId which won't match the connId of this TCPSocket,
		 // so you won't get here. Rather, when you see TCP_I_ESTABLISHED, you'll
		 // want to create a new TCPSocket object via new TCPSocket(msg).
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
			<<"one of the TCP_I_xxx constants expected";
		delete msg;
		OPP_ERROR(details.str().c_str());
	}
}

void TCPSocketExtension::processDataArrived(cPacket *msg, bool urgent)
{
	user_data_ptr_t ret_context = _user_context_data;
	_user_context_data = NULL;

	// depends on receiving mode
	if (sockstate == RECEIVING)
	{
		_cb_handler->recvCallback(connId, msg->getByteLength(), msg, ret_context);
		sockstate = CONNECTED;
	}
	else if (sockstate == CONNECTED)
	{
		// add message to receive buffers
		// no change in socket state
	}
	else if (sockstate == CLOSED || sockstate == TIMED_OUT)
	{
		// print a notice
	}
	else
	{
		OPP_ERROR_INCONSISTENT_STATE;
	}
}

void TCPSocketExtension::processEstablished()
{
	user_data_ptr_t ret_context = _user_context_data;
	_user_context_data = NULL;

	// invoke the "connect" or "accept" callback
	if (sockstate == CONNECTING)
	{
		// need to update the bound ports in the socket mgr
		sockstate = CONNECTED;
		_cb_handler->connectCallback(connId, 0, ret_context);
	}
	else if (sockstate == ACCEPTING)
	{
		ASSERT(!_sockets_pending_acceptance.empty());

		TCPSocketExtension * socket = _sockets_pending_acceptance.front();
		_sockets_pending_acceptance.pop_front();
		// DO NOT delete the socket as it is probably stored in a socket map somewhere else
		sockstate = LISTENING;

		_cb_handler->acceptCallback(connId, socket->getConnectionId(), ret_context);
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
	user_data_ptr_t ret_context = _user_context_data;
	_user_context_data = NULL;

	if (sockstate == RECEIVING)
	{
		sockstate = PEER_CLOSED;
		_cb_handler->recvCallback(connId,
				TCPSocketAPI_Base::CB_E_CLOSED, NULL, ret_context);
	}
	else if (sockstate == CONNECTED)
	{
		sockstate = PEER_CLOSED;
		this->close();
	}
	else
	{
		OPP_ERROR_INCONSISTENT_STATE;
	}
}

void TCPSocketExtension::processClosed()
{
	// clean up in socket mgr?
}

void TCPSocketExtension::processFailure(int code)
{
	user_data_ptr_t ret_context = _user_context_data;
	_user_context_data = NULL;

	// invoke the recv callback or just handle the close operation?
	if (sockstate == CONNECTING)
	{
		sockstate = SOCKERROR;
		_cb_handler->connectCallback(connId, code, ret_context);
	}
	else if (sockstate == RECEIVING)
	{
		sockstate = SOCKERROR;
		_cb_handler->recvCallback(connId, code, NULL, ret_context);
	}
	else
	{
		// error or close the socket?
		OPP_ERROR_INCONSISTENT_STATE;
	}
}

void TCPSocketExtension::processStatusArrived(TCPStatusInfo *status)
{
	ASSERT(status != NULL);

	// nothing special for now
	delete status;
}
