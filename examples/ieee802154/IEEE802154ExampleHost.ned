//
// Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
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

package inet.examples.ieee802154;

import inet.applications.generic.IPvXTrafGen;
import inet.transportlayer.dummy.DummyTransport;
import inet.node.inet.StandardHost;
import inet.linklayer.ieee802154.Ieee802154NarrowbandNic;
import inet.networklayer.geographic.NeighborTable;


//
// A wireless host containing an Ieee802154NarrowbandNic
// and a IPvXTrafGen.
//
module IEEE802154ExampleHost extends StandardHost
{
    parameters:
        string neighborTableType = default("NeighborTable");
        *.neighborTableModule = default(neighborTableType != "" ? absPath(".neighborTable") : "");
        
        numRadios = default(1);
        wlan[0].typename = default("Ieee802154NarrowbandNic");

        trafficgen.protocol = 59; // TODO possible with enum?
        trafficgen.packetLength = 10B;

        @statistic[rcvdPkSrc](title="packets received per source"; source=rcvdPk; record="groupCount(sourceAddr)"; interpolationmode=none);

        @display("i=device/accesspoint;bgb=,-");
        forwarding = default(true);

    submodules:
        trafficgen: IPvXTrafGen {
            @display("p=149,287");
        }
        
        neighborTable: NeighborTable { // <neighborTableType> like ...
            @display("p=149,300");
        }

    connections:
        networkLayer.transportOut++ --> trafficgen.ipIn;
        networkLayer.transportIn++ <-- trafficgen.ipOut;

}
