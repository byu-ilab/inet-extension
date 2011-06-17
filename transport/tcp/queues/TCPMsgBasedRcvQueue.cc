//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "TCPMsgBasedRcvQueue.h"

Register_Class(TCPMsgBasedRcvQueue);


TCPMsgBasedRcvQueue::TCPMsgBasedRcvQueue() : TCPVirtualDataRcvQueue()
{
}

TCPMsgBasedRcvQueue::~TCPMsgBasedRcvQueue()
{
	/* KPB +++> */
	for (PayloadList::iterator pl_itr = payloadList.begin();
			pl_itr != payloadList.end(); pl_itr++)
	{
		ASSERT(pl_itr->second != NULL);

		delete pl_itr->second;
	}
	/* <+++ */
}

void TCPMsgBasedRcvQueue::init(uint32 startSeq)
{
    TCPVirtualDataRcvQueue::init(startSeq);
}

std::string TCPMsgBasedRcvQueue::info() const
{
    std::string res;
    char buf[32];
    sprintf(buf, "rcv_nxt=%u ", rcv_nxt);
    res = buf;

    for (RegionList::const_iterator i=regionList.begin(); i!=regionList.end(); ++i)
    {
        sprintf(buf, "[%u..%u) ", i->begin, i->end);
        res+=buf;
    }
    sprintf(buf, "%u msgs", payloadList.size());
    res+=buf;

    return res;
}
/**
 * Called when TCP receives a message, and there is still room in the receive buffer
 * to hold the new data. (freeRecieveBuffer >= tcpseg.byteLength())
 */
uint32 TCPMsgBasedRcvQueue::insertBytesFromSegment(TCPSegment *tcpseg)
{
	/* KPB +++> */
	// This must now be executed at the end of the function so as not to add
	// the end sequence number of the segment until after the payload is
	// extracted.  This makes the seqGreater logic valid such that a repeat
	// message is not added to the queue and handed up to the application
	// a second time.
	/* <+++ */
    // ---> TCPVirtualDataRcvQueue::insertBytesFromSegment(tcpseg); <---

    cPacket *msg;
    uint32 endSeqNo;
    while ((msg=tcpseg->removeFirstPayloadMessage(endSeqNo))!=NULL)
    {
    	/* KPB +++> */
    	if (seqLE(rcv_nxt, endSeqNo))
    	{
    		/* <+++ */

    		// insert, avoiding duplicates
			PayloadList::iterator i = payloadList.find(endSeqNo);
			if (i!=payloadList.end()) {delete msg; continue;} // KPB assumes that the send queue duplicates messages when they are packed
			payloadList[endSeqNo] = msg;

			/* +++> */
    	}
    	else
    	{
    		// otherwise the message and the bytes have already been received
    		delete msg;
    	}
    	/* <+++ */
    }

    return TCPVirtualDataRcvQueue::insertBytesFromSegment(tcpseg);
    // ---> return rcv_nxt; <---
}


/**
 * Called when there is not enough room in the receive buffer for this new segment.
 */
cPacket *TCPMsgBasedRcvQueue::extractBytesUpTo(uint32 seq)
{
    extractTo(seq);

    // pass up payload messages, in sequence number order
    if (payloadList.empty() || seqGreater(payloadList.begin()->first, seq))
        return NULL;

    cPacket *msg = payloadList.begin()->second;
    payloadList.erase(payloadList.begin());
    return msg;
}

