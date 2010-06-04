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
        string www;
        int port;
        string controller;
        int httpProtocol;
        volatile int errorReplySize;
        int logLevel;
        string logFile;
} 