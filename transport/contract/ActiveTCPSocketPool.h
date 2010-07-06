/*
 * ActiveTCPSocketPool.h
 *
 *  Created on: Jun 18, 2010
 *      Author: black
 */

#ifndef ACTIVETCPSOCKETPOOL_H_
#define ACTIVETCPSOCKETPOOL_H_

#include "TCPSocketAPI.h"
#include "TCPPortRangeDefs.h"

#include "DebugDef.h"
#include "DeleteSafeDefs.h"
#include <set>
#include <deque>
#include <map>

#define UNLIMITED_LOAD -1
#define MAX_LOAD INT32_MAX
#define MIN_LOAD 1

#define REQUEST_ID_MIN 1
#define REQUEST_ID_MAX INT32_MAX

#define SCB_NO_PENDING_RESPONSES 0
#define SCB_NOT_READY -1

#define MASKED_SOCKET -1

enum RequestStatus { RS_PENDING, RS_SENT, RS_UNKNOWN };

class ActiveTCPSocketPool : public TCPSocketAPI::CallbackInterface
{

protected:

	struct RequestRecord
	{
		int request_id;
		cPacket * request;

		RequestRecord(int id, cPacket * req) { request_id = id; request = req; }
		virtual ~RequestRecord () { request = NULL; }
	};

	typedef std::deque<RequestRecord *> RequestRecordQueue;
	typedef std::map<int, int *> PendingRequestsForSocketMap;
	typedef std::set<int> RequestIdSet;

	TCPSocketAPI * _socketapi; //< the socket API to use
	TCPSocketAPI::CallbackInterface * _pool_owner; //< the pool owner who will recv responses
	RequestIdSet _sent_requests; //< tracks the id of the requests that have been sent
	RequestRecordQueue _pending_requests; //< the requests that remain to be scheduled
	int _socket_cap; //< the maximum number of sockets in this socket pool
	int _max_load; //< the maximum number of pending responses for a socket
	int _current_target_load; //< the desired load to be put on each socket
	void * _owner_recv_info_ptr; //< the information to be returned to the owner on recv
	PendingRequestsForSocketMap _sockets_cbdata; //< maps socket id to the number of pending responses on that socket
	PendingRequestsForSocketMap::iterator _scb_itr;

	int _current_request_id; //< the current request id
	std::string _server_address;
	int _server_port;
	SimTime _socket_timeout;
	int _num_ready_sockets;

public:
	// if max_load is UNLIMITED_LOAD (by default) then the socket pool will not limit the number of
	// requests sent and pending response on any socket
	ActiveTCPSocketPool(TCPSocketAPI * socketapi, TCPSocketAPI::CallbackInterface * pool_owner,
			int max_num_sockets, const std::string server_address, int server_port,
			simtime_t timeout, int max_load = UNLIMITED_LOAD, void * your_recv_info_ptr=NULL);

	virtual ~ActiveTCPSocketPool();

	virtual void * getMyRecvCallbackData();

	// doesn't call the pool owner's connectCallback, the socket pool knows how to handle connections
	virtual void connectCallback(int socket_id, int ret_status, void * myPtr);

	// will call the pool owner's recvCallback, only that it will mask the socket_id by
	// returning MASKED_SOCKET for socket_id
	virtual void recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr);

	// assumes responsibility for the cPacket
	virtual int  submitRequest(cPacket * request);

//	// returns the request and doesn't send it as long as it is still pending, returns NULL otherwise
//	// Relinquishes responsibility for the cPacket
//	virtual cPacket * cancelPendingRequest(int reqest_id);
//
//	// returns true if the pending request was canceled and deleted
//	virtual bool cancelAndDeletePendingRequest(int request_id);
//
//	// returns true if there were pending requests that were canceled and deleted
//	virtual bool cancelAndDeleteAllPendingRequests();

	// returns the status of the request, will be RS_PENDING if the request is in the pending
	// map, will be RS_SENT if the request has been sent, and will be RS_UNKNOWN if the
	// request id is invalid
	virtual RequestStatus getRequestStatus(int request_id);

protected:
	virtual RequestRecord * findPendingRequestRecord(int request_id);

	virtual void updateLoad ();
	virtual int openSocket();
	virtual void closeSocket(int socket_id);

	// returns true if the entry for the indicated socket was newly added and
	// false if an entry had previously been set and was reset
	virtual bool setPendingResponsesOnSocket(int socket_id, int value);
	virtual void incrementPendingResponsesOnSocket(int socket_id);
	virtual void decrementPendingResponsesOnSocket(int socket_id);
	virtual int  getPendingResponsesOnSocket(int socket_id);
	virtual void removePendingResponsesOnSocket(int socket_id);
};

#endif /* ACTIVETCPSOCKETPOOL_H_ */
