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

#ifndef TCPMSGBYTEBUFFERRCVQUEUE_H_
#define TCPMSGBYTEBUFFERRCVQUEUE_H_

#include "TCPMsgBasedRcvQueue.h"

/**
 * Intended for use with TCPMsgBasedSendQueue.
 */
class TCPMsgByteBufferRcvQueue: public TCPMsgBasedRcvQueue
{
private:
	static int __serial;
	int my_serial;

public:
	TCPMsgByteBufferRcvQueue() : TCPMsgBasedRcvQueue() { my_serial = __serial++; }
	virtual ~TCPMsgByteBufferRcvQueue() {}

	// Debug handle
	virtual uint32 insertBytesFromSegment(TCPSegment *tcpseg);

	/**
	 * Returns a MsgByteBuffer with its byte length set to the number of bytes
	 * extracted and includes application layer messages in its payload iff
	 * the beginning sequence number of the application layer message (the
	 * end sequence number - the message byte length) is less than or equal to
	 * the provided sequence number.
	 */
	virtual cPacket * extractBytesUpTo(uint32 seq);
};

#endif /* TCPMSGBYTEBUFFERRCVQUEUE_H_ */
