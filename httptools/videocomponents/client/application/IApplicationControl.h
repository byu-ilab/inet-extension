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

#ifndef IAPPLICATIONCONTROL_H_
#define IAPPLICATIONCONTROL_H_

class IModule;
class INetworkControl;
class IApplicationControl {
protected:
	IModule * module;
	INetworkControl * network; // module & network may be same
public:
	IApplicationControl();
	virtual ~IApplicationControl();

	void setNetwork(INetworkControl * network) {this->network = network;}

	virtual void connectionInitialized(int connId) = 0;

	virtual void connectionFailed(int connId) = 0;

	virtual void jobFinished(int jobId) = 0;

	virtual void jobProgress(int jobId) = 0;

	virtual void handleCallback() = 0;
};

#endif /* IAPPLICATIONCONTROL_H_ */
