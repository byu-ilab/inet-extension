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

#include "AdditiveController.h"
#include "VerticalPolicy.h"
#include "MeanVerticalPolicy.h"
#include "HorizontalPolicy.h"
#include "DiagonalPolicy.h"



AdditiveController::AdditiveController(IModule * module, cSimpleModule * mainModule): module(module), host(mainModule) {

	string policy_s = host->par("policy").str();
	policy_s.erase(
	    remove( policy_s.begin(), policy_s.end(), '\"' ),
	    policy_s.end()
	    );
	if (policy_s.compare("vertical") == 0) {
		policy = new VerticalPolicy();
	} else if (policy_s.compare("meanvertical") == 0) {
		policy = new MeanVerticalPolicy();
	} else if (policy_s.compare("horizontal") == 0) {
		policy = new HorizontalPolicy();
	} else if (policy_s.compare("diagonal") == 0) {
		policy = new DiagonalPolicy();
	} else {
		host->error("Unknown policy.");
	}
	segmentDuration = host->par("segmentDuration");
	playback = new VideoPlayback(host->par("numSegments"), host);
	blockSize = host->par("blockSize");
	connId = oldConnId = -1;
	nextJobId = 0;
	state = BEFORE;
	next_job_send_time = MAXTIME;
	lastJobSent = -1;
	currJob = -1;
	currJobProgress = 0;
	isRequestPending = false;
	simtime_t start_time = host->par("startTime");
	module->scheduleCallback(start_time, BEGIN);
}

AdditiveController::~AdditiveController() {
	delete policy;
	delete playback;
}

void AdditiveController::sendRequest()
{
	if (oldConnId == -1) { // normal request
		state = DOWNLOADING;
		int segment = policy->selectSegment(&buffer, playback, networkMonitor.getRate());
		buffer.requestedBlock(segment);
		int job = nextJobId++;
		activeJobs[job] = segment;
		lastJobSent = job;
		network->queueJob(job, connId, blockSize);
		cout<<"At time "<<simTime()<<", Sending Request for Segment "<<segment<<", jobId "<<job<<endl;
		networkMonitor.setLastSendTime(simTime());
		//send_time = simTime(); // used for rtt measurement.
	} else { // there was a failed connection.  Queue this for later.
		isRequestPending = true;
	}
}

void AdditiveController::connectionInitialized(int connId) {
	this->connId = connId;
	if (this->oldConnId == -1) { // first time connection initialized.
		module->scheduleCallback(simTime(), SEND_REQUEST);
		module->scheduleCallback(simTime() + segmentDuration, ADVANCE_PLAYBACK);
		next_job_send_time = simTime();
	} else if (this->oldConnId != -1 && activeJobs.size() > 0) { // old connection failed while busy.
		network->resumeConnectionJobs(oldConnId, connId);
	}
	if (this->oldConnId != -1){ // old conn failed
		// resume jobs that were to be completed while connection had failed.
		if (isRequestPending) {
			module->scheduleCallback(simTime(), SEND_REQUEST);
		}
	}
	oldConnId = -1;
}
void AdditiveController::connectionFailed(int connId) {
	oldConnId = connId;
	network->addConnection();
}
void AdditiveController::jobFinished(int jobId) {

	bool valid = buffer.receivedBlock(activeJobs[jobId]);
	//cout<<"At time "<<simTime()<<", job "<<jobId<<" completed.";
	//if (!valid) {
	//	cout<<" REJECTED";
	//}cout<<endl;

	if (!playback->canDownload()) {
		state = DONE;
	} else if (playback->videoBuffering()) {
		playback->networkReady(&buffer);
		module->scheduleCallback(simTime() + segmentDuration, ADVANCE_PLAYBACK);
	}
}
void AdditiveController::measureNetwork(int jobId)
{
	// measure rate
	networkMonitor.updateRate();

	// measure rtt
	//cout<<"At time "<<simTime()<<", rate="<<rate<<endl;
    if (jobId == lastJobSent) {
        networkMonitor.updateRTT();
    }
}

void AdditiveController::rescheduleNextRequest(int jobId) {
	// did the 'next' request not get sent already?
	if (jobId == lastJobSent) {
		// recalculate next job send time.
		int bytesLeft = network->getJobProgress(jobId)->getRemainingBytes();
		double targetSend = networkMonitor.getNextRequestTime(bytesLeft);

		if (abs(next_job_send_time.dbl() - targetSend) > .001) {
			module->cancelCallback(SEND_REQUEST);
			double timeFromNow = max(0.0, targetSend);
			next_job_send_time = simTime() + timeFromNow;
			module->scheduleCallback(next_job_send_time, SEND_REQUEST);
		}
	}
}
void AdditiveController::jobProgress(int jobId) {
	ASSERT(state == DOWNLOADING || state == DONE);
	if (state == DONE) {
		return;
	}
	if (!playback->canDownload()) { // can't send any more requests
		module->cancelCallback(SEND_REQUEST);
	} else {
		if (currJob != jobId) {
			if (currJob >= 0) {
				networkMonitor.receivedBytes(blockSize - currJobProgress);
			}
			currJob = jobId;
			currJobProgress = 0;
		}
		int bytes_recvd = network->getJobProgress(jobId)->getProgress() - currJobProgress;
		networkMonitor.receivedBytes(bytes_recvd);
		currJobProgress = network->getJobProgress(jobId)->getProgress();
		measureNetwork(jobId);
		rescheduleNextRequest(jobId);
	}
}
void AdditiveController::handleCallback(short type) {
	int qual;
	switch(type) {
	case BEGIN:
		state = CONNECTING;
		network->addConnection();
		break;
	case ADVANCE_PLAYBACK:
		cout<<"At time "<<simTime()<<", Advancing playback head to segment "<<(playback->getHeadPosition()+1);
		qual = playback->advanceHead(&buffer);
		if (playback->videoBuffering()) {
			cout<<" and Buffering";
		} else if (playback->videoComplete()) {
			cout<<" and Complete";
		} cout<<"; quality "<<qual<<endl;
		if (!playback->videoBuffering() && !playback->videoComplete()) {
			module->scheduleCallback(simTime() + segmentDuration, ADVANCE_PLAYBACK);
		}
		break;
	case SEND_REQUEST:
		sendRequest();
		break;
	default:
		break;
	}
}
