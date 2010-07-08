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


#ifndef __INET_DROPTAILQUEUE_H
#define __INET_DROPTAILQUEUE_H

#include <omnetpp.h>
#include "PassiveQueueBase.h"

enum QUEUE_EVENT { QE_NONE, QE_ENQUEUE, QE_DEQUEUE };

/**
 * Drop-front queue. See NED for more info.
 */
class INET_API DropTailQueue : public PassiveQueueBase
{
  protected:
    // configuration
    int frameCapacity;
    int queueLength;
    // state
    cQueue queue;
    cGate *outGate;

    // statistics
    cOutVector qlenVec;
    cOutVector dropVec;

    bool recordTrendOnly;
    QUEUE_EVENT lastEvent;
    simtime_t lastEventTime;

  protected:
    virtual void initialize();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual bool enqueue(cMessage *msg);

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual cMessage *dequeue();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void sendOut(cMessage *msg);
};

#endif


