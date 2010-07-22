// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
import inet.applications.tcpapp.socketapi.TCPSocketAPIAppInterface;
import inet.httptools.base.httptServerBaseInterface;

package inet.httptools.videocomponents;

//
// VideoServer
//
// A video server that streams content like a MOVE videoserver. This module
// is intended for use with the INET StandardHostWithTCPSocketAPI module and
// plugs in as a tcpApp. The INET TCP/IP stack modeling is used for transport.
// See the INET documentation for more details.
//
// @see httptServerBase
//
// @version 1.0 
// @author  Travis Andelin
//
simple VideoServer like TCPSocketAPIAppInterface, httptServerBaseInterface
{
    parameters:
        @signal[msgevent](type="cMessageEventDatagram objects");
        bool shouldTrackDuplicateMessageNames = default(false);
        
 		// specific to VideoServer
        //string siteDefinition = default("");			// The site script file. Blank to disable.
        //xml config;										// The XML configuration file for random sites
        string workloadGenerator = default("workloadGenerator");
        
         // From TCPSocketAPIApp
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
                
}

