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
DashController::DashController(IModule * module, cSimpleModule * mainModule) {
	this->module = module;
	this->numSockets = mainModule->par("numSockets");
	this->bytesPerQuality = mainModule->par("bytesPerQuality");
	this->k = mainModule->par("k");

}

DashController::~DashController() {
}
/** IApplicationControl Functions **/

/**
 * handles callback from application.
 */
void DashController::handleCallback() {
}


/**
 * tells app that connection has been registered with tcp.
 * // signal to begin downloading segment.
 */
void DashController::connectionInitialized(int connId) {
}

/**
 * tells app that connection has timed out or something similar.
 */
void DashController::connectionFailed(int connId) {
}

/**
 * tells app that job jobId finished
 */
void DashController::jobFinished(int jobId) {

}

/**
 * tells app that progress bytes were received.
 * // notification
 * Perhaps to change for consistency with net::getJobProgress()
 */
void DashController::jobProgress(int jobId) {
}

