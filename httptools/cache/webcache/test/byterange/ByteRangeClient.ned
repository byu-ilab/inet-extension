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

package inet.httptools.cache.webcache.test.byterange;

import inet.applications.tcpapp.socketapi.ITCPSocketMgrApp;

simple ByteRangeClient like ITCPSocketMgrApp
{
    parameters:
        string socketapi;
        string controller = default("controller");
        string serverwww = default("server.omnet.net");
        //int serverport = default(1000);
        int numRequests = default(1);
        int numRanges = default(3);
        int requestFileSize @unit(KB) = default(1KB);
        
        string address = default("");
        int port = default(2000);
        int httpProtocol = default(11);
 		int logLevel = default(2);
 		string logFile = default("");
 		
}
