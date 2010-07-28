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

#include "ByteBufferClient.h"
#include "TCPSocketAPIAppUtils.h"
#include "httptMessages_m.h"
#include "httptController.h"
#include "ModuleAccess.h"
#include "MsgByteBuffer.h"

#define DEBUG_CLASS true

Define_Module(ByteBufferClient);

uint32 ByteBufferClient::__serial = 0;

ByteBufferClient::ByteBufferClient()
	: reqs_sent(STATNAME_REQS_SENT, (unsigned long) 0),
	  rep_fragments_rcvd(STATNAME_REPFRAG_RCVD, (unsigned long) 0),
	  reps_rcvd(STATNAME_REPS_RCVD, (unsigned long) 0)
{
	socketapi = NULL;
	reqs_to_send = 0;
	bytes_rcvd = 0;
	bytes_to_rcv = 0;
	msg_ev_signal = SIMSIGNAL_NULL;
	mode = BBN_MODE_NOTSET;
	my_serial = __serial++;
}

void ByteBufferClient::initialize()
{
	LOG_DEBUG_FUN_BEGIN("");
	// configuration from ned file
	reqs_to_send = par("numRequestsToSend");
	req_phase_density = par("requestPhaseDensity");
		ASSERT(0 < req_phase_density);
	serverwww = par("serverwww").stdstringValue();
		ASSERT(!serverwww.empty());

	setMode(mode, par("mode"));
	LOG_DEBUG_APPEND_LN("mode: "<<(mode == BBN_MODE_WHOLE ? BBN_MODE_NAME_WHOLE : BBN_MODE_NAME_FRAGMENTS));

	// configuration from network
	socketapi = findTCPSocketAPI(this);

	httptController * controller = check_and_cast<httptController *>(findModuleSomewhereUp(par("controller"),this));

	char szModuleName[128];
	controller->getServerInfo(serverwww.c_str(),szModuleName,remote_port);

	remote_address = szModuleName;

	msg_ev_signal = registerSignal(SIGNAME_MSGEV);

	cMessage * start = new cMessage("START", BBC_START);

//	if (my_serial != 0)
//	{
//		scheduleAt(simTime() + 2.0, start);
//	}
//	else
//	{
//		scheduleAt(simTime() + 1.0, start);
//	}
	scheduleAt(simTime() + ((double)my_serial * 0.1), start);
	LOG_DEBUG_FUN_END("");
}

void ByteBufferClient::handleMessage(cMessage * msg)
{
	LOG_DEBUG_FUN_BEGIN("");
	if (msg->getKind() == BBC_START)
	{
		//means start
		// connect to the server
		LOG_DEBUG_APPEND_LN("active socket parameters: "<<"\'\':-1 <=> "<<remote_address<<":"<<remote_port);
		current_socket_id = socketapi->makeActiveSocket(this, "", -1, remote_address, remote_port);
	}
	else if (msg->getKind() == BBC_RESUME)
	{
//		if (reqs_sent.unsignedLongValue() < reqs_to_send) //else
//		{
//			LOG_DEBUG_LN("resuming activity on socket "<<current_socket_id<<" at t="<<simTime());
//			sendRequest(current_socket_id);
//
//			if ((reqs_sent.unsignedLongValue() - rep_fragments_rcvd.unsignedLongValue()) <= 1)
//			{
//				LOG_DEBUG_LN("waiting for recv on socket "<<current_socket_id);
//				socketapi->recv(current_socket_id);
//			}
//		}
//		else
//		{
//			LOG_DEBUG_LN("closing socket "<<current_socket_id);
//			socketapi->close(current_socket_id);
//		}
		error("resume messages shouldn't be used");
	}
	else
	{
		error("Unknown message kind.");
	}
	delete msg;
	LOG_DEBUG_FUN_END("");
}

void ByteBufferClient::connectCallback(int socket_id, int ret_status, void * myPtr)
{
	Enter_Method_Silent();

	LOG_DEBUG_FUN_BEGIN("");

	ASSERT(myPtr == NULL);
	ASSERT(!TCPSocketAPI::isCallbackError(ret_status));
	ASSERT(current_socket_id == socket_id);

	LOG_DEBUG_APPEND_LN("connected socket "<<socket_id<<" at t="<<simTime());

	sendRequest(socket_id);

	LOG_DEBUG_FUN_END("waiting for recv on socket "<<socket_id);
	socketapi->recv(socket_id);
}

