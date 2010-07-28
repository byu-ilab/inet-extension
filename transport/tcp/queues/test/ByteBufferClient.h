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

#ifndef __INET_BYTEBUFFERCLIENT_H_
#define __INET_BYTEBUFFERCLIENT_H_

#include <omnetppextension.h>
#include <dynamicresultrecorders.h>
#include "TCPSocketAPI.h"
#include "ByteBufferNode.h"

#define SIGNAME_MSGEV "msgevent"
#define STATNAME_REQS_SENT "client.requests.sent"
#define STATNAME_REPFRAG_RCVD "client.replies.fragments.rcvd"
#define STATNAME_REPS_RCVD "client.replies.rcvd"

enum ByteBufferClientMessageKinds { BBC_START, BBC_RESUME };

class ByteBufferClient : public cSimpleModule, public TCPSocketAPI::CallbackInterface
{
private:
	static uint32 __serial;

	// Configured from module ned file
	ByteBufferNodeMode mode;
	uint64 reqs_to_send;
	std::string serverwww;
	int req_phase_density;

	// Configured from network elements
	TCPSocketAPI * socketapi;
	std::string remote_address;
	int remote_port;

	// Internal statistic counters
	simsignal_t msg_ev_signal;
	DynamicResultValue reqs_sent;
	DynamicResultValue rep_fragments_rcvd;
	DynamicResultValue reps_rcvd;

//	uint32 reqs_sent;
//	uint32 rep_fragments_rcvd;

	int32 bytes_rcvd;
	int32 bytes_to_rcv;

	uint32 my_serial;
	int current_socket_id;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage * msg);

    virtual void connectCallback(int socket_id, int ret_status, void * myPtr);
    virtual void recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr);

    virtual void sendRequest(int socket_id);

public:
    ByteBufferClient();
    	virtual ~ByteBufferClient() {}
};

#endif
