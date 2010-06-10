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
#include "httptNodeBase.h"

#include <iostream>
#define BR_INFO std::cout

struct RangeRequestInfo
{
	int request_id;
	int range_id;

	RangeRequestInfo(int req_id, int rng_id)
	{
		request_id = req_id;
		range_id = rng_id;
	}
};

class ByteRangeClient : public httptNodeBase, TCPSocketAPI::CallbackInterface
{
protected:
	TCPSocketAPI * _socketapi;
	httptController * _controller;
	int _range_size;
	int _file_size;
	int _num_ranges;
	int _num_requests_to_make;
	int _num_requests_made;
	int _request_round;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    virtual void connectCallback(int socket_id, int ret_status, void * myPtr);
    virtual void recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr);

    virtual int createSocket(int request_id, int range_id);
    virtual httptRequestMessage * generateBRRequest(const std::string & uri, int fbp, int lbp);
    virtual void sendBRRequest(int socket_id, int request_id, int range_id);
};

#endif
