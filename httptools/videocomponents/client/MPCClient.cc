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

#include "MPCClient.h"
#include "ModelPredictiveController.h"
#include "NetworkController.h"
Define_Module(MPCClient);

void MPCClient::initialize()
{
    AdaptiveClient::initialize();
}

void MPCClient::handleMessage(cMessage *msg)
{
    AdaptiveClient::handleMessage(msg);
}
INetworkControl * MPCClient::createNetwork() {
	return new NetworkController(this);
}
IApplicationControl * MPCClient::createApplication() {
	return new ModelPredictiveController(this);
}
