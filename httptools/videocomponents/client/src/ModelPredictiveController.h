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

#ifndef MODELPREDICTIVECONTROLLER_H_
#define MODELPREDICTIVECONTROLLER_H_
#include <iostream>
#include "IApplicationControl.h"
class IModule;
class IApplicationControl;
using namespace std;
class ModelPredictiveController: public IApplicationControl {
private:
	int jobIdCtr;
	int newConnId, brokeConnId;
public:
	ModelPredictiveController(IModule * module);
	virtual ~ModelPredictiveController();

	/** IApplicationControl Functions **/
	/**
	 * tells app that connection has been registered with tcp.
	 */
	virtual void connectionInitialized(int connId); // signal to begin downloading segment.

	/**
	 * tells app that connection has timed out or something similar.
	 */
	virtual void connectionFailed(int connId);

	/**
	 * tells app that job jobId finished
	 * network not responsible for job info after jobFinished() call.
	 */
	virtual void jobFinished(int jobId);

	/**
	 * tells app that progress bytes were received.
	 * Perhaps to change for consistency with net::getJobProgress()
	 */
	virtual void jobProgress(int jobId); // notification

	/**
	 * handles callback from application.
	 */
	virtual void handleCallback();
};

#endif /* MODELPREDICTIVECONTROLLER_H_ */
