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

#ifndef VIDEOSOCKET_H_
#define VIDEOSOCKET_H_

#include <deque> // push: push_back(), pop: pop_front()
#include <InternetTypeDefsWithStandardTypes.h>
#include "Job.h"

typedef deque<Job*> JobQueue;
class Worker {
private:
	socket_id_t socket_id;
	JobQueue pendingResponses;
public:
	static bytecount_t RECV_MODE;
	Worker(socket_id_t socket_id);
	virtual ~Worker();
	void setSocketId(socket_id_t socketId);
	void enqueueJob(Job * job);
	Job * dequeueJob();
	/**
	 * Returns job on front of the queue, or null if none.
	 */
	Job * getHeadJob();
	JobQueue & getJobQueue();
	int getNumPendingResponses();
	void socketFailure(); // socket fails. Set each Job's status to paused.
};

#endif /* VIDEOSOCKET_H_ */
