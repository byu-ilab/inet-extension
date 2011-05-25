//
// Copyright (C) 2005 Andras Varga
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


#include <omnetpp.h>
#include "DropTailQueue.h"


Define_Module(DropTailQueue);

void DropTailQueue::initialize()
{
    PassiveQueueBase::initialize();
    queue.setName("l2queue");

//    qlenVec.setName("queue length");
    dropVec.setName("drops");
    qlenSignal = registerSignal(SIGNAME_QLEN);
    qdropSignal = registerSignal(SIGNAME_QDROP);

    outGate = gate("out");

//    recordTrendOnly = par("recordTrendOnly");
//    lastEvent = QE_DEQUEUE;
//    lastEventTime = 0;

    // configuration
    frameCapacity = par("frameCapacity");
    queueLength=0;
    WATCH(queueLength);
}

bool DropTailQueue::enqueue(cMessage *msg)
{
    if (frameCapacity && queue.length() >= frameCapacity)
    {
        EV << "Queue full, dropping packet.\n";
        delete msg;
//        emit(qdropSignal, (int) 1); // emitted by PassiveQueueBase
        dropVec.record(1);
        return true;
    }
    else
    {
        queue.insert(msg);

        emit(qlenSignal, queue.length());

//        if (recordTrendOnly)
//        {
//        	if ( lastEvent == QE_DEQUEUE )
//        	{
//        		qlenVec.recordWithTimestamp(lastEventTime, queue.length()-1);
//        	}
//        }
//        else
//        {
//        	qlenVec.record(queue.length());
//        }

//        lastEvent = QE_ENQUEUE;
//        lastEventTime = simTime();
        return false;
    }
}

cMessage *DropTailQueue::dequeue()
{
    if (queue.empty())
        return NULL;

   cMessage *pk = (cMessage *)queue.pop();
   queueLength = queue.length();

    // statistics
   emit(qlenSignal, queue.length());
//   if (recordTrendOnly)
//   {
//	   if ( lastEvent == QE_ENQUEUE )
//	   {
//		   qlenVec.recordWithTimestamp(lastEventTime, queue.length()+1);
//	   }
//   }
//   else
//   {
//	   qlenVec.record(queue.length());
//   }
//
//    lastEvent = QE_DEQUEUE;
//    lastEventTime = simTime();
    return pk;
}

void DropTailQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}


