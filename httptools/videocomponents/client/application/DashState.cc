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

#include "DashState.h"
#include "DashController.h"

/* Default actions */
DashState::DashState() {}
void DashState::connectionInitialized(DashController *, int connId) {ASSERT(false);}
void DashState::connectionFailed(DashController *, int connId) {ASSERT(false);}
void DashState::jobFinished(DashController *, int jobId) {ASSERT(false);}
void DashState::jobProgress(DashController *, int jobId) {ASSERT(false);}
void DashState::handleCallback(DashController *) {ASSERT(false);}

void DashState::ChangeState(DashController * controller, DashState * state) {
	controller->ChangeState(state);
}

/* Dash Idle Actions */
DashIdle::DashIdle() {}
DashState * DashIdle::_instance = 0;
DashState * DashIdle::Instance() {
	if (_instance == 0) {
		_instance = new DashIdle();
	}
	return _instance;
}
void DashIdle::handleCallback(DashController *app) {
	// handleCallback during DashIdle means start downloading a video
	app->getNetwork()->addConnection();
	app->ChangeState(DashConnect::Instance());
}

/* DashConnect Actions */
DashConnect::DashConnect(){}
DashState * DashConnect::_instance = 0;
DashState * DashConnect::Instance() {
	if (_instance == 0) {
		_instance = new DashConnect();
	}
	return _instance;
}
void DashConnect::connectionInitialized(DashController *app, int connId) {
	app->setConnection(connId);
	app->requestNextSegment();
	if (app->isBufferFull()) {
		app->ChangeState(DashBufferFull::Instance());
	} else {
		app->ChangeState(DashRequest::Instance());
	}
}

void DashConnect::connectionFailed(DashController *app, int connId) {
	// assume: was trying to connect, but app failed!
	app->getNetwork()->addConnection();
}

/* DashRequest Actions */
DashRequest::DashRequest(){}
DashState * DashRequest::_instance = 0;
DashState * DashRequest::Instance() {
	if (_instance == 0) {
		_instance = new DashRequest();
	}
	return _instance;
}
void DashRequest::connectionFailed(DashController *app, int connId) {
	app->pauseSegmentDownload(connId);
	app->getNetwork()->addConnection();
	app->ChangeState(DashConnect::Instance());
}
void DashRequest::jobFinished(DashController *app, int jobId) {
	app->segmentComplete(jobId);
	if (app->isVideoDownloaded()) {
		app->closeConnections();
		app->ChangeState(DashIdle::Instance());
	} else if (app->isBufferFull()) {
		app->waitUntilBufferEmpty();
		app->ChangeState(DashBufferFull::Instance());
	} else {
		app->requestNextSegment();
	}
}
void DashRequest::jobProgress(DashController *app, int jobId) {
	app->measureSpeed(jobId);
}

/* DashBufferFull Actions */
DashBufferFull::DashBufferFull(){}
DashState * DashBufferFull::_instance = 0;
DashState * DashBufferFull::Instance() {
	if (_instance == 0) {
		_instance = new DashBufferFull();
	}
	return _instance;
}
void DashBufferFull::connectionFailed(DashController *app, int connId) {
	app->getNetwork()->addConnection();
	app->ChangeState(DashConnect::Instance());
}
void DashBufferFull::handleCallback(DashController *app) {
	app->requestNextSegment();
	app->ChangeState(DashRequest::Instance());
}
