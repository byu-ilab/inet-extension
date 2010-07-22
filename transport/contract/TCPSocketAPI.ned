// Author: Kevin Black
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

package inet.transport.contract;

import inet.applications.tcpapp.TCPApp;

simple TCPSocketAPI
{
    parameters:
        @display("i=block/layer");
        
        // if shouldMapTCPConnections is true
        @signal[tcpconninfo](type="TCPConnInfoDatagram objects");
        
        // if shouldTrackDuplicateMessageNames is true
        @signal[msgevent](type="cMessageEventDatagram objects");
        
        bool shouldMapTCPConnections = default(false);
        bool shouldTrackDuplicateMessageNames = default(false);
        
    gates:
        input tcpIn;
        output tcpOut;
}
