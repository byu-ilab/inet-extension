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

#include "AdaptiveClient.h"
#include "NetworkController.h"
#include "IModule.h"
#include "IApplicationControl.h"
Define_Module(AdaptiveClient);

AdaptiveClient::AdaptiveClient() {
	return;
}
AdaptiveClient::~AdaptiveClient() {
	return;
}

/** Inherited from cSimpleModule **/
void AdaptiveClient::initialize()
{
	SimTime startTime = par("startTime");
	network = this->createNetwork(); //new NetworkController(this);
	application = this->createApplication(); //new ModelPredictiveController(this);
	network->setApplication(application);
	application->setNetwork(network);
	//scheduleAt(startTime, new cMessage("AppCallback", 0)); To be done within implementations.
}

void AdaptiveClient::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage()) {
		map<short, cMessage*>::iterator finder = callbacks.find(msg->getKind());
		ASSERT(finder != callbacks.end());
		delete finder->second;
		callbacks.erase(finder);
		application->handleCallback(msg->getKind());
	} else {
		// don't know what this is for...
		error("Could not determine what message was.");
	}

}
void AdaptiveClient::finish()
{
	Enter_Method_Silent();
	delete application;
	delete network;
	map<short, cMessage*>::iterator it = callbacks.begin();
	for (; it != callbacks.end(); it++) {
		cancelAndDelete(it->second);
	}
	callbacks.clear();
}


/** Inherited from IModule **/
void AdaptiveClient::scheduleCallback(simtime_t time, short type) {
	Enter_Method_Silent();
	map<short, cMessage*>::iterator finder = callbacks.find(type);
	ASSERT(finder == callbacks.end());
	cMessage * cb = new cMessage("AppCallback", type);
	callbacks[type] = cb;
	scheduleAt(time, cb);
}
void AdaptiveClient::cancelCallback(short type) {
	Enter_Method_Silent();
	map<short, cMessage*>::iterator finder = callbacks.find(type);
	if (finder != callbacks.end()) {
		cancelAndDelete(finder->second);
		callbacks.erase(finder);
	}
}
simtime_t AdaptiveClient::getSimTime() {
	return simTime();
}

INetworkControl * AdaptiveClient::createNetwork() {
	error("Must override this function in descendant class.");
	return NULL;
}
IApplicationControl * AdaptiveClient::createApplication() {
	error("Must override this function in descendant class.");
	return NULL;
}
