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

package inet.httptools.cache.webcache.test.extfilter;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.StandardHostWithTCPSocketAPI;
import ned.DatarateChannel;
import inet.httptools.base.httptController;

network CacheExtensionFilterTest
{
    parameters:
        int numClients;
        //int numServers;

        @display("bgb=409,384");
    types:
        channel C extends DatarateChannel
        {
            datarate = 1Mbps;
            delay = 0.1us;
        }
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=61,137");
        }
        controller: httptController {
            parameters:
                @display("p=61,72");
        }

        clients[numClients]: StandardHost { //s[numClients]: StandardHost {
            parameters:
                @display("p=100,322,row,100;i=device/pc3");
        }
        server: StandardHost { //s[numServers]: StandardHost {
            parameters:
                @display("p=135,72,row,100;i=device/pc2");
        }
        router: Router {
            parameters:
                @display("p=261,202");
        }
        cache: StandardHostWithTCPSocketAPI {
            parameters:
                @display("p=261,72");
        }
    connections:
        //        client.pppg++ <--> C <--> router.pppg++;
        router.pppg++ <--> C <--> cache.pppg++;
        cache.pppg++ <--> C <--> server.pppg++;

        for c=0..numClients-1 {
            clients[c].pppg++ <--> C <--> router.pppg++;
        }

        //for s=0..numServers-1 {
        //    servers[s].pppg++ <--> C <--> router.pppg++;
        //}
}
