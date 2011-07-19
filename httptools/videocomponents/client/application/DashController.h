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
#include "INetworkControl.h"
#include "IModule.h"
class DashState;

struct Playback {
	enum {PENDING, PAUSED, PLAYING} Status;
	Playback():Status(PENDING),currSegment(-1),currSegmentStart(0) {}
	int currSegment;
	SimTime currSegmentStart;
};
struct Download {
	Download() {segment = 0;reset();}
	void reset() {DLStatus = PENDING; lastRecvTime = 0;}
	enum {PENDING,PAUSED,DOWNLOADING,COMPLETE} DLStatus;
	int segment;
	SimTime lastRecvTime; // time of last received packet.
};

class DashController: public IApplicationControl {
private:
	IModule * module;
	int numSockets; // Todo: change implementation to support multiple sockets.
	int bytesPerQuality;
	int k;
	int numSegments;
	int bufferSize;
	int nextSegId;
	int pausedSegmentConn; // -1 means nothing is paused.
	int currentConn; // connection currently used.
	Playback playback;
	Download currentDownload;
	double rate; // estimated weight
	double alpha; // ewma weight for current measurement. (0 <= alpha <= 1)

public:
	DashController(IModule * module, cSimpleModule * mainModule);
	virtual ~DashController();

	/**
	 * Helper functions
	 */
	void setConnection(int connId); // set current connection.
	bool isBufferFull(); // is the wait buffer full
	void pauseSegmentDownload(int connId); // signals a paused download
	void segmentComplete(int jobId); // signals completion of a segment
	bool isVideoDownloaded(); // well, is it?
	void closeConnections(); // close all connections (assume that they are not working).
	void waitUntilBufferEmpty(); // schedules a callback when playback reaches new segment.
	void requestNextSegment(); // either resumes segment, or picks a new segment to download.
	void measureSpeed(int jobId); // measures job speed.

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
	virtual void handleCallback(short type);
private:
	friend class DashState;
	DashState * _state;
public:

	void ChangeState(DashState*);
};

#endif /* DASHCONTROLLER_H_ */
