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


/**
 * A Network Controller that will download segments of arbitrary size that are
 * assigned to it from the application.
 *
 * Assumptions:
 * - Segments are downloaded in order of assignment (no parallel downloading of
 *		multiple concurrent segments)
 * - Pipelining is allowed, in other words, requests are sent as soon as the application
 * 		begins its download.
 * Download Policy:
 * - Segments are downloaded via byte ranges among existing (connected) connections.
 * - Byte ranges are divided equally among connections.
 * - On socket failure, a new socket is created, which resumes download of the remaining
 * 		bytes from the failed socket.
 */
#ifndef NETWORKCONTROLLER_H_
#define NETWORKCONTROLLER_H_
#include <omnetpp.h>
#include "TCPSocketMgr.h"
#include "httptController.h"
#include "IPAddressResolver.h"
#include "TCPSocketAPI_Inet.h"
#include "INetworkControl.h"
#include "ModuleAccess.h"
#include "Job.h"
#include "Worker.h"
#include <iostream>
#include <map>
#include "MsgByteBuffer.h"
using namespace std;



class NetworkController: public INetworkControl,
	public TCPSocketAPI_Inet::CallbackHandler
{
private:
	cSimpleModule * module;
	TCPSocketMgr * socketmgr;
	httptController * controller;
	IPAddressResolver * ipaddr_resolver;
	string host_name;
	int port;
	typedef map<int, Worker *> SocketMap;
	typedef map<int, Job *> JobMap;
	SocketMap sockets;
	JobMap jobs;
	httptReplyMessage * extractReply(cPacket * msg);
	bool socketErrorCheck(socket_id_t socket_id, cb_status_t result);
	cPacket * generateRequest(int numBytes);
public:
	NetworkController(cSimpleModule * app);
	virtual ~NetworkController();

    /** Inherited from INetworkControl **/
	/**
	 * schedules opening a socket, returns socket id.
	 */
	virtual int addConnection();

	/**
	 * cleansUp a closed connection (removes all jobs).
	 *
	 * @pre socket must have failed/timed out, then close must have been called.
	 */
	virtual void cleanupConnection(int connId);

	/**
	 * closes a connection (removes all jobs).
	 *
	 * @pre socket must have failed/timed out, then close must have been called.
	 */
	virtual void closeConnection(int connId);

	/**
	 * resume all jobs on connection oldConn using the connection newConnId
	 * Assumption: the only reason that a connection stops is due to socket failure.
	 * @pre oldConnId, newConnId are existing, valid socket ids.
	 */
	virtual void resumeConnectionJobs(int oldConnId, int newConnId);

	/**
	 * tells a connection to start work on a job of downloading jobSize bytes
	 * @pre connId must be a valid socket.
	 */
	virtual void queueJob(int jobId, int connId, int jobSize);

	/**
	 * asks for progress made on a specific Job
	 * @pre job jobId must be a valid job
	 */
	virtual Job * getJobProgress(int jobId);

protected:
	/** @name Extension of TCPSocketAPI_Inet::CallbackHandler */
	//@{
	virtual void acceptCallback  (socket_id_t id, cb_status_t result, user_data_ptr_t context);
	virtual void connectCallback (socket_id_t id, cb_status_t result, user_data_ptr_t context);
	virtual void recvCallback(socket_id_t id, cb_status_t return_value, cPacket * msg, user_data_ptr_t yourPtr);
	//@}

};

#endif /* NETWORKCONTROLLER_H_ */
