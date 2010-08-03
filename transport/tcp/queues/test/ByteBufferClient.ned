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

package inet.transport.tcp.queues.test;

import inet.applications.tcpapp.socketapi.ITCPSocketMgrApp;

simple ByteBufferClient like ITCPSocketMgrApp
{
    parameters:
        @signal[msgevent](type="unsigned int");
        @statistic[messagecounts](title="client message counts"; pattern="client*.re*.**"; source="msgevent"; record=dvector);
        
        string socketapi = default("tcpsocketapi");
        string controller = default("controller");
        string serverwww;
        string mode = default("whole"); // should be "whole" or "fragments"
        int numRequestsToSend = default(2);
        int requestPhaseDensity = default(1); // the number of requests to be sent every time the client is making requests to the server
}
