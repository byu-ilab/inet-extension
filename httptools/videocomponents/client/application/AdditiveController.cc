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
#include "NonAdditivePolicy.h"
#include "AdditiveCodec.h"
#include "NonAdditiveCodec.h"

AdditiveController::AdditiveController(IModule * module, cSimpleModule * mainModule):
	module(module), host(mainModule), buffer(mainModule->par("segmentDuration"),
	mainModule->par("maxBufferSizeKB"), mainModule->par("epsilon")),networkMonitor(mainModule->par("rttMeasurementAlpha"),
	mainModule->par("rateMeasurementAlpha"),mainModule->par("meanRateAlpha"),
	mainModule->par("useRealMean")) {

	/*string policy_s = host->par("policy").str();
	policy_s.erase(
	    remove( policy_s.begin(), policy_s.end(), '\"' ),
	    policy_s.end()
	    );
	    */
	int pol_enum = host->par("policy");
	double slope = -1.0;

	cout<<"Policy: "<<pol_enum<<endl;
	codec = NULL;
	switch(pol_enum) {
	case VERTICAL: // 0
		policy = new VerticalPolicy();
		break;
	case MEANVERTICAL: // 1
		codec = new AdditiveCodec(host->par("blockSize"));
		policy = new MeanVerticalPolicy(codec->getBlockSize(0,1));
		break;
	case HORIZONTAL: // 2
		policy = new HorizontalPolicy();
		break;
	case NONADDITIVE: // 3
		codec = new NonAdditiveCodec(host->par("blockSize"));
		policy = new NonAdditivePolicy(0.8, codec);
		break;
	case DIAG_N1: // 4
		slope = -1.0;
		policy = new DiagonalPolicy(slope);
		break;
	case DIAG_NHALF: // 5
		slope = -0.5;
		policy = new DiagonalPolicy(slope);
		break;
	case DIAG_N2: // 6
		slope = -2.0;
		policy = new DiagonalPolicy(slope);
		break;
	case DIAG_NTHIRD: // 7
		slope = -1.0 / 3;
		policy = new DiagonalPolicy(slope);
		break;
	case DIAG_N3: // 8
		slope = -3.0;
		policy = new DiagonalPolicy(slope);
		break;
	case DIAG_NFOURTH: // 9
		slope = -0.25;
		policy = new DiagonalPolicy(slope);
		break;
	case DIAG_N4: // 10
		slope = -4.0;
		policy = new DiagonalPolicy(slope);
		break;
	default:
		host->error("Unknown policy.");
		break;
	}
	if (!codec) {
		codec = new AdditiveCodec(host->par("blockSize"));
	}
	rateMeasurementInterval = host->par("rateMeasurementInterval");
	segmentDuration = host->par("segmentDuration");
	playback = new VideoPlayback(host->par("numSegments"), segmentDuration, host);
	connId = oldConnId = -1;
	nextJobId = 0;
	state = BEFORE;
	next_job_send_time = MAXTIME;
	lastJobSent = -1;
	currJob = -1;
	currJobProgress = 0;
	currJobSize = -1;
	isRequestPending = false;
	counter = 0;
	simtime_t start_time = host->par("startTime");
	module->scheduleCallback(start_time, BEGIN);

	o_rate.setName("rate");
	o_rateSample.setName("sampledRate");
	o_rtt.setName("rtt");
	o_playbackQuality.setName("quality");
	o_missedBlocks.setName("missedBlocks");
	o_zeroSegments.setName("zeroSegments");
	o_bufferSize.setName("bufferSize");
	o_connectionFailures.setName("connectionFailures");
}

AdditiveController::~AdditiveController() {
	delete policy;
	delete playback;
}

// with new idea, can validate with server idle time, and server-side queue time.
void AdditiveController::sendRequest()
{
	if (oldConnId == -1) { // normal request
		state = DOWNLOADING;
		int segment = policy->selectSegment(codec, &buffer, playback, &networkMonitor);
		int job = nextJobId++;
		int quality = buffer.expectedQualityAt(segment);
		activeJobs[job] = Pair(segment,quality);
		lastJobSent = job;
		int blockSize = codec->getBlockSize(segment, quality);
		network->queueJob(job, connId, blockSize);
		networkMonitor.setLastSendTime(simTime());
	} else { // there was a failed connection.  Queue this for later.
		isRequestPending = true;
	}
}

