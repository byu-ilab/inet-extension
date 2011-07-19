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

#include "DashController.h"
#include "DashState.h"
#include "INetworkControl.h"
DashController::DashController(IModule * module, cSimpleModule * mainModule) {
	this->module = module;
	this->numSockets = mainModule->par("numSockets");
	this->bytesPerQuality = mainModule->par("bytesPerQuality");
	this->k = mainModule->par("k");
	this->numSegments = mainModule->par("numSegments");
	this->bufferSize = mainModule->par("bufferSize");
	this->_state = DashIdle::Instance();
	this->rate =0;
	alpha = 0.5;

}

DashController::~DashController() {
}

void DashController::ChangeState(DashState * state) {
	this->_state = state;
}

void DashController::setConnection(int connId) {
	this->currentConn = connId;
}

// is the wait buffer full
bool DashController::isBufferFull() {
	return currentDownload.segment - 1  - playback.currSegment == bufferSize;
}

// signals a paused download
void DashController::pauseSegmentDownload(int connId) {
	ASSERT(connId == currentDownload.segment);
	currentDownload.DLStatus = Download::PAUSED;
}

// signals completion of a segment
void DashController::segmentComplete(int jobId) {
	ASSERT(currentDownload.segment == jobId);
	currentDownload.DLStatus = Download::COMPLETE;
}

bool DashController::isVideoDownloaded() {
	return currentDownload.DLStatus == Download::COMPLETE && currentDownload.segment == numSegments;
}

// close all connections (assume that they are not working).
void DashController::closeConnections() {
	network->closeConnection(currentConn);
	network->cleanupConnection(currentConn);
}

// schedules a callback when playback reaches new segment.
void DashController::waitUntilBufferEmpty() {
	ASSERT(isBufferFull());
	ASSERT(playback.Status == Playback::PLAYING);
	SimTime wake = playback.currSegmentStart + k;
	ASSERT(wake >= simTime());
	module->scheduleCallback(wake, 0);
}

// either resumes segment, or picks a new segment to download.
void DashController::requestNextSegment() {
	if (currentDownload.DLStatus == Download::DOWNLOADING) {
		//currentDownloa
	} else if (currentDownload.DLStatus == Download::PAUSED){
		network->resumeConnectionJobs(pausedSegmentConn, currentConn);
		currentDownload.DLStatus = Download::DOWNLOADING;
	} else {
		ASSERT(false);
	}
}

void DashController::measureSpeed(int jobId) {

}

/** IApplicationControl Functions **/

/**
 * handles callback from application.
 * Assumed that the first callback is a start message.
 * Future callbacks are client-induced.
 */
void DashController::handleCallback(short type) {
	_state->handleCallback(this);
}


/**
 * tells app that connection has been registered with tcp.
 * // signal to begin downloading segment.
 */
void DashController::connectionInitialized(int connId) {
	_state->connectionInitialized(this, connId);
}

/**
 * tells app that connection has timed out or something similar.
 */
void DashController::connectionFailed(int connId) {
	_state->connectionFailed(this, connId);
}

/**
 * tells app that job jobId finished
 */
void DashController::jobFinished(int jobId) {
	_state->jobFinished(this, jobId);
}

/**
 * tells app that progress bytes were received.
 * // notification
 * Perhaps to change for consistency with net::getJobProgress()
 */
void DashController::jobProgress(int jobId) {
	_state->jobProgress(this, jobId);
}
