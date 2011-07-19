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

#ifndef __INET_ADAPTIVECLIENT_H_
#define __INET_ADAPTIVECLIENT_H_

#include <omnetpp.h>

#include "IModule.h"
#include "INetworkControl.h"



/**
 * Customizable Adaptive Client.
 * The adaptive client is a network module which interfaces with the application
 * via the INetworkControl and IModule, interfaces.  Ideally, the module would
 * not have been an INetworkControl descendant, but is so due to the tight coupling
 * of the networking with cModules.
 */
class AdaptiveClient : public cSimpleModule,
		public IModule
{
private:
	// App variables.
	IApplicationControl * application;
	INetworkControl * network;

	map<short,cMessage*> callbacks;

public:
	AdaptiveClient();
	~AdaptiveClient();
protected:
	/** Inherited from cSimpleModule **/
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();


    /** Inherited from IModule **/
	virtual void scheduleCallback(simtime_t time, short type);
	virtual void cancelCallback(short type);
	virtual simtime_t getSimTime();

	/** Makes it possible to build different client types **/
	virtual INetworkControl * createNetwork();
	virtual IApplicationControl * createApplication();
};

#endif
