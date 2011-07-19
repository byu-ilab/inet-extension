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

package inet.httptools.videocomponents.client.test;
import inet.httptools.videocomponents.workgen.NullWorkloadGenerator;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.StandardHostWithTCPSocketMgr;

import ned.DatarateChannel;
import inet.httptools.base.*;

network TestNetwork
{
    parameters:

    types:
        channel C1 extends DatarateChannel
        {
            datarate = 1Mbps;
            delay = 1ms;
        }
        channel C2 extends DatarateChannel
        {
            datarate = 1Mbps;
            delay = 1ms;
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
        workloadGenerator: NullWorkloadGenerator {
            @display("is=s;p=654,84");
        }
        client: StandardHostWithTCPSocketMgr { //s[numClients]: StandardHost {
            parameters:
                @display("p=574,137,row,100;i=device/pc3");
        }
        server: StandardHostWithTCPSocketMgr { //s[numServers]: StandardHost {
            parameters:
                @display("p=135,137,row,100;i=device/pc2");
        }

        router: Router {
            parameters:
                @display("p=346,137");
        }

    connections:
        server.pppg++ <--> C1 <--> router.pppg++;
        router.pppg++ <--> C2 <--> client.pppg++;
}
