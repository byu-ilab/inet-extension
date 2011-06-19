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

#ifndef DASHCONTROLLER_H_
#define DASHCONTROLLER_H_

#include <omnetpp.h>
#include "IApplicationControl.h"

class DashController: public IApplicationControl {
private:
	IModule * module;
	int numSockets;
	int bytesPerQuality;
	int k;
public:
	DashController(IModule * module, cSimpleModule * mainModule);
	virtual ~DashController();

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

#endif /* DASHCONTROLLER_H_ */
