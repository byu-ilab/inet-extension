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
#include "Codec.h"
#include <map>
#include "IApplicationControl.h"
enum CallbackType{BEGIN=0,ADVANCE_PLAYBACK,SEND_REQUEST,MEASURE_RATE};
enum AdditiveState{BEFORE, CONNECTING, DOWNLOADING, DONE};
struct Pair{
	int segment;
	int quality;
	Pair(int s,int q):segment(s),quality(q) {}
	Pair(const Pair & p):segment(p.segment),quality(p.quality) {}
	Pair():segment(0),quality(0) {}
};
typedef map<int,Pair> JobMap;
class AdditiveController: public IApplicationControl {
private:
	IModule * module;
	cSimpleModule * host;
	IPolicy * policy;
	VideoPlayback * playback;
	ActiveRegion buffer;
	Codec * codec;
	JobMap activeJobs; // job to segment/quality.
	double segmentDuration;
	int connId, oldConnId;
	int nextJobId;
	AdditiveState state;

	// rate measurement
	NetworkMonitor networkMonitor;
	double rateMeasurementInterval;
	int currJob;
	int currJobProgress;
	int currJobSize;
	int lastJobSent;
	simtime_t next_job_send_time; // estimated time to schedule next request.
	bool isRequestPending;
	unsigned counter;

	// output
	cOutVector o_rate;
	cOutVector o_rtt;
	cOutVector o_playbackQuality;
	cOutVector o_missedBlocks;
	cOutVector o_zeroSegments;
	cOutVector o_bufferSize;
	cOutVector o_connectionFailures; // 1 when down, 2 when back up.

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