void AdditiveController::connectionInitialized(int connId) {
	this->connId = connId;
	if (this->oldConnId == -1) { // first time connection initialized.
		playback->getReady();
		module->scheduleCallback(simTime(), SEND_REQUEST);
		module->scheduleCallback(simTime() + segmentDuration, ADVANCE_PLAYBACK);
		module->scheduleCallback(simTime() + rateMeasurementInterval, MEASURE_RATE);
		next_job_send_time = simTime();
	} else if (this->oldConnId != -1 && activeJobs.size() > 0) { // old connection failed while busy.
		network->resumeConnectionJobs(oldConnId, connId);
	}
	if (this->oldConnId != -1){ // old conn failed
		// resume jobs that were to be completed while connection had failed.
		if (isRequestPending) {
			module->scheduleCallback(simTime(), SEND_REQUEST);
		}
		o_connectionFailures.record(2);
	}
	oldConnId = -1;
}
void AdditiveController::connectionFailed(int connId) {
	//cout<<"ConnectionFailed..."<<endl;
	oldConnId = connId;
	network->addConnection();
	o_connectionFailures.record(1);
}
void AdditiveController::jobFinished(int jobId) {
	int segment = activeJobs[jobId].segment;
	int quality = activeJobs[jobId].quality;
	bool valid = codec->receivedBlock(&buffer, segment, quality);
	//bool valid = buffer.receivedBlock(activeJobs[jobId]);
	if (!valid) {
		o_missedBlocks.record(activeJobs[jobId].segment);
	}
	activeJobs.erase(jobId);

	if (!playback->canDownload()) {
		state = DONE;
	} else if (playback->videoBuffering()) {
		module->scheduleCallback(simTime(), ADVANCE_PLAYBACK);
	}
}
// not used.
void AdditiveController::measureNetwork(int jobId)
{
	// measure rate
	networkMonitor.updateRate();
	if (counter++ % 10 == 0) {
		o_rate.record(networkMonitor.getRate());
	}
	// measure rtt
    if (jobId == lastJobSent) {
        networkMonitor.updateRTT();
        o_rtt.record(networkMonitor.getRTT());
    }
}

void AdditiveController::rescheduleNextRequest(int jobId) {
	// did the 'next' request not get sent already?
	if (jobId == lastJobSent) {
		// recalculate next job send time.
		bool current = activeJobs.find(jobId) != activeJobs.end();
		int bytesLeft = (current?network->getJobProgress(jobId)->getRemainingBytes():0);
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
				networkMonitor.receivedBytes(currJobSize - currJobProgress);
			}
			currJobSize= network->getJobProgress(jobId)->getSize();
			currJob = jobId;
			currJobProgress = 0;
		}
		int bytes_recvd = network->getJobProgress(jobId)->getProgress() - currJobProgress;

		networkMonitor.receivedBytes(bytes_recvd);
		currJobProgress = network->getJobProgress(jobId)->getProgress();
		//measureNetwork(jobId);
	    if (jobId == lastJobSent && networkMonitor.rttMeasurementPending()) {
	        networkMonitor.updateRTT();
	        o_rtt.record(networkMonitor.getRTT());
	    }
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
		o_bufferSize.record(buffer.getSize(false,NULL));
		qual = playback->advanceHead(&buffer);
		//cout<<"At t="<<simTime()<<", qual="<<qual<<endl;
		if (!playback->videoBuffering() && !playback->videoComplete()) {
			module->scheduleCallback(simTime() + segmentDuration, ADVANCE_PLAYBACK);
			o_playbackQuality.record(qual);
		} else if (playback->videoBuffering()) {
			o_zeroSegments.record(playback->getHeadPosition()+1);
		}
		break;
	case SEND_REQUEST:
		if (playback->canDownload()) {
			sendRequest();
		}
		break;
	case MEASURE_RATE:
		o_rateSample.record(networkMonitor.updateRate()); // returns sampled rate
		o_rate.record(networkMonitor.getRate()); // returns smoothed rate.
		if (state != DONE && playback->canDownload()) {
			if (!buffer.isFull(codec)) {
				rescheduleNextRequest(currJob);
			}
			module->scheduleCallback(simTime() + rateMeasurementInterval, MEASURE_RATE);
		}
		break;
	default:
		break;
	}
}
