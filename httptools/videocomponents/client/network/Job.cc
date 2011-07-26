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

#include "Job.h"

Job::Job(int jobId, int size):
	id(jobId), size(size),
	bytesReceived(0),state(QUEUEING),
	lastReceiveAmount(-1){
	return;
}
Job::~Job() {}
/**
 * Notify job that numBytes (of application content) have been received.
 * @pre must be expecting response.
 * @throws false assertion if receives more than expected response size.
 */
void Job::receivedBytes(int numBytes) {
	bytesReceived += numBytes;
	ASSERT(bytesReceived <= size);
	if (bytesReceived == size) {
		state = COMPLETE;
	}
	lastReceiveAmount = numBytes;
}
/**
 * Bytes left
 * // bytes left to read in total response (header + content).
 * @pre must be expecting response
 */
int Job::getRemainingBytes() {
	return size - bytesReceived;
}
const int Job::getProgress() {
	return bytesReceived;
}
int Job::getSize() {
	return size;
}
int Job::getId() {
	return id;
}
void Job::expectResponse(int contentLength) {
	ASSERT(contentLength == getRemainingBytes());
	setState(RECEIVING);
}

/**
 * State of the Job
 */
State Job::getState() {
	return state;
}
void Job::setState(State state) {
	this->state = state;
}

int Job::getLastBytesRecvd() {
	ASSERT(lastReceiveAmount != -1);
	return lastReceiveAmount;
}

