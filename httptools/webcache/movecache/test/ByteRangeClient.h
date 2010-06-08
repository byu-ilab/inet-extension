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

#ifndef __BYTERANGECLIENT_H__
#define __BYTERANGECLIENT_H__

#include <omnetpp.h>
#include "TCPSocketAPI.h"
#include "TCPSocketAPIAppUtils.h"
#include "httptController.h"
#include "httptLogdefs.h"
#include "httptByteRangeMessages_m.h"
#include "httptNodeBase.h"

class ByteRangeClient : public httptNodeBase, TCPSocketAPI::CallbackInterface
{
protected:
	TCPSocketAPI * _socketapi;
	httptController * _controller;
	int _range_size;
	int _file_size;
	int _num_ranges;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    virtual void connectCallback(int socket_id, int ret_status, void * myPtr);
    virtual void recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr);

    virtual int createSocket(int range_id);
    virtual httptByteRangeRequestMessage * generateBRRequest(const std::string & uri, int fbp, int lbp);
};

#endif
