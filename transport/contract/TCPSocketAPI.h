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
#include "IPAddressResolver.h"
#include "SocketTimeoutMsg_m.h"

#include <string>
#include <map>

enum CALLBACK_TYPE {CB_T_CONNECT, CB_T_ACCEPT, CB_T_RECV};

enum CALLBACK_STATE {CB_S_NONE, CB_S_CONNECT, CB_S_ACCEPT, CB_S_RECV, CB_S_CLOSE, CB_S_TIMEOUT};

enum CALLBACK_ERROR {
	CB_E_TIMEOUT = -2,
	CB_E_UNKNOWN = -1,
	CB_E_CLOSED = 0
};

class INET_API TCPSocketAPI : public cSimpleModule, TCPSocket::CallbackInterface
{
public:

	// Defines the functions that must be implemented to make the TCPSocketAPI's
	// callbacks work.
	class CallbackInterface {
	public:

		// Should return true if the object implementing this interface
		// implements the function for the indicated callback type and
		// false if it does not
		virtual bool hasCallback (CALLBACK_TYPE type) =0;

		// @param socket_id -- the id of the connected socket
		// @param ret_status -- the status of the previously invoked connect method
		// @param yourPtr -- the pointer to the data passed to the connect method
		virtual void connectCallback (int socket_id, int ret_status, void * yourPtr) {}

		// @param socket_id -- the id of the accepted socket
		// @param ret_status -- the status of the previously invoked accept method or
		//						the id of the accepted socket
		// @param yourPtr -- the pointer to the data passed to the accept method
		virtual void acceptCallback  (int socket_id, int ret_status, void * yourPtr) {}

		// @param socket_id -- the id of the accepted socket
		// @param ret_status -- the status of the previously invoked recv method or
		//						the number of bytes received
		// @param msg -- a pointer to the received message
		// @param yourPtr -- the pointer to the data passed to the accept method
		virtual void recvCallback    (int socket_id, int ret_status, cPacket * msg,
										void * yourPtr) {}
	};

protected:

	struct CallbackData {
		int socket_id;
		CALLBACK_STATE state;

		CallbackInterface * cbobj;
		void * function_data;
		CallbackInterface * cbobj_for_accepted;
	};

	TCPSocketMap _socket_map;

	// port -> callback data
	std::map<int, CallbackData *> _accept_callbacks;

	// socket id -> callback data
	std::map<int, CallbackData *> _registered_callbacks;

	// socket id -> timeout message
	std::map<int, SocketTimeoutMsg *> _timeout_timers;

	IPAddressResolver _resolver;

public:

	TCPSocketAPI ();
	~TCPSocketAPI ();

	// @brief This function is similar to calling the BSD socket API
	//		socket(AF_INET, SOCK_STREAM, 0)
	//
	// @return the id of a new socket
	// @throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	virtual int socket (CallbackInterface * cbobj);

	// if local_address is empty (i.e. "") then will assign the socket to any
	// available IP address
	// @throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	virtual void bind (int socket_id, std::string local_address, int local_port);

	// @throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	// @callback the ret_status value will be -1 if an error occurred and 0 otherwise
	virtual void connect (int socket_id, std::string remote_address, int remote_port,
				void * yourPtr=NULL);

	// @param cbobj_for_accepted -- the CallbackInterface for connected spawned by
	// 								this listening socket
	// @throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	virtual void listen (int socket_id, CallbackInterface * cbobj_for_accepted=NULL);

	// calls socket, bind, and connect
	// if local_port is negative then will not call bind
	// @return the socket id
	virtual int makeActiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, std::string remote_address, int remote_port, void * yourPtr=NULL);

	// calls socket, bind, and listen
	// @return the socket id
	virtual int makePassiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, CallbackInterface * cbobj_for_accepted=NULL);

	// @callback the ret_status value will be -1 if an error occurred and otherwise the
	// socket_id of the accepted socket
	virtual void accept (int socket_id, void * yourPtr=NULL);

	// sends the data (there doesn't appear to be any buffer limits)
	virtual void send (int socket_id, cMessage * msg);

	// build a send function that will take a translator and data to get the cMessage?

	// @throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	// @callback the ret_status value will be -1 if an error occurs, 0 if the socket
	// is closed, and the number of bytes of the received message otherwise
	// the msg pointer will point to the received message or be NULL if an error occurs
	// or the socket is closed
	virtual void recv (int socket_id, void * yourPtr=NULL);

	virtual void setTimeout(int socket_id, simtime_t timeout_interval);

	// @return true if there was a timeout set and false otherwise
	virtual bool removeTimeout(int socket_id);

	// @throws a cRuntimeError if an error occurs (see omnetpp/include/cexception.h)
	virtual void close (int socket_id);

	// @return the void pointer to the data previously set in a connect, accept, or
	// recv call.  Returns NULL if the socket_id doesn't pertain to a current socket
	// or if the provided void pointer is NULL.
	virtual void * getMyPtr (int socket_id);

protected:

	// cSimpleModule functions
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    // TCPSocket::CallbackInterface functions
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
	virtual void socketEstablished(int connId, void *yourPtr);
	virtual void socketPeerClosed(int connId, void *yourPtr);
	virtual void socketClosed(int connId, void *yourPtr);
	virtual void socketFailure(int connId, void *yourPtr, int code);

	// implement if other things should be done other than delete the status info
	//virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status);

	// utility / convenience functions
		// NOT part of the TCPSocket::CallbackInteface
	virtual void socketTimeout(int connId, void * yourPtr);

	virtual TCPSocket * findAndCheckSocket(int socket_id, std::string method);

	virtual CallbackData * makeCallbackData(int socket_id, CallbackInterface * cbobj,
			void * function_data, CALLBACK_STATE type);
};

#endif
