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

package inet.httptools.base;

// It is anticipated that a module implementing this interface,
// and therefore the httptServerBase, will also extend some sort
// of TCP application interface so as to plug into a StandardHost's
// tcp applications array.

// defines the parameters which the httptServerBase requires
moduleinterface httptServerBaseInterface
{
    parameters:
        // the domain name for this server
        string www;
        // the port this server is listening on
        int port;
        // the httptController to be used by this server
        string controller;
        // the HTTP protocol to use: 10 for 1.0 and 11 for 1.1
        int httpProtocol;
        // the size of a reply message indicating an error
        volatile int errorReplySize;
        // the level of logging to be used: 2: Debug, 1: Info; 0: Errors and warnings only
        int logLevel;
        // the file to log events in
        string logFile;
        // the time it takes for the server to be activated
        double activationTime @unit(s);
} 


