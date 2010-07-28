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
import inet.applications.tcpapp.socketapi.TCPSocketAPIAppInterface;

simple ByteBufferServer like TCPSocketAPIAppInterface
{
    parameters:
        @signal[msgevent](type="unsigned long encapsulated in DynamicResultValue");
        @statistic[messagecounts](title="server message counts"; pattern="server.re*.**"; source="msgevent"; record=dvector);
        
        string mode = default("whole"); // "whole" or "fragments"
        int msgSizeStep = default(1000); // use a negative value for the messages to decrease in size
        								 // (wraps around to the starting msg size if size would be non positive).
        int startingMsgSize = default(1000);
        int msgsInStepInterval = default(1);
        
        
        string socketapi = default("tcpsocketapi");
        // the domain name for this server
        string www = default("server.omnet.net");
        // the port this server is listening on
        int port = default(1000);
        // the httptController to be used by this server
        string controller = default("controller");
        // the HTTP protocol to use: 10 for 1.0 and 11 for 1.1
        int httpProtocol = default(11);
        // the size of a reply message indicating an error
        volatile int errorReplySize @unit(B) = default(200B);
        // the level of logging to be used: 2: Debug, 1: Info; 0: Errors and warnings only
        int logLevel = default(0);
        // the file to log events in
        string logFile = default("");
        // the time it takes for the server to be activated
        double activationTime @unit(s) = default(0s);   
}



