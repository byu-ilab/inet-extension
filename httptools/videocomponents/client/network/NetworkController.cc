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
#include <omnetpp.h>
#include "IApplicationControl.h"
#include "NetworkController.h"
#include "TCPSocketMgrAppUtils.h"

bytecount_t Worker::RECV_MODE =TCPSocketAPI_Inet::RECV_MODE_INSTANT_MAINTAIN_BOUNDARIES;
NetworkController::NetworkController(cSimpleModule * module) {
	// TODO Auto-generated constructor stub
	this->module = module;
	socketmgr = findTCPSocketMgr(module);
	controller = check_and_cast<httptController *>(findModuleSomewhereUp(module->par("controller"), module));
	ipaddr_resolver = new IPAddressResolver();
}

NetworkController::~NetworkController() {
	// TODO Auto-generated destructor stub
}

int NetworkController::addConnection() {
	int s = socketmgr->socket(this);
	socketmgr->connect(s, ipaddr_resolver->addressOf(controller->getServerModule(module->par("serverwww"))->getParentModule()).str(), module->par("port"));
	sockets[s] = new Worker(s);
	return s;
}
/**
 * Cleans up jobs associated with this connection.
 */
void NetworkController::cleanupConnection(int connId) {
	SocketMap::iterator finder = sockets.find(connId);
	ASSERT(finder != sockets.end());
	Worker * socketInfo = finder->second;

	delete socketInfo;
	sockets.erase(finder);
}
void NetworkController::closeConnection(int connId) {
	socketmgr->close(connId);
}
void NetworkController::resumeConnectionJobs(int oldConnId, int newConnId) {
	SocketMap::iterator finder = sockets.find(oldConnId);
	ASSERT(finder != sockets.end());
	Worker * socketWorker = finder->second;
	socketWorker->setSocketId(newConnId);

	sockets.erase(finder);
	sockets[newConnId] = socketWorker;

	// resume each job.
	JobQueue & queue = socketWorker->getJobQueue();
	for (unsigned int i=0; i < queue.size(); i++) {
		Job * job = queue[i];
		ASSERT(job->getState() == PAUSED);
		int left = job->getRemainingBytes();
		job->setState(QUEUEING);
		cPacket * request = generateRequest(left);
		socketmgr->send(newConnId, request);
		cout<<"Resending request of size "<<left<<" for job "<<job->getId()<<endl;
	}
	socketmgr->recv(newConnId, Worker::RECV_MODE); // start recv'ing again!
}

cPacket * NetworkController::generateRequest(int numBytes) {
	// Construct the HTTP request
	const char * hdr = "GET /Segment HTTP/1.1";
	httptRequestMessage * request = new httptRequestMessage(hdr, HTTPT_REQUEST_MESSAGE);
	request->setHeaderLength(1);
	request->setContentLength(0);
	request->setByteLength(1); // = |header| + |content|
	request->setEntitySize(numBytes); // size of the entity to be returned.
	request->setUri("/Segment");
	request->setProtocol(11); // 1.1
	request->setHeading(hdr);
	request->setTargetUrl(module->par("serverwww"));
	request->setKeepAlive(true);
	return request;
}

/** Inherited from INetworkControl **/
void NetworkController::queueJob(int jobId, int connId, int numBytes) {

	Job * job = new Job(jobId, numBytes);
	jobs[jobId] = job;

	SocketMap::iterator finder = sockets.find(connId);
	ASSERT(finder != sockets.end());
	Worker * socket = finder->second;

	cPacket * request = generateRequest(numBytes);
	socketmgr->send(connId, request);

	socket->enqueueJob(job);

	if (socket->getNumPendingResponses() == 1) { // call recv here if this is the only pending response; otherwise call recv in recvCallback.
		socketmgr->recv(connId, Worker::RECV_MODE);
	}
}

Job * NetworkController::getJobProgress(int jobId) {
	JobMap::iterator jobFinder = jobs.find(jobId);
	ASSERT(jobFinder != jobs.end());
	return jobFinder->second;
}

bool NetworkController::socketErrorCheck(socket_id_t socket_id, cb_status_t result) {
	if (TCPSocketAPI_Inet::isCallbackError(result))
	{
		socketmgr->close(socket_id);
		SocketMap::iterator finder = sockets.find(socket_id);
		ASSERT(finder != sockets.end());
		Worker * socket = finder->second;
		socket->socketFailure();
		application->connectionFailed(socket_id);
		return true;
	}
	return false;
}

/** Extension of TCPSocketAPI_Inet::CallbackHandler */
void NetworkController::acceptCallback(socket_id_t id, cb_status_t result, user_data_ptr_t context) {
	module->error("This method (AcceptCallback) should not be called, as this is the client");
}
void NetworkController::connectCallback(socket_id_t id, cb_status_t result, user_data_ptr_t context) {
	if (socketErrorCheck(id, result)) {
		return;
	}
	application->connectionInitialized(id);
}
httptReplyMessage * NetworkController::extractReply(cPacket * msg) {
	ASSERT(msg != NULL);
	httptReplyMessage * reply = NULL;
	MsgByteBuffer * mbb = dynamic_cast<MsgByteBuffer *>(msg);
	ASSERT(mbb != NULL);
	if (mbb != NULL) {
		int pa_size = mbb->getPayloadArraySize();
		ASSERT(pa_size <= 1);
		if (pa_size == 1) {
			reply = dynamic_cast<httptReplyMessage *>(mbb->removeFirstPayloadMessage());
		} else {
			msg = NULL;
		}
		delete mbb;
	}
	return reply;
}
void NetworkController::recvCallback(socket_id_t id, cb_status_t result, cPacket * msg, user_data_ptr_t yourPtr) {
	if (socketErrorCheck(id, result)) {
		return;
	}
	SocketMap::iterator sock_itr = sockets.find(id);
	ASSERT(sock_itr != sockets.end());
	Worker * socket = sock_itr->second;
	Job * job = socket->getHeadJob();
	ASSERT(job != NULL); // Must be a job to expect...

	int appBytes = result;
	// detect new message.
	if (msg != NULL) {
		httptReplyMessage * response = extractReply(msg);
		//ASSERT(response != NULL); // Reply must be the right message type
		if (response != NULL) {
			ASSERT(job->getState() != RECEIVING); // This must be the first packet that the job is receiving.
			ASSERT(response->result() == HTTP_CODE_200);
			job->expectResponse(response->contentLength());

			// change result so that it is the number of app bytes of result.
			appBytes -= response->headerLength();
			if (appBytes < 0)
				appBytes = 0;

			delete response;
		}
	}
	ASSERT(job->getState() == RECEIVING); // cannot reach here without calling expectResponse()

	job->receivedBytes(appBytes); // send bytes toward message.
	int left = job->getRemainingBytes();
	ASSERT(left >= 0);


	application->jobProgress(job->getId());
	// is current message complete?
	if (job->getState() == COMPLETE) {
		application->jobFinished(job->getId());
		socket->dequeueJob();

		//erase.
		JobMap::iterator jobFind = jobs.find(job->getId());
		ASSERT(jobFind != jobs.end());
		jobs.erase(jobFind);
		delete job;

		if (socket->getHeadJob() != NULL) { // if pending replies, recv again.
			socketmgr->recv(id, Worker::RECV_MODE);
		}
	} else { // state is receiving.
		socketmgr->recv(id, Worker::RECV_MODE);
	}
}
