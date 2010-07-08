/*
 * ActiveTCPSocketPool.cc
 *
 *  Created on: Jun 18, 2010
 *      Author: Kevin Black
 */

#include "ActiveTCPSocketPool.h"

#define DEBUG_CLASS false

ActiveTCPSocketPool::ActiveTCPSocketPool(TCPSocketAPI * socketapi, TCPSocketAPI::CallbackInterface * pool_owner,
		int max_num_sockets, const std::string server_address, int server_port, simtime_t timeout,
		int max_load, void * your_recv_info_ptr)
{
	LOG_DEBUG_FUN_BEGIN("")

	/* Check preconditions */
	ASSERT(socketapi);
	ASSERT(pool_owner);
	ASSERT(0 < max_num_sockets);
	ASSERT(!server_address.empty());
	ASSERT(TCP_PORT_RANGE_MIN <= server_port && server_port <= TCP_PORT_RANGE_MAX);
	ASSERT(max_load == UNLIMITED_LOAD || ( MIN_LOAD <= max_load  && max_load < ( MAX_LOAD / max_num_sockets ) ) );
	/* End check preconditions */

	_socketapi = socketapi;
	_pool_owner = pool_owner;
	_socket_cap = max_num_sockets;
	_max_load = (max_load == UNLIMITED_LOAD) ? INT32_MAX / max_num_sockets : max_load;
	_owner_recv_info_ptr = your_recv_info_ptr;
	_current_request_id = REQUEST_ID_MIN;
	_server_address = server_address;
	_server_port = server_port;
	_socket_timeout = (timeout > 0) ? timeout : 0;
	_num_ready_sockets = 0;
	_current_target_load = 1;

	LOG_DEBUG_FUN_END("")
}

ActiveTCPSocketPool::~ActiveTCPSocketPool()
{
	LOG_DEBUG_FUN_BEGIN("")
	// DO NOT delete _socketapi or _pool_owner
	for (_scb_itr = _sockets_cbdata.begin(); _scb_itr != _sockets_cbdata.end(); ++_scb_itr)
	{
		deleteSafe(_scb_itr->second);
	}
	for (RequestRecordQueue::iterator pr_itr = _pending_requests.begin(); pr_itr != _pending_requests.end(); ++pr_itr)
	{
		deleteSafe((*pr_itr)->request);
		deleteSafe(*pr_itr);
	}
	LOG_DEBUG_FUN_END("")
}


void * ActiveTCPSocketPool::getMyRecvCallbackData()
{
	return _owner_recv_info_ptr;
}

void ActiveTCPSocketPool::connectCallback(int socket_id, int ret_status, void * myPtr)
{
	LOG_DEBUG_FUN_BEGIN("")

	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		closeSocket(socket_id);
	}
	else
	{
		setPendingResponsesOnSocket(socket_id, SCB_NO_PENDING_RESPONSES);
		_num_ready_sockets++;
	}

	updateLoad();

	LOG_DEBUG_FUN_END("")
}

// will call the pool owner's recvCallback, only that it will mask the socket_id by
// returning MASKED_SOCKET for socket_id
void ActiveTCPSocketPool::recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr)
{
	LOG_DEBUG_FUN_BEGIN("")

	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		closeSocket(socket_id);
		return;
	}
//	else
//	{
		decrementPendingResponsesOnSocket(socket_id);
		_pool_owner->recvCallback(MASKED_SOCKET, ret_status, msg, _owner_recv_info_ptr);
//	}

	updateLoad();
	if (1 < getPendingResponsesOnSocket(socket_id))
	{
		LOG_DEBUG_LN("receiving on socket "<<socket_id<<" at t="<<simTime());
		_socketapi->recv(socket_id, NULL);
	}

	LOG_DEBUG_FUN_END("")
}

