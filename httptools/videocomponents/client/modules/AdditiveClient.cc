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

#include "AdditiveClient.h"
#include "NetworkController.h"
#include "AdditiveController.h"

Define_Module(AdditiveClient);

void AdditiveClient::initialize()
{
    AdaptiveClient::initialize();
}

void AdditiveClient::handleMessage(cMessage *msg)
{
    AdaptiveClient::handleMessage(msg);
}
INetworkControl * AdditiveClient::createNetwork() {
	return new NetworkController(this);
}
IApplicationControl * AdditiveClient::createApplication() {
	return new AdditiveController(this,this);
}
