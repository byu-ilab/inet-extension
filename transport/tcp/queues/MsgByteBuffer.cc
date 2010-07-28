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

#include "MsgByteBuffer.h"

Register_Class(MsgByteBuffer);

MsgByteBuffer& MsgByteBuffer::operator=(const MsgByteBuffer& other)
{
	if (this==&other)
	{
		return *this;
	}

	//TODO empty the current payloadList?
    MsgByteBuffer_Base::operator=(other);

    for (std::list<cPacket *>::const_iterator i=other.payloadList.begin(); i!=other.payloadList.end(); ++i)
        addPayloadMessage((*i)->dup());

    return *this;
}

MsgByteBuffer::~MsgByteBuffer()
{
    while (!payloadList.empty())
    {
        cPacket * msg = payloadList.front();
        payloadList.pop_front();
        dropAndDelete(msg);
    }
}

void MsgByteBuffer::parsimPack(cCommBuffer *b)
{
    MsgByteBuffer_Base::parsimPack(b);
    doPacking(b, payloadList);
}

void MsgByteBuffer::parsimUnpack(cCommBuffer *b)
{
    MsgByteBuffer_Base::parsimUnpack(b);
    doUnpacking(b, payloadList);
}

void MsgByteBuffer::setPayloadArraySize(unsigned int size)
{
    throw cRuntimeError(this, "setPayloadArraySize() not supported, use addPayloadMessage()");
}

unsigned int MsgByteBuffer::getPayloadArraySize() const
{
    return payloadList.size();
}

cPacketPtr& MsgByteBuffer::getPayload(unsigned int k)
{
	ASSERT(k < payloadList.size());
    std::list<cPacket *>::iterator i = payloadList.begin();
    while (k>0 && i!=payloadList.end())
        (++i, --k);
    return *i;
}

void MsgByteBuffer::setPayload(unsigned int k, const cPacketPtr& payload_var)
{
    throw cRuntimeError(this, "setPayload() not supported, use addPayloadMessage()");
}

void MsgByteBuffer::addPayloadMessage(cPacket * msg)
{
    take(msg);

    payloadList.push_back(msg);
}

cPacket * MsgByteBuffer::removeFirstPayloadMessage()
{
    if (payloadList.empty())
        return NULL;

    cPacket *msg = payloadList.front();
    payloadList.pop_front();
    drop(msg);
    return msg;
}

