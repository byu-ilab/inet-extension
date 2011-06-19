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

#include "ModelPredictiveController.h"
#include "IModule.h"
#include "INetworkControl.h"
#include <iostream>
ModelPredictiveController::ModelPredictiveController(IModule * module) {
	this->module = module;
	jobIdCtr = 0;
	newConnId = brokeConnId = -1;
}

ModelPredictiveController::~ModelPredictiveController() {

}

/** IApplicationControl Functions **/

/**
 * handles callback from application.
 */
void ModelPredictiveController::handleCallback() {
	cout<<"At Callback"<<endl;
	cout<<"Starting a socket"<<endl;
	network->addConnection();
}


/**
 * tells app that connection has been registered with tcp.
 * // signal to begin downloading segment.
 */
void ModelPredictiveController::connectionInitialized(int connId) {
	cout<<"ConnectionInitialized: "<<connId<<endl;
	if (newConnId == -1)
		network->queueJob(++jobIdCtr,connId,8000);
	else {
		network->resumeConnectionJobs(brokeConnId,newConnId);
	}
}

/**
 * tells app that connection has timed out or something similar.
 */
void ModelPredictiveController::connectionFailed(int connId) {
	cout<<"ConnectionFailed: "<<connId<<endl;
	newConnId = network->addConnection();
	brokeConnId = connId;
}

/**
 * tells app that job jobId finished
 */
void ModelPredictiveController::jobFinished(int jobId) {
	cout<<"Job Finished: "<<jobId<<endl;
}

/**
 * tells app that progress bytes were received.
 * // notification
 * Perhaps to change for consistency with net::getJobProgress()
 */
void ModelPredictiveController::jobProgress(int jobId) {
	int progress = network->getJobProgress(jobId)->getProgress();
	cout<<"JobProgress: "<<jobId<<", Progress: "<<progress<<endl;
}


