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

#ifndef INETWORKCONTROL_H_
#define INETWORKCONTROL_H_
#include "Job.h"
class IApplicationControl;
class INetworkControl {
protected:
	IApplicationControl * application;
public:
	INetworkControl();
	virtual ~INetworkControl();
	void setApplication(IApplicationControl * app) {this->application = app;}

	virtual int addConnection() = 0;

	virtual void cleanupConnection(int connId) = 0;

	virtual void resumeConnectionJobs(int oldConnId, int newConnId) = 0;

	virtual void queueJob(int jobId, int connId, int jobSize) = 0;

	virtual Job * getJobProgress(int jobId) = 0;
};

#endif /* INETWORKCONTROL_H_ */
