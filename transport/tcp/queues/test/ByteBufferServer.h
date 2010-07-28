//
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

#ifndef __INET_BYTEBUFFERSERVER_H_
#define __INET_BYTEBUFFERSERVER_H_

#include <omnetppextension.h>
#include <dynamicresultrecorders.h>
#include "httptServerBase.h"
#include "ByteBufferNode.h"
#include "TCPSocketAPI.h"

#define SIGNAME_MSGEV "msgevent"
#define STATNAME_REPS_SENT "server.replies.sent"
#define STATNAME_REQS_RCVD "server.requests.fragments.rcvd"

struct ByteBufferServerSocketWrapper
{
private:
	void initialize(int socket_id)
	{
		this->socket_id = socket_id;
		bytes_rcvd = 0;
		bytes_to_rcv = 0;
		pending_request = NULL;
	}

public:
	int socket_id;
	int32 bytes_rcvd;
	int32 bytes_to_rcv;
	cPacket * pending_request;


	ByteBufferServerSocketWrapper() { initialize(-1); }

	ByteBufferServerSocketWrapper(int socket_id) { initialize(socket_id); }

	virtual ~ByteBufferServerSocketWrapper() {}

	void reset()
	{
		initialize(socket_id);
	}
};

class ByteBufferServer : public httptServerBase, public TCPSocketAPI::CallbackInterface
{
private:
	ByteBufferNodeMode mode;
	TCPSocketAPI * socketapi;
	uint32 msg_size_step;

	DynamicResultValue reps_sent;
	DynamicResultValue req_fragments_rcvd;
	simsignal_t msg_ev_signal;

	typedef std::map<int, ByteBufferServerSocketWrapper> SocketMap;
	SocketMap sockets;

protected:
	virtual void initialize();
	virtual void handleMessage(cMessage * msg);

	virtual httptReplyMessage * handleGetRequest( httptRequestMessage *request, string resource_url);

	virtual void acceptCallback(int socket_id, int ret_status, void * myPtr);
	virtual void recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr);

	virtual void sendReply(int socket_id, httptReplyMessage * reply);

public:
	ByteBufferServer();
	virtual ~ByteBufferServer() {}
};

#endif
