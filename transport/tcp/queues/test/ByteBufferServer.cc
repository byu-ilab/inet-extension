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

#include "ByteBufferServer.h"
#include "TCPSocketMgrAppUtils.h"
#include "MsgByteBuffer.h"

#define DEBUG_CLASS true

Define_Module(ByteBufferServer);

ByteBufferServer::ByteBufferServer()
	: httptServerBase(),
	  reps_sent(STATNAME_REPS_SENT, (unsigned long) 0),
	  req_fragments_rcvd(STATNAME_REQS_RCVD, (unsigned long) 0)
{
	socketapi = NULL;
	mode = BBN_MODE_NOTSET;
	msg_ev_signal = SIMSIGNAL_NULL;
}

ByteBufferServer::~ByteBufferServer()
{
	for (SocketMap::iterator i = sockets.begin(); i != sockets.end(); i++)
	{
		delete i->second;
	}
}

void ByteBufferServer::initialize()
{
	LOG_DEBUG_FUN_BEGIN("");
	httptServerBase::initialize();

	//ASSERT(0 <= par("msgSizeStep"));
	msg_size_step = par("msgSizeStep"); // a negative message size means that the size decreases
	LOG_DEBUG_APPEND_LN("message size step: "<<msg_size_step);

	ASSERT(0 < (int) par("startingMsgSize")); // must be positive
	starting_msg_size = par("startingMsgSize");
	LOG_DEBUG_APPEND_LN("starting msg size: "<<starting_msg_size);

	ASSERT(0 < (int) par("msgsInStepInterval")); // must be positive
	max_msgs_in_step_interval = par("msgsInStepInterval");
	LOG_DEBUG_APPEND_LN("max msgs in step interval: "<<max_msgs_in_step_interval);

	current_msgs_sent_in_step_interval = 0;
	current_interval = 0;

	socketapi = findTCPSocketMgr(this);

	setMode(mode, par("mode"));

	msg_ev_signal = registerSignal(SIGNAME_MSGEV);

	cMessage * start = new cMessage("START");
	scheduleAt(0, start);

	LOG_DEBUG_FUN_END("");
}

void ByteBufferServer::handleMessage(cMessage * msg)
{
	// means start
	int fd = socketapi->makePassiveSocket(this, "", port);
	socketapi->accept(fd);
	LOG_DEBUG_LN("socket "<<fd<<" listening");
	delete msg;
}

void ByteBufferServer::acceptCallback(int listening_socket_id, int accepted_socket_id, void * myPtr)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN("");

	ASSERT(!TCPSocketAPI_Inet::isCallbackError(accepted_socket_id));
	ASSERT(myPtr == NULL);

	LOG_DEBUG_APPEND_LN("accepted socket "<<accepted_socket_id<<" from socket "<<listening_socket_id);

	if (mode == BBN_MODE_FRAGMENTS)
	{
		sockets[accepted_socket_id] = new ByteBufferServerSocketWrapper(accepted_socket_id);
	}
	// otherwise don't store socket state data

	socketapi->accept(listening_socket_id);
	LOG_DEBUG_FUN_END("");
	socketapi->recv(accepted_socket_id);
}