// assumes responsibility for the cPacket
int ActiveTCPSocketPool::submitRequest (cPacket * request)
{
	LOG_DEBUG_FUN_BEGIN("")

	ASSERT(request);
	ASSERT( REQUEST_ID_MIN <= _current_request_id && _current_request_id <= REQUEST_ID_MAX );

	RequestRecord * rr = new RequestRecord(_current_request_id++, request);
	_pending_requests.push_back(rr);
	updateLoad(); // updateLoad may delete the request record so don't return rr->request_id

	LOG_DEBUG_FUN_END("")

	return (_current_request_id - 1); // updateLoad may delete the request record so don't return rr->request_id
}

void ActiveTCPSocketPool::updateLoad ()
{
	LOG_DEBUG_FUN_BEGIN("")

	if (_pending_requests.empty())
	{ // then there is nothing to send
		LOG_DEBUG_FUN_END("no pending requests")
		return;
	}
	// else there is something to send

	// find a socket to use

	// compute the current target load and manage number of sockets
	int used_load_slots = 0;
	PendingRequestsForSocketMap::iterator scd_itr = _sockets_cbdata.begin();
	while (scd_itr != _sockets_cbdata.end())
	{
		if (*(scd_itr->second) != SCB_NOT_READY )
		{
			used_load_slots += *(scd_itr->second);
		}
		scd_itr++;
	}

	int available_load_slots = (_current_target_load * _num_ready_sockets) - used_load_slots;

	if (_num_ready_sockets == 0 || available_load_slots == 0)
	{ // sockets aren't ready yet, make some if needed
		while (_sockets_cbdata.size() < _pending_requests.size() && _sockets_cbdata.size() < (unsigned int)_socket_cap)
		{
			openSocket();
		}
		LOG_DEBUG_FUN_END("connecting newly made sockets")
		return; //they have to connect before we send requests on them
	}

	int diff_als_drs = available_load_slots - _pending_requests.size();
	if (diff_als_drs < 0)
	{
		// increment the _current_target_load
		while ( ( (_current_target_load + 1) * _num_ready_sockets ) < (int) _pending_requests.size() &&
				_current_target_load < _max_load )
		{
			_current_target_load++;
		}
	}
	else if (diff_als_drs > 0)
	{ // decrement the _current_target_load
		while ( ( (_current_target_load - 1) * _num_ready_sockets ) > (int) _pending_requests.size() )
		{
			_current_target_load--;
		}
	}
	// else diff_als_drs == 0
	LOG_DEBUG_LN("current target load: "<<_current_target_load);

	// send requests on all of the sockets
//	int selected_fd = -1;
//	int * selected_pending_responses = NULL;
//	int num_sockets_not_ready = 0;

	scd_itr = _sockets_cbdata.begin();
	while (scd_itr != _sockets_cbdata.end() && !_pending_requests.empty()) //true)
	{
//		if (scd_itr == _sockets_cbdata.end())
//		{ // then increment the target load to find a socket
//			_current_target_load++;
//			scd_itr = _sockets_cbdata.begin();
//		}
		int current_socket_id = scd_itr->first;
		//int * pending_responses = scd_itr->second;
		int pending_responses = getPendingResponsesOnSocket(current_socket_id);
		if ( SCB_NO_PENDING_RESPONSES <= pending_responses && pending_responses < _current_target_load )
		{
			// send a request on the socket
			while (getPendingResponsesOnSocket(current_socket_id) < _current_target_load && !_pending_requests.empty())
			{
				RequestRecord * rr_ptr = _pending_requests.front();
				_pending_requests.pop_front();
				LOG_DEBUG_LN("sending \""<<rr_ptr->request->getName()<<
						"\" on socket "<<current_socket_id<<" at t="<<simTime());
				_socketapi->send(scd_itr->first, rr_ptr->request);
				incrementPendingResponsesOnSocket(current_socket_id);
				_sent_requests.insert(rr_ptr->request_id);
				delete rr_ptr;
				// if this is the first request sent on the socket then signal it to recv
				if (getPendingResponsesOnSocket(current_socket_id) == 1)
				{
					LOG_DEBUG_LN("receiving on socket "<<scd_itr->first<<" at t="<<simTime());
					_socketapi->recv(scd_itr->first, NULL);
				}
			}
//			selected_fd = scd_itr->first;
//			selected_pending_responses = scd_itr->second;
//			break;
		}
//		else
//		{
//			num_sockets_not_ready++;
//			if (num_socket_not_ready == _sockets_cbdata.size())
//			{
//				return;
//			}
//		}

		scd_itr++;
	}

//	// send a request on the socket
//	while (*selected_pending_responses < _current_target_load && !_pending_requests.empty())
//	{
//		RequestRecord * rr_ptr = _pending_requests.front();
//		_socketapi->send(selected_fd, rr_ptr->request);
//		(*selected_pending_responses)++;
//		_sent_requests.insert(rr_ptr->request_id);
//
//		// if this is the first request sent on the socket then signal it to recv
//		if (*selected_pending_responses == 1)
//		{
//			_socketapi->recv(selected_fd, NULL);
//		}
//	}
	LOG_DEBUG_FUN_END("normal end")
}

