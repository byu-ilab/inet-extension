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

package inet.applications.tcpapp.socketapi;

import inet.applications.tcpapp.TCPApp;

moduleinterface TCPSocketAPIApp
{
    parameters:
        string socketapi;
}

// code snippet to extract pointer to TCPSocketAPI
//    std::string api_obj_name = par("socketapi").stringValue();
//    if (api_obj_name.empty())
//    	opp_error("TCPSAPIGenericSrvApp::initialize(): no tcp socket api specified!");
//    socketapi = check_and_cast<TCPSocketAPI *>(getParentModule()->getSubmodule(api_obj_name.c_str()));