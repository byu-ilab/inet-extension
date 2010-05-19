//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_TCPGENERICSRVAPP_H
#define __INET_TCPGENERICSRVAPP_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "TCPSocketAPI.h"

/**
 * Generic server application. It serves requests coming in GenericAppMsg
 * request messages. Clients are usually subclassed from TCPGenericCliAppBase.
 *
 * @see GenericAppMsg, TCPGenericCliAppBase
 */
class INET_API TCPSAPIGenericSrvApp : public cSimpleModule, TCPSocketAPI::CallbackInterface
{
  protected:
//	  int port;
    simtime_t delay;
    simtime_t maxMsgDelay;

    TCPSocketAPI * socketapi;

//    cMessage * mymsg;

    long msgsRcvd;
    long msgsSent;
    long bytesRcvd;
    long bytesSent;

  protected:
//    virtual void sendBack(cMessage *msg);
//    virtual void sendOrSchedule(cMessage *msg, simtime_t delay);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual bool hasCallback (TCPSocketAPI::CALLBACK_TYPE type);
    //virtual void connectCallback(int socket_id, int ret_status, void * myPtr);
    virtual void acceptCallback (int socket_id, int ret_status, void * myPtr);
    virtual void recvCallback   (int socket_id, int ret_status, cPacket * msg, void * myPtr);
};

#endif


