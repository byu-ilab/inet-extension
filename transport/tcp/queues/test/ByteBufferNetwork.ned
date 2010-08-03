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

import ned.DatarateChannel;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.StandardHostWithTCPSocketMgr;
import inet.httptools.base.httptController;

network ByteBufferNetwork
{
    parameters:
        @display("bgb=745,636");

        int numClients;

    types:
        channel CustomerToDSLAM extends DatarateChannel
        {
            datarate = 10Mbps;
        }

        channel DSLAMToOrigin extends DatarateChannel
        {
            datarate = 1Gbps;
        }

    submodules:
        configurator: FlatNetworkConfigurator {
            @display("is=s;p=443,84");
        }

        controller: httptController {
            @display("is=s;p=539,84");
        }

        server: StandardHostWithTCPSocketMgr {
            parameters:
                @display("i=device/server_l;p=72,84");
        }

        clients[numClients]: StandardHostWithTCPSocketMgr {
            parameters:
                @display("i=device/pc;p=78,220,m,10,50,70");
        }

        dslam: Router {
            @display("p=226,84");
        }

    connections:
        dslam.pppg++ <--> DSLAMToOrigin <--> server.pppg++;

        for n=0..numClients-1 {
            clients[n].pppg++ <--> CustomerToDSLAM <--> dslam.pppg++;
        }
}


