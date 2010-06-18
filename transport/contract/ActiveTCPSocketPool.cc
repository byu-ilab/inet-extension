/*
 * ActiveTCPSocketPool.cc
 *
 *  Created on: Jun 18, 2010
 *      Author: black
 */

#include "ActiveTCPSocketPool.h"

ActiveTCPSocketPool::ActiveTCPSocketPool(TCPSocketAPI * socketapi, TCPSocketAPI::CallbackInterface * pool_owner,
		int max_num_sockets, const std::string server_address, int server_port, simtime_t timeout,
		int max_load, void * your_recv_info_ptr)
{
	ASSERT(socketapi);
	ASSERT(pool_owner);
	ASSERT(0 < max_num_sockets);
	ASSERT(!server_address.empty());
	ASSERT(0 <= port && port <= 65535);
	ASSERT(max_load == UNLIMITED_LOAD || ( 0 < max_load  && max_load < ( INT32_MAX / max_num_sockets ) ) );

	_socketapi = socketapi;
	_pool_owner = pool_owner;
	_socket_cap = max_num_sockets;
	_max_load = max_load;
	_owner_recv_info_ptr = your_recv_info_ptr;
	_current_request_id = REQUEST_ID_MIN;
	_server_address = server_address;
	_server_port = server_port;
	_socket_timeout = (0 < timeout) ? timeout : 0;
	_num_ready_sockets = 0;
}

ActiveTCPSocketPool::~ActiveTCPSocketPool()
{
	// DO NOT delete _socketapi or _pool_owner
}


void ActiveTCPSocketPool::connectCallback(int socket_id, int ret_status, void * myPtr)
{
	PendingRequestsForSocketMap::iterator scb_itr = _sockets_cbdata.find(socket_id);
	ASSERT(scb_itr != _sockets_cbdata.end()); // socket_id should always be valid

	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		closeSocket(socket_id);
	}
	else
	{
		*(scb_itr->second) = SCB_NO_PENDING_RESPONSES;
		_num_ready_sockets++;
	}

	updateLoad();
}

// will call the pool owner's recvCallback, only that it will mask the socket_id by
// returning MASKED_SOCKET for socket_id
void ActiveTCPSocketPool::recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr)
{
	PendingRequestsForSocketMap::iterator scb_itr = _sockets_cbdata.find(socket_id);
	ASSERT(scb_itr != _sockets_cbdata.end()); // socket_id should always be valid

	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		closeSocket(socket_id);
	}
	else
	{
		*(scb_itr->second)--;
		_pool_owner->recvCallback(MASKED_SOCKET, ret_status, msg, _owner_recv_info_ptr);
	}

	updateLoad();
}

// assumes responsibility for the cPacket
int  ActiveTCPSocketPool::submitRequest(cPacket * request)
{
	ASSERT(request);
	ASSERT( REQUEST_ID_MIN <= _current_requeset_id && _current_request_id <= REQUEST_ID_MAX );

	RequestRecord * rr = new RequestRecord(_current_request_id++, request);
	_pending_requests.push_back(rr);
	UpdateLoad();
	return rr->request_id;
}

