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

package inet.httptools.videocomponents.client.modules;

//
// TODO auto-generated module
//
simple DASHClient like IAdaptiveClient
{
    parameters:
        string socketapi;
        string controller = default("controller");
        string serverwww = default("server.omnet.net");        
        string address = default("");
        int port = default(2000);
        int httpProtocol = default(11);
 		int logLevel = default(2);
 		string logFile = default("");
 		
 		int numSockets = default(1);
 		int k = default(2); // seconds per chunk
 		int numQualities = default(10);
 		int bytesPerQuality = default(10000); // assumed linear increase in bytes per quality.
}