int ActiveTCPSocketPool::openSocket()
{
	LOG_DEBUG_FUN_BEGIN("")

	if (_sockets_cbdata.size() == (unsigned int) _socket_cap)
	{
		LOG_DEBUG_FUN_END("socket cap reached")
		return -1;
	}

	int fd = _socketapi->socket(this);
	if (_socket_timeout > 0)
	{
		LOG_DEBUG_LN("on socket "<<fd<<" set timeout of "<<_socket_timeout);
		_socketapi->setTimeout(fd, _socket_timeout);
	}

	setPendingResponsesOnSocket(fd, SCB_NOT_READY);

	LOG_DEBUG_LN("connecting socket "<<fd<<" to "<<_server_address<<":"<<_server_port);
	_socketapi->connect(fd, _server_address, _server_port, NULL);

	LOG_DEBUG_FUN_END("")
	return fd;
}

void ActiveTCPSocketPool::closeSocket(int socket_id)
{
	LOG_DEBUG_FUN_BEGIN("")

	_socketapi->close(socket_id);
	removePendingResponsesOnSocket(socket_id);

	_num_ready_sockets--;
	LOG_DEBUG_FUN_END("")
}

//// returns the request and doesn't send it as long as it is still pending, returns NULL otherwise
//// Relinquishes responsibility for the cPacket
//cPacket * ActiveTCPSocketPool::cancelPendingRequest(int request_id)
//{
//	LOG_DEBUG_FUN_BEGIN("")
//	RequestRecord * rr_ptr = findPendingRequestRecord(request_id);
//	if (rr_ptr)
//	{
//		LOG_DEBUG_FUN_END("")
//		return rr_ptr->request;
//	}
//	LOG_DEBUG_FUN_END("")
//	return NULL;
//}
//
//// returns true if the pending request was canceled and deleted
//bool ActiveTCPSocketPool::cancelAndDeletePendingRequest(int request_id)
//{
//	LOG_DEBUG_FUN_BEGIN("")
//	RequestRecord * rr_ptr = findPendingRequestRecord(request_id);
//
//	if (rr_ptr && rr_ptr->request)
//	{
//		delete rr_ptr->request;
//		LOG_DEBUG_FUN_END("")
//		return true;
//	}
//	LOG_DEBUG_FUN_END("")
//	return false;
//}
//
//// returns true if there were pending requests that were canceled and deleted
//bool ActiveTCPSocketPool::cancelAndDeleteAllPendingRequests()
//{
//	LOG_DEBUG_FUN_BEGIN("")
//
//	if (_pending_requests.empty())
//	{
//		LOG_DEBUG_FUN_END("")
//		return false;
//	}
//
//	RequestRecordQueue::iterator pr_itr = _pending_requests.begin();
//	while (pr_itr != _pending_requests.end())
//	{
//		if ( (*pr_itr)->request ) {
//			delete (*pr_itr)->request;
//		}
//		pr_itr = _pending_requests.erase(pr_itr);
//	}
//	LOG_DEBUG_FUN_END("")
//	return true;
//}

