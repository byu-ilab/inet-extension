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

#include "Worker.h"
#include "Job.h"
Worker::Worker(socket_id_t socket_id): socket_id(socket_id) {

}

Worker::~Worker() {
	// TODO Auto-generated destructor stub
}
void Worker::setSocketId(socket_id_t socketId) {
	this->socket_id = socketId;
}
void Worker::enqueueJob(Job * job) {
	pendingResponses.push_back(job);
}
Job * Worker::dequeueJob() {
	if (pendingResponses.size() == 0) {
		return NULL;
	}
	Job * front = pendingResponses.front();
	pendingResponses.pop_front();
	return front;
}
Job * Worker::getHeadJob() {
	if (pendingResponses.size() == 0) {
		return NULL;
	}
	return pendingResponses[0];
}
JobQueue & Worker::getJobQueue() {
	return pendingResponses;
}
int Worker::getNumPendingResponses() {
	return pendingResponses.size();
}
// socket fails. Set each Job's status to paused.
void Worker::socketFailure() {
	for (unsigned int i=0; i < pendingResponses.size(); i++) {
		Job * job = pendingResponses[i];
		job->setState(PAUSED);
	}
}
