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

#ifndef __DUMMYSERVER_H__
#define __DUMMYSERVER_H__

#include <omnetpp.h>
#include "httptServerBase.h"
#include "TCPSocketAPI.h"
#include "TCPSocketAPIAppUtils.h"

class DummyServer : public httptServerBase, TCPSocketAPI::CallbackInterface
{
protected:
	TCPSocketAPI * _socketapi;
	int _listening_fd;
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    virtual void acceptCallback(int socket_id, int ret_status, void * myPtr);
    virtual void recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr);
};

#endif
