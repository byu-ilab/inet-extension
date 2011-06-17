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

#include "TCPMsgByteBufferSendQueue.h"

#define DEBUG_CLASS false

Register_Class(TCPMsgByteBufferSendQueue);

int TCPMsgByteBufferSendQueue::__serial = 0;

void TCPMsgByteBufferSendQueue::enqueueAppData(cPacket *msg)
{
	LOG_DEBUG_FUN_BEGIN(this->my_serial<<" t="<<simTime());
	LOG_DEBUG_APPEND("# app msgs: "<<payloadQueue.size());
	LOG_DEBUG_APPEND("\t# bytes: "<<(end-begin));

	TCPMsgBasedSendQueue::enqueueAppData(msg);

	LOG_DEBUG_APPEND_LN("\tenqueued 1 message: "<<msg->getName());
	LOG_DEBUG_APPEND("# app msgs: "<<payloadQueue.size());
	LOG_DEBUG_APPEND_LN("\t# bytes: "<<(end-begin));
	LOG_DEBUG_FUN_END("");
}

void TCPMsgByteBufferSendQueue::discardUpTo(uint32 seqNum)
{
	LOG_DEBUG_FUN_BEGIN(this->my_serial<<" t="<<simTime());

	LOG_DEBUG_APPEND("# app msgs: "<<payloadQueue.size());
	LOG_DEBUG_APPEND("\t# bytes: "<<(end-begin));

	uint32 size_before = payloadQueue.size();

	TCPMsgBasedSendQueue::discardUpTo(seqNum);

	uint32 size_after = payloadQueue.size();

	LOG_DEBUG_APPEND_LN("\tdiscarded "<<(size_before-size_after)<<" messages");
	LOG_DEBUG_APPEND("# app msgs: "<<payloadQueue.size());
	LOG_DEBUG_APPEND_LN("\t# bytes: "<<(end-begin));

	LOG_DEBUG_FUN_END("");
}

TCPSegment *TCPMsgByteBufferSendQueue::createSegmentWithBytes(uint32 fromSeq, ulong numBytes)
{
	LOG_DEBUG_FUN_BEGIN(this->my_serial<<" t="<<simTime());

    ASSERT(seqLE(begin,fromSeq) && seqLE(fromSeq+numBytes,end));

    TCPSegment *tcpseg = conn->createTCPSegment(NULL);
    tcpseg->setSequenceNo(fromSeq);
    tcpseg->setPayloadLength(numBytes);

    LOG_DEBUG_APPEND("seg. id: "<<tcpseg->getId());
    LOG_DEBUG_APPEND("\tseq #: "<<fromSeq);
    LOG_DEBUG_APPEND("\ttotal B: "<<tcpseg->getByteLength());
    LOG_DEBUG_APPEND_LN("\tpayload B: "<<tcpseg->getPayloadLength());

    // add payload messages whose beginSequenceNo is between fromSeq and fromSeq+numBytes (inclusively)
    PayloadQueue::iterator i = payloadQueue.begin();
    int n=0;
    while (i!=payloadQueue.end() && seqLE(i->endSequenceNo, fromSeq))
    {
    	LOG_DEBUG_APPEND_LN("app msg "<<n<<" end seq #: "<<i->endSequenceNo);
        ++i;
        ++n;
    }
    uint32 toSeq = fromSeq + numBytes;
    const char *payloadName = NULL;
    while (i!=payloadQueue.end()) // queue of (unsent) cMessages sent from app.  (removed from queue when sent)
    {
    	// ===> Main difference from TCPMsgBasedSendQueue
    	uint32 beginSequenceNo = i->endSequenceNo - i->msg->getByteLength();
    	LOG_DEBUG_APPEND_LN("app msg "<<n<<" begin seq #: "<<beginSequenceNo);

    	// SHOULD NOT BE
    	// if (seqLess(beginSequenceNo, fromSeq) && seqLE(toSeq, beginSequenceNo)
    	// {
    	// 		break;
    	// }
    	// Because an app msg may have an endSequenceNo greater than fromSeq
    	// but a beginning sequence number less than fromSeq

    	// fromSeq is inclusive but toSeq is not
    	if (seqLE(fromSeq, beginSequenceNo) && seqLess(beginSequenceNo, toSeq))
    	{
    		// <===
			if (!payloadName)
			{
				payloadName = i->msg->getName();
			}

			tcpseg->addPayloadMessage(i->msg->dup(), i->endSequenceNo);
			LOG_DEBUG_APPEND_LN("added duplicate of msg "<<i->msg->getId()<<" to tcp segment payload");
    	}
        ++i;
        ++n;
    }

    LOG_DEBUG_APPEND_LN("packed "<<tcpseg->getPayloadArraySize()<<" messages into tcp segment");

    // give segment a name
	char msgname[80];
	if (!payloadName)
	{
		sprintf(msgname, "tcpseg(l=%lu,%dmsg,e=%lu)", numBytes, tcpseg->getPayloadArraySize(), (tcpseg->getSequenceNo()+numBytes));
		tcpseg->setName(msgname);
	}
	else if (conn->getTcpMain()->shouldTrackDupMessageNames)
	{
		tcpseg->setName(payloadName);
	}
	else
	{
		sprintf(msgname, "%.10s(l=%lu,%dmsg,e=%lu)", payloadName, numBytes, tcpseg->getPayloadArraySize(), (tcpseg->getSequenceNo()+numBytes));
		tcpseg->setName(msgname);
	}

	//cout<<"At time="<<simTime()<<", Socket "<<this->conn->connId<<" creating tcp segment with info: "<<tcpseg->getName()<<""<<endl;
	LOG_DEBUG_FUN_END("");
    return tcpseg;
}


