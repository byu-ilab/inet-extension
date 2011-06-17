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
#include "ModelPredictiveController.h"
#include "NetworkController.h"
#include "IModule.h"

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
	network = this->createNetwork(); //new NetworkController(this);
	application = this->createApplication(); //new ModelPredictiveController(this);
	network->setApplication(application);
	application->setNetwork(network);
	appCallback = new cMessage("AppCallback");
	scheduleAt(simTime()+1, appCallback);
}

void AdaptiveClient::handleMessage(cMessage *msg)
{
	if (msg == appCallback) {
		application->handleCallback();
	} else {
		// don't know what this is for...
		error("Could not determine what message was.");
	}

}
void AdaptiveClient::finish()
{
	delete application;
	delete network;
	cancelAndDelete(appCallback);
}


/** Inherited from IModule **/
void AdaptiveClient::scheduleCallback(SimTime time) {
	scheduleAt(time, appCallback);
}
void AdaptiveClient::cancelCallback() {
	cancelEvent(appCallback);
}
INetworkControl * AdaptiveClient::createNetwork() {
	error("Must override this function in descendant class.");
	return NULL;
}
IApplicationControl * AdaptiveClient::createApplication() {
	error("Must override this function in descendant class.");
	return NULL;
}