// returns the status of the request, will be RS_PENDING if the request is in the pending
// map, will be RS_SENT if the request has been sent, and will be RS_UNKNOWN if the
// request id is invalid
RequestStatus ActiveTCPSocketPool::getRequestStatus(int request_id)
{
	LOG_DEBUG_FUN_BEGIN("")
	RequestRecord * rr_ptr = findPendingRequestRecord(request_id);
	if (rr_ptr)
	{
		LOG_DEBUG_FUN_END("")
		return RS_PENDING;
	}

	RequestIdSet::iterator rid_itr = _sent_requests.find(request_id);
	if (rid_itr != _sent_requests.end())
	{
		LOG_DEBUG_FUN_END("")
		return RS_SENT;
	}

	LOG_DEBUG_FUN_END("")
	return RS_UNKNOWN;
}

ActiveTCPSocketPool::RequestRecord * ActiveTCPSocketPool::findPendingRequestRecord(int request_id)
{
	LOG_DEBUG_FUN_BEGIN("")
	if ( request_id < REQUEST_ID_MIN || REQUEST_ID_MAX < request_id )
	{
		LOG_DEBUG_FUN_END("")
		return NULL;
	}

	RequestRecordQueue::iterator pr_itr = _pending_requests.begin();
	while (pr_itr != _pending_requests.end())
	{
		if ((*pr_itr)->request_id == request_id)
		{
			LOG_DEBUG_FUN_END("")
			return *pr_itr;
		}
		pr_itr++;
	}
	LOG_DEBUG_FUN_END("")
	return NULL;
}

bool ActiveTCPSocketPool::setPendingResponsesOnSocket(int socket_id, int value)
{
	LOG_DEBUG_FUN_BEGIN("")
	_scb_itr = _sockets_cbdata.find(socket_id);

	if (_scb_itr == _sockets_cbdata.end())
	{
		_sockets_cbdata[socket_id] = new int(value);
		LOG_DEBUG_LN("pending responses on socket "<<socket_id<<": "<<value);
		LOG_DEBUG_FUN_END("")
		return true;
	}

	*(_scb_itr->second) = value;
	LOG_DEBUG_LN("pending responses on socket "<<socket_id<<": "<<value);
	LOG_DEBUG_FUN_END("")
	return false;
}


void ActiveTCPSocketPool::incrementPendingResponsesOnSocket(int socket_id)
{
	LOG_DEBUG_FUN_BEGIN("")
	_scb_itr = _sockets_cbdata.find(socket_id);
		ASSERT(_scb_itr != _sockets_cbdata.end()); // socket_id should always be valid

	(*_scb_itr->second)++;
	LOG_DEBUG_LN("pending responses on socket "<<socket_id<<": "<<*(_scb_itr->second));
	LOG_DEBUG_FUN_END("")
}

void ActiveTCPSocketPool::decrementPendingResponsesOnSocket(int socket_id)
{
	LOG_DEBUG_FUN_BEGIN("")
	_scb_itr = _sockets_cbdata.find(socket_id);
		ASSERT(_scb_itr != _sockets_cbdata.end()); // socket_id should always be valid

	(*_scb_itr->second)--;
	LOG_DEBUG_LN("pending responses on socket "<<socket_id<<": "<<*(_scb_itr->second));
	LOG_DEBUG_FUN_END("")
}


int ActiveTCPSocketPool::getPendingResponsesOnSocket(int socket_id)
{
	LOG_DEBUG_FUN_BEGIN("")
	_scb_itr = _sockets_cbdata.find(socket_id);
			ASSERT(_scb_itr != _sockets_cbdata.end()); // socket_id should always be valid

	LOG_DEBUG_LN("pending responses on socket "<<socket_id<<": "<<*(_scb_itr->second));
	LOG_DEBUG_FUN_END("")
	return *(_scb_itr->second);
}

void ActiveTCPSocketPool::removePendingResponsesOnSocket(int socket_id)
{
	_scb_itr = _sockets_cbdata.find(socket_id);
	if (_scb_itr == _sockets_cbdata.end())
	{
		return;
	}

	if (_scb_itr->second)
	{
		delete _scb_itr->second;
	}
	_sockets_cbdata.erase(_scb_itr);
}
