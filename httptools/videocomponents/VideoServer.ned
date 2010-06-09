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

package inet.httptools.videocomponents;

import inet.applications.tcpapp.socketapi.TCPSocketAPIAppInterface;

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
simple VideoServer like TCPSocketAPIAppInterface
{
    parameters:
        string socketapi;									// the socketAPI name.
        string www = default("");						// The WWW name of the server
        int port = default(80);							// The listening port number
        int httpProtocol=default(11);					// The http protocol: 10 for http/1.0, 11 for http/1.1. Not used at the present time.
        int logLevel = default(0);						// The log level: 2: Debug, 1: Info; 0: Errors and warnings only
        string logFile = default("");					// Name of server log file. Events are appended, allowing sharing of file for multiple servers.
        string siteDefinition = default("");			// The site script file. Blank to disable.
        double activationTime @unit("s") = default(0s);	// The initial activation delay. Zero to disable.
        xml config;										// The XML configuration file for random sites
}