void ByteBufferServer::recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN("received "<<ret_status<<" bytes on socket "<<socket_id);
	if (ret_status == TCPSocketAPI_Inet::CB_E_CLOSED)
	{
		socketapi->close(socket_id);
		LOG_DEBUG_FUN_END("socket "<<socket_id<<" closed");
		return;
	}

	ASSERT(!TCPSocketAPI_Inet::isCallbackError(ret_status));
	ASSERT(myPtr == NULL);

	httptReplyMessage * reply = NULL;
	if (mode == BBN_MODE_WHOLE)
	{
		ASSERT(ret_status == msg->getByteLength());

		reply = handleRequestMessage(msg);
		ASSERT(reply != NULL);
		sendReply(socket_id, reply);
	}
	else if (mode == BBN_MODE_FRAGMENTS)
	{
		MsgByteBuffer * buffer = dynamic_cast<MsgByteBuffer *>(msg);
		ASSERT(buffer != NULL);
		ASSERT(sockets.find(socket_id) != sockets.end());
		ByteBufferServerSocketWrapper * w = sockets[socket_id];

		// Designed for multiple requests at once
		int remainder = ret_status;

		do
		{
			LOG_DEBUG_APPEND_LN("pending request: "<<(w->pending_request == NULL ? "null" : w->pending_request->getName()));

			LOG_DEBUG_APPEND("remainder: "<<remainder);
			LOG_DEBUG_APPEND_LN("\t# msgs in buffer: "<<buffer->getPayloadArraySize());

			LOG_DEBUG_APPEND("before bytes rcvd: "<<w->bytes_rcvd);
			LOG_DEBUG_APPEND_LN("\tbytes to rcv: "<<w->bytes_to_rcv);

			// If there is not a request that is currently being received
			if (w->pending_request == NULL)
			{
				// then extract it from the byte buffer
				ASSERT(0 < buffer->getPayloadArraySize());
				cPacket * plmsg = buffer->removeFirstPayloadMessage();
				ASSERT(plmsg != NULL);

				LOG_DEBUG_APPEND_LN("beginning to receive: "<<plmsg->getName());

				ASSERT(w->bytes_rcvd == 0);
				w->bytes_to_rcv = plmsg->getByteLength();
				w->pending_request = plmsg;
			}

			// Update the byte counters
			w->bytes_rcvd += remainder;
			w->bytes_to_rcv -= remainder;
			LOG_DEBUG_APPEND(" after bytes rcvd: "<<w->bytes_rcvd);
			LOG_DEBUG_APPEND_LN("\tbytes to rcv: "<<w->bytes_to_rcv);

			// If the whole request has been received
			if (w->bytes_to_rcv <= 0)
			{
				// Then handle the message and reset the socket wrapper
					// remember w->bytes_to_rcv is zero or negative
				remainder = -w->bytes_to_rcv;
				w->bytes_rcvd += w->bytes_to_rcv;
				ASSERT(w->bytes_rcvd == w->pending_request->getByteLength());

				reply = handleRequestMessage(w->pending_request);
				ASSERT(reply != NULL);
				sendReply(socket_id, reply);

				delete w->pending_request;
				w->reset();
			}
			else
			{
				// Then there should not be any messages in the buffer
				ASSERT(buffer->getPayloadArraySize() == 0);
			}

			if (0 < buffer->getPayloadArraySize())
			{
				ASSERT(w->pending_request == NULL);
				ASSERT(w->bytes_to_rcv == 0);
				ASSERT(w->bytes_rcvd == 0);
			}
		} while(0 < buffer->getPayloadArraySize());

		// Designed for just one request at a time
//		if (w->bytes_to_rcv == 0) // first fragment
//		{
//			ASSERT(0 < buffer->getPayloadArraySize());
//			cPacket * plmsg = NULL;
//			while((plmsg = buffer->removeFirstPayloadMessage()) != NULL)
//			{
//				w->bytes_rcvd = ret_status;
//				w->bytes_to_rcv = plmsg->getByteLength() - w->bytes_rcvd;
//				w->pending_request = plmsg;
//			}
//		}
//		else
//		{
//			ASSERT(buffer->getPayloadArraySize() == 0);
//
//			w->bytes_rcvd += ret_status;
//			w->bytes_to_rcv -= ret_status;
//
//			ASSERT(0 <= w->bytes_to_rcv);
//		}
//
//		if (w->bytes_to_rcv == 0)
//		{
//			w->bytes_rcvd = 0;
//			reply = handleRequestMessage(w->pending_request);
//			delete w->pending_request;
//			w->pending_request = NULL;
//		}
	}
	else
	{
		error("Mode value is not set.");
	}

	delete msg;
	req_fragments_rcvd.increment();
	emit(msg_ev_signal, &req_fragments_rcvd);

	socketapi->recv(socket_id);

	LOG_DEBUG_FUN_END("waiting for recv on socket "<<socket_id);
}

void ByteBufferServer::sendReply(int socket_id, httptReplyMessage * reply)
{
	LOG_DEBUG_FUN_BEGIN("");
	ASSERT(reply != NULL);

	LOG_DEBUG_APPEND_LN("sending reply: "<<reply->getName()<<" of size "<<reply->getByteLength()<<" bytes at t="<<simTime());

	socketapi->send(socket_id, reply);

	reps_sent.increment();
	emit(msg_ev_signal, &reps_sent);
	LOG_DEBUG_FUN_END("");
}

httptReplyMessage * ByteBufferServer::handleGetRequest( httptRequestMessage *request, string resource_url)
{
	LOG_DEBUG_FUN_BEGIN("");
	LOG_DEBUG_APPEND_LN("handling request "<<request->getName());
	httptReplyMessage * reply = NULL;
	if (resource_url == "/index")
	{
		// generate the right sized message
		int size = starting_msg_size;

		if (max_msgs_in_step_interval <= current_msgs_sent_in_step_interval)
		{
			current_interval++;
			current_msgs_sent_in_step_interval = 0;
		}

		size += (int) msg_size_step * current_interval;
		if (size <= 0)
		{
			size = starting_msg_size;
			current_interval = 0;
		}
		current_msgs_sent_in_step_interval++;

		reply = generateStandardReply(request, resource_url, HTTP_CODE_200, size,rt_text);
	}
	else
	{
		reply = generateErrorReply(request, resource_url, HTTP_CODE_404);
	}

	LOG_DEBUG_FUN_END("");
	return reply;
}
