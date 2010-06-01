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

package inet.httptools.webcache.movecache.mod;

import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;
//
// TODO auto-generated module
//
module ClientCluster
{
    parameters:
        int numClients;
    gates:
        inout pppg @labels(PPPFrame-conn);
    types:
        channel ClientRouter extends DatarateChannel
        {
            datarate = 1Mbps;
            delay = 50000us;
        }
    submodules:
        router: Router {
            @display("p=113,58");
        }
        clients[numClients]: StandardHost {
            @display("p=66,173,row,100;i=device/pc");
        }
    connections:
        pppg <--> router.pppg++;
        for i=0..numClients-1 {
            clients[i].pppg++ <--> ClientRouter <--> router.pppg++;
        }
}
