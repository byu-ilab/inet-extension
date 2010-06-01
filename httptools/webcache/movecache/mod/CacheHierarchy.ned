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

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;
import inet.httptools.*;
package inet.httptools.webcache;

network CacheHierarchy
{
    parameters:
        //int numServers;
        int numCaches;
        int clustersPerCache;

        @display("bgb=745,636");
    types:
        channel ClusterToCache extends DatarateChannel
        {
            datarate = 1Mbps;
            delay = 50000us;
        }
        channel CacheToSRouter extends DatarateChannel
        {
            datarate = 1Mbps;
            delay = 50000us;
        }
        channel SRouterToServer extends DatarateChannel
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
        server: StandardHost {
            parameters:
                @display("p=394,34,row,100;i=device/server");
        }
        caches[numCaches]: StandardHost {
            parameters:
                @display("p=179,248,row,100,device/pc1;i=device/server2");
        }
        tlcache: StandardHost {
            parameters:
            @display("p=394,117");
        }
        clusters[numCaches * clustersPerCache]: ClientCluster {
            parameters:
                @display("p=92,483,row,100;i=block/network2");
        }

        serverRouter: Router {
            parameters:
                @display("p=394,190");
        }
        cacheRouters[numCaches]: Router {
            parameters:
                @display("p=145,350,row,100");
        }

    connections:
        server.pppg++ <--> SRouterToServer <--> tlcache.pppg++;
        tlcache.pppg++ <--> SRouterToServer <--> serverRouter.pppg++;

        for c=0..(numCaches-1) {
            caches[c].pppg++ <--> CacheToSRouter <--> serverRouter.pppg++;
            cacheRouters[c].pppg++ <--> ClusterToCache <--> caches[c].pppg++;
        }
        for d=0..(numCaches*clustersPerCache)-1 {
            cacheRouters[d/clustersPerCache].pppg++ <--> ClusterToCache <--> clusters[d].pppg;
        }
}
