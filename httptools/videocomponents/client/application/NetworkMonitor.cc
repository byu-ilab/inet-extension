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

#include "NetworkMonitor.h"
NetworkMonitor::NetworkMonitor(double rttAlpha, double rateAlpha, double meanAlpha, bool realMean) {
	rtt = .1;
	rate = 0; // bits per second!!
	rtt_alpha = rttAlpha; // sampled rarely, so we don't care abt. history.
	rate_alpha = rateAlpha; // sampled often, so we care about history
	mean_alpha = meanAlpha;
	this->realMean = realMean;
	last_recv_time = 0.0;
	last_recv_bytes = 0;
	send_time = 0.0;
	totalBytes = 0;
	timeElapsed = 0;
}

NetworkMonitor::~NetworkMonitor() {
	// TODO Auto-generated destructor stub
}
void NetworkMonitor::setLastSendTime(simtime_t lastSendTime) {
	send_time = lastSendTime;
}
bool NetworkMonitor::rttMeasurementPending() {
	return send_time > 0.0;
}
double NetworkMonitor::updateRate() {
	double dT = (simTime() - last_recv_time).dbl();
	double currRate = last_recv_bytes * 8.0 / dT;
	if (dT > 0.0) {
		totalBytes += last_recv_bytes;
		timeElapsed += dT;
		//cout<<"TotalBytes: "<<totalBytes<<", timeElapsed: "<<timeElapsed<<endl;
	    rate = rate_alpha * currRate + (1 - rate_alpha) * rate;
	    meanRate = mean_alpha * currRate + (1 - mean_alpha) * meanRate;
	    last_recv_time = simTime();
	    last_recv_bytes = 0;
	}
	return currRate;

}
void NetworkMonitor::updateRTT() {
	ASSERT(rttMeasurementPending());
	double currRtt = (simTime() - send_time).dbl();
	//cout<<"currRtt: "<<currRtt<<endl;
	rtt = rtt_alpha * currRtt + (1 - rtt_alpha) * rtt;
	send_time = 0.0;
}
double NetworkMonitor::getMeanRate(){
	if (timeElapsed == 0) return 0;
	return 8.0 * totalBytes / timeElapsed;
}
double NetworkMonitor::getNextRequestTime(int bytesLeft) {
	double secondsLeft = 8.0 * bytesLeft / rate;
	double targetSend = secondsLeft - rtt;
	return targetSend;
}
void NetworkMonitor::receivedBytes(int numBytes) {
	last_recv_bytes += numBytes;
}

