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

#ifndef MSGBYTEBUFFER_H_
#define MSGBYTEBUFFER_H_

#include <list>
#include <iostream>
#include "INETDefs.h"
#include "MsgByteBuffer_m.h"
using namespace std;

/**
 * Modeled off of TCPSegment.
 */
class MsgByteBuffer : public MsgByteBuffer_Base
{
protected:
	std::list<cPacketPtr> payloadList;

public:
	MsgByteBuffer(const char *name=NULL, int kind=0) : MsgByteBuffer_Base(name,kind) {}
	MsgByteBuffer(const MsgByteBuffer& other) : MsgByteBuffer_Base(other.getName()) {operator=(other);}
	virtual ~MsgByteBuffer();

	MsgByteBuffer& operator=(const MsgByteBuffer& other); //MsgByteBuffer_Base::operator=(other); return *this;}
	virtual MsgByteBuffer * dup() const {return new MsgByteBuffer(*this);}
	virtual void parsimPack(cCommBuffer *b);
	virtual void parsimUnpack(cCommBuffer *b);

	/** Generated but unused method, should not be called. */
	virtual void setPayloadArraySize(unsigned int size);
	/** Generated but unused method, should not be called. */
	virtual void setPayload(unsigned int k, const cPacketPtr& payload_var);

	/**
	 * Returns the number of payload messages in this message byte buffer.
	 */
	virtual unsigned int getPayloadArraySize() const;

	/**
	 * Returns the kth payload message in this message byte buffer.
	 */
	virtual cPacketPtr& getPayload(unsigned int k);

	/**
	 * Adds a message object to the message byte buffer.
	 */
	virtual void addPayloadMessage(cPacket * msg);

	/**
	 * Removes and returns the first message object in this TCP segment.
	 * It also returns the sequence number+1 of its last octet in outEndSequenceNo.
	 */
	virtual cPacket * removeFirstPayloadMessage();
};

#endif /* MSGBYTEBUFFER_H_ */
