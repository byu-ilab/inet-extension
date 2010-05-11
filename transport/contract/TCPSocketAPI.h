// Author: Kevin Black
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

#ifndef __INET_TCPSOCKETAPI_H
#define __INET_TCPSOCKETAPI_H

#include <string>
#include <omnetpp.h>
#include "INETDefs.h"

#include "TCPSocket.h"
#include "TCPSocketMap.h"

#define SOCK_CB void (*callback)(int socket_id, int ret_value, void * yourPtr)

class INET_API TCPSocketAPI : public cSimpleModule
{
public:
	// @brief This function is similar to calling the BSD socket API
	//		socket(AF_INET, SOCK_STREAM, 0)
	//
	// @return the id of a new socket or -1 if and error occurs
	virtual int socket ();

	// @return -1 if an error occurs otherwise returns 0
	virtual int bind (int socket_id, std::string local_address, short local_port);

	// @return the value returned in the callback function is -1 if an error occurs,
	// otherwise returns 0
	virtual void connect (int socket_id, std::string remote_address, short remote_port, void * yourPtr,
			SOCK_CB );

	// @return -1 if an error occurs otherwise returns 0
	virtual int listen (int socket_id);

	// @return the value returned in the callback function is -1 if an error occurs,
	// otherwise it is the socket id of the newly created socket
	virtual void accept (int socket_id /*, address structure? */, void * yourPtr, SOCK_CB);

	// @return the number of bytes sent
	virtual int send (int socket_id, std::string data);

	// @return the value returned in the callback function is -1 if an error occurs,
	// 0 if the socket is closed, or the number of bytes returned by receive
	virtual void recv (int socket_id, void * yourPtr, SOCK_CB);

	// @return -1 if an error occurs otherwise returns 0
	virtual int close (int socket_id);




protected:

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