void ByteBufferClient::recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr)
{
	Enter_Method_Silent();

	ASSERT(current_socket_id == socket_id);

	LOG_DEBUG_FUN_BEGIN("received "<<ret_status<<" bytes on socket "<<socket_id<<" at t="<<simTime());

	if (ret_status == TCPSocketAPI::CB_E_CLOSED)
	{
		socketapi->close(socket_id);
		LOG_DEBUG_FUN_END("socket "<<socket_id<<" closed");
		return;
	}

	ASSERT(myPtr == NULL);
	ASSERT(!TCPSocketAPI::isCallbackError(ret_status));

	if (mode == BBN_MODE_WHOLE)
	{
		httptReplyMessage * reply = dynamic_cast<httptReplyMessage *>(msg);
		ASSERT(reply != NULL);
		ASSERT(reply->getByteLength() == ret_status);

		LOG_DEBUG_APPEND_LN("received "<<reply->getName());

		reps_rcvd.increment();
		emit(msg_ev_signal, &reps_rcvd);

		if (reqs_sent.unsignedLongValue() < reqs_to_send)
		{
			sendRequest(socket_id);
//			cMessage * resume = new cMessage("RESUME", BBC_RESUME);
//			LOG_DEBUG_LN("scheduling resume time 2.0 seconds from "<<simTime());
//			scheduleAt(simTime()+2.0, resume);
		}
	}
	else if (mode == BBN_MODE_FRAGMENTS)
	{
		MsgByteBuffer * buffer = dynamic_cast<MsgByteBuffer *>(msg);
		ASSERT(buffer != NULL);

		// Designed for multiple replies
		int remainder = ret_status;

		do
		{
			LOG_DEBUG_APPEND_LN("remaining rcvd bytes: "<<remainder);
			LOG_DEBUG_APPEND_LN("bytes to rcv: "<<bytes_to_rcv);
			LOG_DEBUG_APPEND_LN("byte buffer payload array size: "<<buffer->getPayloadArraySize());
			// If there is not a reply that is currently being received
			if (bytes_to_rcv == 0)
			{
				// then extract it from the byte buffer
				ASSERT(0 < buffer->getPayloadArraySize());
				cPacket * plmsg = buffer->removeFirstPayloadMessage();
				ASSERT(plmsg != NULL);

				LOG_DEBUG_APPEND_LN("beginning to receive "<<plmsg->getName());

				ASSERT(bytes_rcvd == 0);
				bytes_to_rcv = plmsg->getByteLength();

				delete plmsg;
			}

			// Update the byte counters
			bytes_rcvd += remainder;
			bytes_to_rcv -= remainder;
			LOG_DEBUG_APPEND_LN("bytes rcvd: "<<bytes_rcvd);
			LOG_DEBUG_APPEND_LN("bytes to rcv: "<<bytes_to_rcv);

			// If the whole reply has been received
			if (bytes_to_rcv <= 0)
			{
				// Then handle the message
					// remember bytes_to_rcv is zero or negative
				remainder = -bytes_to_rcv;
				LOG_DEBUG_APPEND_LN("remaining bytes: "<<remainder);
				if (0 < remainder)
				{
					ASSERT(0 < buffer->getPayloadArraySize());
				}

				bytes_rcvd = 0;
				bytes_to_rcv = 0;

				// The whole reply has been "received" now (its
				// already been deleted)
				reps_rcvd.increment();
				emit(msg_ev_signal, &reps_rcvd);

				if (reqs_sent.unsignedLongValue() < reqs_to_send)
				{
					sendRequest(socket_id);
				}
			}
			else
			{
				// Then there should not be any messages in the buffer
				ASSERT(buffer->getPayloadArraySize() == 0);
			}

			if (0 < buffer->getPayloadArraySize())
			{
				ASSERT(bytes_to_rcv == 0);
				ASSERT(bytes_rcvd == 0);
			}
		} while(0 < buffer->getPayloadArraySize());

		// Designed for a single reply
//		if (bytes_to_rcv == 0) // first fragment
//		{
//			ASSERT(0 < buffer->getPayloadArraySize());
//			cPacket * plmsg = NULL;
//			while((plmsg = buffer->removeFirstPayloadMessage()) != NULL)
//			{
//				ASSERT(dynamic_cast<httptReplyMessage *>(plmsg) != NULL);
//				bytes_rcvd = ret_status;
//				bytes_to_rcv = plmsg->getByteLength() - bytes_rcvd;
//				delete plmsg;
//			}
//		}
//		else
//		{
//			ASSERT(buffer->getPayloadArraySize() == 0);
//
//			bytes_rcvd += ret_status;
//			bytes_to_rcv -= ret_status;
//
//			ASSERT(0 <= bytes_to_rcv);
//
//			if (bytes_to_rcv == 0 && reqs_sent.unsignedLongValue() < reqs_to_send)
//			{
//				bytes_rcvd = 0;
//				//TODO set a resume timer?
//				sendRequest(socket_id);
//			}
//		}
	}
	else
	{
		error("Mode value is not set.");
	}

	delete msg;
	rep_fragments_rcvd.increment();
	emit(msg_ev_signal, &rep_fragments_rcvd);

//	if (reqs_sent.unsignedLongValue() <= reqs_to_send)
//	{
//		cMessage * resume = new cMessage("RESUME", BBC_RESUME);
//		LOG_DEBUG_LN("scheduling resume time 2.0 seconds from now");
//		scheduleAt(simTime()+2.0, resume);

		LOG_DEBUG_FUN_END("waiting for recv on socket "<<socket_id);
		socketapi->recv(socket_id);
//	}
//	else
//	{
//		LOG_DEBUG_FUN_END("closing socket "<<socket_id);
//		socketapi->close(socket_id);
//	}

}

void ByteBufferClient::sendRequest(int socket_id)
{
	LOG_DEBUG_FUN_BEGIN("sending on socket "<<socket_id);

	for (int i = 0; i < req_phase_density; i++)
	{
		std::string uri = "/index";
		std::string header = "GET "+uri+" HTTP/1.1";
		httptRequestMessage * request = new httptRequestMessage(header.c_str());
		request->setHeading(header.c_str());
		request->setUri(uri.c_str());

		request->setTargetUrl(serverwww.c_str());
		request->setProtocol(11);
		request->setByteLength(200);
		request->setKeepAlive(true);
		request->setKind(HTTPT_REQUEST_MESSAGE);

		reqs_sent.increment();
		emit(msg_ev_signal, &reqs_sent);

		LOG_DEBUG_APPEND_LN("sending request #"<<reqs_sent.unsignedLongValue()<<
				" for "<<request->getName()<<
				" on socket "<<socket_id<<
				" at t="<<simTime());
		socketapi->send(socket_id, request);
	}
//	cMessage * resume = new cMessage("RESUME", BBC_RESUME);
//	LOG_DEBUG_LN("scheduling resume time 2.0 seconds from "<<simTime());
//	scheduleAt(simTime()+2.0, resume);

	LOG_DEBUG_FUN_END("");
}
