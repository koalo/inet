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
//import inet.transportlayer.dummy.DummyTransport;
import inet.node.inet.StandardHost;
import inet.linklayer.ieee802154.Ieee802154NarrowbandNic;
import inet.networklayer.geographic.INeighborTable;


//
// A wireless host containing an Ieee802154NarrowbandNic
// and a IPvXTrafGen.
//
module IEEE802154ExampleHost extends StandardHost
{
    parameters:
        bool isPANCoordinator = default(false);
        bool isCoordinator = default(isPANCoordinator);
        
        string neighborTableType = default("StraightestNeighborTable");
        *.neighborTableModule = default(neighborTableType != "" ? absPath(".neighborTable") : "");

        numRadios = default(1);
        wlan[0].typename = default("Ieee802154NarrowbandNic");

        trafficgen.protocol = 59; // TODO possible with enum?
        trafficgen.packetLength = default(10B);

        @statistic[rcvdPkSrc](title="packets received per source"; source=rcvdPk; record="groupCount(sourceAddr)"; interpolationmode=none);

        @display("i=device/accesspoint;bgb=,-");
        forwarding = default(true);

    submodules:
        trafficgen: IPvXTrafGen {
            @display("p=149,287");
        }

        neighborTable: <neighborTableType> like INeighborTable {
            @display("p=117,231");
        }

    connections:
        networkLayer.transportOut++ --> trafficgen.ipIn;
        networkLayer.transportIn++ <-- trafficgen.ipOut;

}
