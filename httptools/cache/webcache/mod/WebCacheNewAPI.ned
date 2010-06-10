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
import inet.applications.tcpapp.socketapi.TCPSocketAPIAppInterface;
import inet.httptools.base.httptServerBaseInterface;

package inet.httptools.cache.webcache.mod;

simple WebCacheNewAPI like TCPSocketAPIAppInterface, httptServerBaseInterface
{
    parameters:
        // From TCPSocketAPIApp
        	// the socketAPI name
        string socketapi;								// the socketAPI name.
        
        // From httptServerBaseInterface
        	// domain name of the server
        string www = default("");						// The WWW name of the server
        	// the port the server listens on
        int port = default(80);							// The listening port number
        	// the httptController with which this server should register
        string controller = default("controller");
        	// the protocl to be used
        int httpProtocol=default(11);					// The http protocol: 10 for http/1.0, 11 for http/1.1. Not used at the present time.
        	// the size of a reply message indicating an error
        volatile int errorReplySize @unit(B) = default(200B);
        	// the level of logging to be used
        int logLevel = default(0);						// The log level: 2: Debug, 1: Info; 0: Errors and warnings only
        	// the file to write the log to
        string logFile = default("");					// Name of server log file. Events are appended, allowing sharing of file for multiple servers.
        	// the time it takes to activate
        double activationTime @unit(s) = default(0s);	// The initial activation delay. Zero to disable.
        
        // attributes specific to the WebCacheNewAPI
        	// cache capacity
        	// TODO make it TB, need to change LRU to use long
        int cacheSize @unit(B)= default(10KB);  // capacity of cache in terabytes
        	// domain name of upstream server/cache
        string serverwww = default(""); // domain name of upstream server/cache.
        	// time an upstream connection should wait without any activity
        int request_timeout = default(10); // time for an upstream connection to wait.
}
