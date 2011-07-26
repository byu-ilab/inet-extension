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

#ifndef NETWORKMONITOR_H_
#define NETWORKMONITOR_H_
#include <omnetpp.h>
#include <iostream>
using namespace std;
class NetworkMonitor {
private:
	double rtt; // seconds
	double rate; // bps
	int totalBytes; //
	double timeElapsed;
	double rtt_alpha; // params for ewmas.
	double rate_alpha;
	simtime_t last_recv_time; // time of last received packet.
	int last_recv_bytes; // bytes received since last rate update.
	simtime_t send_time; // time of sending packet measuring rtt.
public:
	NetworkMonitor();
	virtual ~NetworkMonitor();
	void setLastSendTime(simtime_t lastSendTime);
	bool rttMeasurementPending();
	void updateRate();
	void updateRTT();
	double getNextRequestTime(int remainingBytes);
	void receivedBytes(int numBytes);
	double getRate() {return rate;}
	double getMeanRate() {if (timeElapsed == 0) return 0;return 8.0 * totalBytes / timeElapsed;}
	double getRTT() {return rtt;}
};

#endif /* NETWORKMONITOR_H_ */
