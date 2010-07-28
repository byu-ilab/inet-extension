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

#include "TCPMsgByteBufferRcvQueue.h"
#include "MsgByteBuffer.h"

#define DEBUG_CLASS false

Register_Class(TCPMsgByteBufferRcvQueue);

int TCPMsgByteBufferRcvQueue::__serial = 0;

uint32 TCPMsgByteBufferRcvQueue::insertBytesFromSegment(TCPSegment *tcpseg)
{
	LOG_DEBUG_FUN_BEGIN(this->my_serial<<" t="<<simTime());
	LOG_DEBUG_APPEND("seg. id: "<<tcpseg->getId());
	LOG_DEBUG_APPEND("\tseq #: "<<tcpseg->getSequenceNo());
	LOG_DEBUG_APPEND("\ttotal B: "<<tcpseg->getByteLength());
	LOG_DEBUG_APPEND("\tpayload B: "<<tcpseg->getPayloadLength());
	LOG_DEBUG_APPEND_LN("\t# app msgs: "<<tcpseg->getPayloadArraySize());

	LOG_DEBUG_APPEND("# app msgs in queue: "<<payloadList.size());
	LOG_DEBUG_APPEND("\t# bytes: "<<getAmountOfBufferedBytes());
	uint32 size_before = payloadList.size();

	uint32 to_return = TCPMsgBasedRcvQueue::insertBytesFromSegment(tcpseg);

	uint32 size_after = payloadList.size();
	LOG_DEBUG_APPEND_LN("\tinserted "<<(size_after-size_before)<<" messages");
	LOG_DEBUG_APPEND("# app msgs in queue: "<<payloadList.size());
	LOG_DEBUG_APPEND_LN("\t# bytes: "<<getAmountOfBufferedBytes());
	LOG_DEBUG_FUN_END("");
	return to_return;
}

cPacket * TCPMsgByteBufferRcvQueue::extractBytesUpTo(uint32 seq)
{
	// no debug msgs here so as to cut down on unnecessary print statements
	ulong numBytes = extractTo(seq);
	if (numBytes == 0)
	{
		return NULL;
	}

	LOG_DEBUG_FUN_BEGIN(this->my_serial<<" t="<<simTime());
	LOG_DEBUG_APPEND("# app msgs in queue: "<<payloadList.size());
	LOG_DEBUG_APPEND("\t# bytes: "<<numBytes+getAmountOfBufferedBytes());

	MsgByteBuffer * buffer = new MsgByteBuffer("buffered bytes");
	buffer->setByteLength(numBytes);

	// pass up payload messages, in sequence number order
	while (!payloadList.empty())
	{
		uint32 beginSeq = payloadList.begin()->first - payloadList.begin()->second->getByteLength();
		if (seqLE(seq, beginSeq)) // up to but not including seq
		{
			break;
		}
		buffer->addPayloadMessage(payloadList.begin()->second);
		payloadList.erase(payloadList.begin());
	}

	LOG_DEBUG_APPEND_LN("\tpacked "<<buffer->getPayloadArraySize()<<" messages into msg byte buffer");
	LOG_DEBUG_APPEND("# app msgs in queue: "<<payloadList.size());
	LOG_DEBUG_APPEND_LN("\t# bytes: "<<getAmountOfBufferedBytes());
	LOG_DEBUG_FUN_END("");
	return buffer;
}
