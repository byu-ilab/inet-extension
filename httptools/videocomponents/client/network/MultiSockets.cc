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

#include "MultiSockets.h"

MultiSockets::MultiSockets(cSimpleModule * module, int numSockets):numSockets(numSockets) {
	this->setNetwork(NULL);
	currSockId = 0;
	nextJobId = -1;
	initializing = false;
	ASSERT(conns == NULL); // socket must be closed.
	conns = new ConnMgr[numSockets];
	for (int i=0; i < numSockets; i++) {
		NetworkController * nc = new NetworkController(module);
		nc->setApplication(this);
		conns[i].controller = nc;
		conns[i].currSockId = -1;
		conns[i].state = ConnMgr::BEFORE;
		conns[i].oldSockId = -1;
	}
}

MultiSockets::~MultiSockets() {
	for (int i=0; i < numSockets; i++) {
		delete conns[i].controller;
	}
	delete [] conns;
}
// create one connection.  only can be called when socket is open.
int MultiSockets::addConnection() {
	++currSockId;
	//cout<<"Adding app connection "<<currSockId<<endl;
	for (int i=0; i < numSockets; i++) {
		int connId = conns[i].controller->addConnection();
		//cout<<"Creating subconnection "<<connId<<endl;
		conns[i].currSockId = connId;
		conns[i].state = ConnMgr::CONNECTING;
	}
	initializing = true;
	return currSockId;
}
void MultiSockets::cleanupConnection(int connId) {
	for (int i=0; i < numSockets; i++) {
		conns[i].controller->cleanupConnection(conns[i].currSockId);
	}
}
void MultiSockets::closeConnection(int connId) {
	//cout<<"closing all subconns for connection "<<connId<<endl;
	for (int i=0; i < numSockets; i++) {
		conns[i].controller->closeConnection(conns[i].currSockId);
	}
}
void MultiSockets::resumeConnectionJobs(int oldConnId, int newConnId) {
	// only called when all connections died and there were active jobs.
	//cout<<"All sockets were toast. resuming conn jobs"<<endl;
	ASSERT(jobs.size() > 0);
	//int * jobIds = jobs[appJob];
	for (int i=0; i < numSockets; i++) {
		ASSERT(conns[i].oldSockId != -1); // and there were active jobs on conns[i]??
		conns[i].controller->resumeConnectionJobs(conns[i].oldSockId, conns[i].currSockId);
	}
}
void MultiSockets::queueJob(int jobId, int connId, int jobSize) {
	//cout<<"Queueing job "<<jobId<<" for conn "<<connId<<" of size "<<jobSize<<endl;
	appJobQueue.push(jobId);
	Job * job = new Job(jobId, jobSize);
	appJobs[jobId] = job;
	int numGoodSockets = 0;
	for (int i=0; i < numSockets; i++) {
		if (conns[i].state == ConnMgr::CONNECTED || conns[i].state == ConnMgr::DOWNLOADING) {
			numGoodSockets++;
		}
	}
	//cout<<"There are "<<numGoodSockets<<" good sockets"<<endl;
	ASSERT(numGoodSockets > 0); // won't happen, because we trigger a callback when this occurs.
	jobs[jobId] = new int[numSockets];
	for (int i=0; i < numSockets; i++) {
		if (conns[i].state != ConnMgr::CONNECTED && conns[i].state != ConnMgr::DOWNLOADING) {
			jobs[jobId][i] = MultiSockets::CONN_IDLE;
			continue;
		}
		int evenJobSize = jobSize / numGoodSockets;
		int subJobSize = (i==0?(jobSize - (numGoodSockets-1)*evenJobSize ):(evenJobSize));
		int subJobId = ++nextJobId;
		netJobToApp[subJobId] = jobId;
		jobs[jobId][i] = subJobId;
		conns[i].state = ConnMgr::DOWNLOADING;
		conns[i].controller->queueJob(subJobId, conns[i].currSockId, subJobSize);
		//cout<<"queueing subjob "<<subJobId<<" for subconn "<<conns[i].currSockId<<" of size "<<subJobSize<<endl;
	}
}
Job * MultiSockets::getJobProgress(int jobId) {
	return appJobs[jobId];
}