void ActiveTCPSocketPool::updateLoad ()
{
	if (_pending_requests.empty())
	{ // then there is nothing to send
		return;
	}
	// else there is something to send

	// compute the current target load and manage number of sockets
	int used_load_slots = 0;
	PendingRequestsForSocketMap::iterator scd_itr = _sockets_cbdata.begin();
	while (scd_itr != _sockets_cbdata.end())
	{
		if (*(scd_itr->second) != SCB_NOT_READY )
		{
			used_load_slots += *(scd_itr->second);
		}
	}

	int available_load_slots = (_current_target_load * _num_ready_sockets) - used_load_slots;

	if (_num_ready_sockets == 0 || available_load_slots == 0)
	{ // sockets aren't ready yet, make some if needed
		while (_sockets_cbdata.size() < _pending_requests.size() && _sockets_cbdata.size() < _socket_cap)
		{
			openSocket();
		}
		return; //they have to connect before we send requests on them
	}

	int diff_als_drs = available_load_slots - _pending_requests.size();
	if (diff_als_drs < 0)
	{
		// increment the _current_target_load
		while ( ( (_current_target_load + 1) * _num_ready_sockets ) < _pending_requests.size() &&
				_current_target_load < _max_load )
		{
			_current_target_load++;
		}
	}
	else if (diff_als_drs > 0)
	{ // decrement the _current_target_load
		while ( ( (_current_target_load - 1) * _num_ready_sockets ) > _pending_requests.size() )
		{
			_current_target_load--;
		}
	}
	// else diff_als_drs == 0

	// send requests on all of the sockets
//	int selected_fd = -1;
//	int * selected_pending_responses = NULL;
//	int num_sockets_not_ready = 0;

	scd_itr = _sockets_cbdata.begin();
	while (scd_itr != _sockets_cbdata.end()) //true)
	{
//		if (scd_itr == _sockets_cbdata.end())
//		{ // then increment the target load to find a socket
//			_current_target_load++;
//			scd_itr = _sockets_cbdata.begin();
//		}

		int * pending_responses = scd_itr->second;
		if ( SCB_NO_PENDING_RESPONSES <= *pending_responses && *pending_responses < _current_target_load )
		{
			// send a request on the socket
			while (*pending_responses < _current_target_load && !_pending_requests.empty())
			{
				RequestRecord * rr_ptr = _pending_requests.front();
				_socketapi->send(scd_itr->first, rr_ptr->request);
				(*pending_responses)++;
				_sent_requests.insert(rr_ptr->request_id);

				// if this is the first request sent on the socket then signal it to recv
				if (*pending_responses == 1)
				{
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
}

int ActiveTCPSocketPool::openSocket()
{
	if (_sockets_cbdata.size() == _socket_cap)
	{
		return -1;
	}

	int fd = _socketapi->socket(this);
	if (0 < _socket_timeout)
	{
		_socketapi->setTimeout(fd, _server_timeout);
	}

	int * pending_responses = new int(SCB_NOT_READY);
	_sockets_cbdata[fd] = pending_responses;

	_socketapi->connect(fd, _server_address, _server_port, NULL);

	return fd;
}

void ActiveTCPSocketPool::closeSocket(int socket_id)
{
	PendingRequestsForSocketMap::iterator scb_itr = _sockets_cbdata.find(socket_id);
	ASSERT(scb_itr != _sockets_cbdata.end());

	_socketapi->close(socket_id);

	if (scb_itr->second)
	{
		delete scb_itr->second;
	}
	_sockets_cbdata.erase(scb_itr);
	_num_ready_sockets--;
}

// returns the request and doesn't send it as long as it is still pending, returns NULL otherwise
// Relinquishes responsibility for the cPacket
cPacket * ActiveTCPSocketPool::cancelPendingRequest(int reqest_id)
{
	return findPendingRequestRecord(request_id);
}

// returns true if the pending request was canceled and deleted
bool ActiveTCPSocketPool::cancelAndDeletePendingRequest(int request_id)
{
	RequestRecord * rr_ptr = findPendingRequestRecord(request_id);

	if (rr_ptr && rr_ptr->request)
	{
		delete rr_ptr->request;
		return true;
	}

	return false;
}

// returns true if there were pending requests that were canceled and deleted
bool ActiveTCPSocketPool::cancelAndDeleteAllPendingRequests()
{
	if (_pending_requests.empty())
	{
		return false;
	}

	RequestRecordQueue::iterator pr_itr = _pending_requests.begin();
	while (pr_itr != _pending_requests.end())
	{
		if ( *pr_itr->request ) {
			delete *pr_itr->request;
		}
		pr_itr = _pending_requests.erase(pr_itr);
	}
	return true;
}

// returns the status of the request, will be RS_PENDING if the request is in the pending
// map, will be RS_SENT if the request has been sent, and will be RS_UNKNOWN if the
// request id is invalid
RequestStatus ActiveTCPSocketPool::getRequestStatus(int request_id)
{
	RequestRecord * rr_ptr = findPendingRequestRecord(request_id);
	if (rr_ptr)
	{
		return RS_PENDING;
	}

	RequestIdSet::iterator rid_itr = _sent_requests.find(request_id);
	if (rid_itr != _sent_requests.end())
	{
		return RS_SENT;
	}

	return RS_UNKNOWN;
}

RequestRecord * ActiveTCPSocketPool::findPendingRequestRecord(int request_id)
{
	if ( request_id < REQUEST_ID_MIN || REQUEST_ID_MAX < request_id )
	{
		return NULL;
	}

	RequestRecordQueue::iterator pr_itr = _pending_requests.begin();
	while (pr_itr != _pending_requests.end())
	{
		if ((*pr_itr).request_id == request_id)
		{
			return *pr_itr;
		}
		pr_itr++;
	}
	return NULL;
}
