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

#ifndef SEGMENTSTATUS_H_
#define SEGMENTSTATUS_H_

#include <map>
#include <omnetpp.h>
#include <algorithm>
using namespace std;
/**
 * State descriptions:
 * QUEUEING: Request has been sent, and waiting for first message of reply.
 * RECEIVING: Request has been sent, and first message of reply received, but not complete.
 * COMPLETE: All of reply has been received.
 * PAUSED: Socket timed out, and hence we start by re-requesting what is left to get.
 */
enum State {QUEUEING,RECEIVING,PAUSED,COMPLETE};
class Job {
private:
	int id; // job id
	int size; // size of job (application bytes)
	int bytesReceived;
	State state;
public:
	Job(int jobId, int size);
	virtual ~Job();

	/**
	 * Notify job that numBytes have been received.
	 * @pre must be expecting response.
	 * @throws false assertion if receives more than expected response size.
	 */
	void receivedBytes(int numBytes);

	/**
	 * Bytes left
	 * @pre must be expecting response
	 */
	int getRemainingBytes(); // bytes left to read in total response (header + content).
	int getId();
	void expectResponse(int contentLength);
	const int getProgress();
	int getSize();
	/**
	 * State of the Job
	 */
	State getState();
	void setState(State state);
};

#endif /* SEGMENTSTATUS_H_ */
