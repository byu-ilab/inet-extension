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

#ifndef MULTISOCKETNETWORKCONTROLLER_H_
#define MULTISOCKETNETWORKCONTROLLER_H_

#include "INetworkControl.h"
#include "IApplicationControl.h"
#include "NetworkController.h"
#include <map>
#include <queue>
struct ConnMgr {
	NetworkController * controller;
	int currSockId;
	int oldSockId;
	enum {BEFORE, CONNECTING, CONNECTED,DOWNLOADING, DONE}state;

};

typedef map<int, Job *> AppJobMap;
class MultiSockets: public INetworkControl, public IApplicationControl {
private:
	int numSockets;
	ConnMgr * conns;
	map<int,int*> jobs; // from job to subJobs
	map<int,int> netJobToApp; // subJobs to job
	AppJobMap appJobs;
	int currSockId;
	int nextJobId;
	queue<int> appJobQueue;
	bool initializing;
	enum {CONN_IDLE=-3,COMPLETE};
public:
	MultiSockets(cSimpleModule * module, int numSockets);
	virtual ~MultiSockets();
    /** Inherited from INetworkControl. I.e., to application, I look like a network. **/
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

	/** Application calls.  I'm faking like I'm the application to the real network controllerss! */
	virtual void connectionInitialized(int connId);
	virtual void connectionFailed(int connId);
	virtual void jobFinished(int jobId);
	virtual void jobProgress(int jobId);
	virtual void handleCallback(short type);

};

#endif /* MULTISOCKETNETWORKCONTROLLER_H_ */
