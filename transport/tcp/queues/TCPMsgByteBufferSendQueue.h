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

#ifndef TCPMSGBYTEBUFFERSENDQUEUE_H_
#define TCPMSGBYTEBUFFERSENDQUEUE_H_

#include <omnetppextension.h>
#include "TCPMsgBasedSendQueue.h"

/**
 * Intended for use with the TCPMsgByteBufferRcvQueue.  This class does not use
 * MsgByteBuffers but includes that term in its name to link it with the
 * TCPMsgByteBufferRcvQueue.
 *
 * The main difference between this queue and the TCPMsgBasedSendQueue is that
 * the application layer message is encapsulated in the first TCP segment of the
 * sequence of TCP segments used to simulate the sending of the application layer
 * message instead of in the last TCP segment of the sequence.
 *
 * @see TCPMsgByteBufferRcvQueue
 */
class TCPMsgByteBufferSendQueue : public TCPMsgBasedSendQueue
{
private:
	static int __serial;
	int my_serial;

public:
	TCPMsgByteBufferSendQueue() : TCPMsgBasedSendQueue() { my_serial = __serial++; }

	/**
	 * No extra functionality to add.
	 */
	virtual ~TCPMsgByteBufferSendQueue() {}

	// Debug handles
	virtual void enqueueAppData(cPacket *msg);
	virtual void discardUpTo(uint32 seqNum);

	/**
     * Places the application's message in the TCPSegment iff the message's
     * beginning sequence number (i.e. it's end sequence number - its length
     * in bytes) is between fromSeq and fromSeq+numBytes (inclusive).  Sends the
     * application layer message in the frst TCP segment of the sequence used to
     * simulate the sending of the application layer message.
     */
    virtual TCPSegment *createSegmentWithBytes(uint32 fromSeq, ulong numBytes);
};

#endif /* TCPMSGBYTEBUFFERSENDQUEUE_H_ */
