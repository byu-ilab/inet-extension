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

#ifndef ADDITIVECONTROLLER_H_
#define ADDITIVECONTROLLER_H_
#include <cmath>
#include <omnetpp.h>
#include "IPolicy.h"
#include "VideoPlayback.h"
#include "ActiveRegion.h"
#include "NetworkMonitor.h"
#include <map>
#include "IApplicationControl.h"
enum CallbackType{BEGIN=0,ADVANCE_PLAYBACK,SEND_REQUEST};
enum AdditiveState{BEFORE, CONNECTING, DOWNLOADING, DONE};
class AdditiveController: public IApplicationControl {
private:
	IModule * module;
	cSimpleModule * host;
	IPolicy * policy;
	VideoPlayback * playback;
	ActiveRegion buffer;
	map<int, int> activeJobs; // job to segment.
	int blockSize;
	double segmentDuration;
	int connId, oldConnId;
	int nextJobId;
	AdditiveState state;

	// rate measurement
	NetworkMonitor networkMonitor;
	int currJob;
	int currJobProgress;
	int lastJobSent;
	simtime_t next_job_send_time; // estimated time to schedule next request.
	bool isRequestPending;

    void sendRequest();
    void measureNetwork(int jobId);
    void rescheduleNextRequest(int jobId);
public:
	AdditiveController(IModule * module, cSimpleModule * mainModule);
	virtual ~AdditiveController();
	virtual void connectionInitialized(int connId);
	virtual void connectionFailed(int connId);
	virtual void jobFinished(int jobId);
	virtual void jobProgress(int jobId);
	virtual void handleCallback(short type);
};

#endif /* ADDITIVECONTROLLER_H_ */
