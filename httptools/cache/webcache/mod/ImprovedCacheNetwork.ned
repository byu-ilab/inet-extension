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
package inet.httptools.cache.webcache.mod;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;
import inet.nodes.inet.StandardHostWithTCPSocketAPI;
import inet.httptools.base.*;

network ImprovedCacheNetwork
{
    parameters:
        //int numClients;
        //int numServers;

        @display("bgb=296,384");
    types:
        channel C extends DatarateChannel
        {
            datarate = 1Mbps;
            delay = 50000us;
        }
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=61,163");
        }
        controller: httptController {
            parameters:
                @display("p=61,98");
        }
        client: StandardHost { //s[numClients]: StandardHost {
            parameters:
                @display("p=100,322,row,100;i=device/pc3");
        }
        server: StandardHost { //s[numServers]: StandardHost {
            parameters:
                @display("p=245,72,row,100;i=device/pc2");
        }
        cache: StandardHostWithTCPSocketAPI {
            parameters:
                @display("p=135,171,i=device/pc1");
        }
        router: Router {
            parameters:
                @display("p=206,257");
        }

    connections:
        client.pppg++ <--> C <--> router.pppg++;
        cache.pppg++ <--> C <--> router.pppg++;
        cache.pppg++ <--> C <--> server.pppg++;
        //for c=0..numClients-1 {
        //    clients[c].pppg++ <--> C <--> router.pppg++;
        //}

        //for s=0..numServers-1 {
        //    servers[s].pppg++ <--> C <--> router.pppg++;
        //}
}