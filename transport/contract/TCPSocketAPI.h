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

#include <omnetpp.h>
#include "INETDefs.h"
#include "httptLogdefs.h"

#include "TCPSocket.h"
#include "TCPSocketMap.h"

#include <string>
#include <map>

#define SOCK_CB void (*callback_function)(int socket_id, int ret_value, void * yourPtr)

enum CALLBACK_TYPE {CONNECT, ACCEPT, RECV};

struct CallbackData {
	int socket_id;
	SOCK_CB;
	void * function_data;
	CALLBACK_TYPE type;
};

class INET_API TCPSocketAPI : public cSimpleModule, TCPSocket::CallbackInterface
{
protected:
	TCPSocketMap _socket_map;

	// port -> callback
	std::map<int, CallbackData *> _accept_callbacks;

//	// port -> socket *
//	std::map<int, TCPSocket *> _listening_sockets;

	// socket_id -> callback
//	std::map<int, CallbackData *> _registered_callbacks;
//
//	CallbackData * _default_recv_callback;
//	CallbackData * _default_accept_callback;

public:

	// @brief This function is similar to calling the BSD socket API
	//		socket(AF_INET, SOCK_STREAM, 0)
	//
	// @return the id of a new socket or -1 if and error occurs
	virtual int socket ();

	// if local_address is "" then will assign the socket to any available IP address
	// throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	virtual void bind (int socket_id, std::string local_address, int local_port);

	// @throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	// @return the value returned in the callback function is -1 if an error occurs, otherwise returns 0
	virtual void connect (int socket_id, std::string remote_address, int remote_port, void * yourPtr,
			SOCK_CB );

	// @throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	virtual void listen (int socket_id);

	// @return the value returned in the callback function is -1 if an error occurs,
	// otherwise it is the socket id of the newly created socket
	virtual void accept (int socket_id, void * yourPtr, SOCK_CB);

//	// registerAcceptCallback must be called before this convenient form can be used
//	virtual void accept(int socket_id, void * yourPtr);
//
//	virtual void registerAcceptCallback(int socket_id, SOCK_CB);
//
//	virtual void registerDefaultAcceptCallback(SOCK_CB);

	// sends the data (there doesn't appear to be any buffer limits)
	virtual void send (int socket_id, std::string data);

	// @throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	// @return the value returned in the callback function is -1 if an error occurs, otherwise
	// returns 0, or the number of bytes returned by receive
	virtual void recv (int socket_id, void * yourPtr, SOCK_CB);

//	// registerRecvCallback must be called before thsi convenient form can be used
//	virtual void recv (int socket_id, void * yourPtr);
//
//	virtual void registerRecvCallback (int socket_id, SOCK_CB);
//
//	virtual void registerDefaultRecvCallback(SOCK_CB);

	// @throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	virtual void close (int socket_id);


protected:

	// cSimpleModule functions
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    // TCPSocket::CallbackInterface functions
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) = 0;
	virtual void socketEstablished(int connId, void *yourPtr) {}
	virtual void socketPeerClosed(int connId, void *yourPtr) {}
	virtual void socketClosed(int connId, void *yourPtr) {}
	virtual void socketFailure(int connId, void *yourPtr, int code) {}

	// implement if other things should be done other than delete the status info
	//virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}

	// utility / convenience functions
	TCPSocket * findAndCheckSocket(std::string method);

	void registerCallbackData (TCPSocket * socket, CallbackData * cbdata);

	void makeCallbackData(int socket_id, SOCK_CB, void * function_data, CALLBACK_TYPE type);
};

#endif