/** Application calls.  I'm faking like I'm the application to the real network controllerss! */
void MultiSockets::connectionInitialized(int connId) {
	//cout<<"Subconnection "<<connId<<" initialized"<<endl;
	bool found = false;
	int connectedCount = 0;
	for (int i=0; i < numSockets; i++) {
		if (conns[i].currSockId == connId) {
			conns[i].state = ConnMgr::CONNECTED;
			if (conns[i].oldSockId != -1 && !initializing) {
				//cout<<"Resuming subconnection "<<connId<<"'s connection jobs"<<endl;
				conns[i].controller->resumeConnectionJobs(conns[i].oldSockId, conns[i].currSockId);
			}
			found = true;
		}
		if (conns[i].state == ConnMgr::CONNECTED|| conns[i].state == ConnMgr::DOWNLOADING) {
			connectedCount++;
		}
	}
	ASSERT(found);
	if (initializing && connectedCount == numSockets) {

		//cout<<"all conns for "<<currSockId<<" are initialized"<<endl;
		application->connectionInitialized(currSockId);
		initializing = false;
	}
}
void MultiSockets::connectionFailed(int connId) {
	//cout<<"Subconnection "<<connId<<" failed"<<endl;
	bool found = false;
	int numGoodSockets = 0;
	for (int i=0; i < numSockets; i++) {
		if (conns[i].currSockId == connId) {
			conns[i].state = ConnMgr::CONNECTING;
			int connId = conns[i].controller->addConnection();
			//cout<<"replacing Subconnection "<<connId<<" with subcon "<<connId<<endl;
			conns[i].oldSockId = conns[i].currSockId;
			conns[i].currSockId = connId;
			found = true;
			break;
		}
		// check to see if any alive.
		if (conns[i].state == ConnMgr::CONNECTED || conns[i].state == ConnMgr::DOWNLOADING) {
			numGoodSockets++;
		}
	}
	//cout<<" NumGoodSockets: "<<numGoodSockets<<endl;
	if (numGoodSockets == 0 && !initializing) {
		//cout<<"No good sockets left. Telling application about "<<currSockId<<endl;
		application->connectionFailed(currSockId);
	}
	ASSERT(found);
}
void MultiSockets::jobFinished(int jobId) {
	//cout<<"Subjob "<<jobId<<" finished"<<endl;
	int appJob = netJobToApp[jobId];
	int * jobIds = jobs[appJob];
	int numFinishedOrIdle = 0;
	for (int i=0; i < numSockets; i++) {
		if (jobIds[i] == jobId) {
			jobIds[i] = MultiSockets::COMPLETE;
		}
		if (jobIds[i] < 0) {
			numFinishedOrIdle++;
		}
	}
	//cout<<"subjobs finished/idle: "<<numFinishedOrIdle<<endl;
	if (numFinishedOrIdle == numSockets) {
		//cout<<"AppJob "<<appJob<<" finished"<<endl;
		ASSERT(appJob == appJobQueue.front());
		application->jobProgress(appJob);// must be called at least once.
		application->jobFinished(appJob);
		appJobQueue.pop();
		// erase stuff!
		for (int i=0; i < numSockets; i++) {
			netJobToApp.erase(jobIds[i]);
		}
		delete [] jobIds;
		jobs.erase(appJob);
		delete appJobs[jobId];
		appJobs.erase(jobId);
	}
}
void MultiSockets::jobProgress(int jobId) {
	int appJobId = netJobToApp[jobId];
	//cout<<"Job progress for job "<<appJobId<<", (subjob "<<jobId<<")"<<endl;
	// figure out which app sent, and how much. update this job.
	int * subJobs = jobs[appJobId];
	int i=0;
	for (; i < numSockets; i++) {
		if (subJobs[i] == jobId) {
			break;
		}
	}
	ASSERT( i < numSockets);
	NetworkController * nc = conns[i].controller;
	Job * netJob = nc->getJobProgress(jobId);
	Job * appJob = appJobs[appJobId];
	appJob->receivedBytes(netJob->getLastBytesRecvd());
	//cout<<"Progress made: "<<appJob->getLastBytesRecvd()<<" bytes"<<endl;
	if (appJobId == appJobQueue.front()) {
		application->jobProgress(appJobId);
	}
}
void MultiSockets::handleCallback(short type) {
	ASSERT(false);
}
