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


//
// A wireless host containing routing, mobility and battery components.
// Supports only IPv4 protocol, TCP and UDP as transport protocol.
// This is a typical mobile node which can participate in adhoc routing
// and may have TCP/UDP applications installed. Supports ICMP (ping) too.
//
// - By default contains a single wireless cards, however it can be configured
//   by the numRadios parameter. Wirless card type is configured by the
//   **.wlan.typename parameter. see:  inet.linklayer.ieee80211 or other
//   modules implementing ~IWirelessNic
// - Node mobility can be set using **.mobility.typename
//   see: inet.mobility and ~IMobility
//
module ModelHost extends StandardHost
{
    parameters:
        numRadios = default(1);
        wlan[0].typename = default("Ieee802154NarrowbandNic");

        trafficgen.protocol = 59; // TODO possible with enum?
        trafficgen.packetLength = 10B;
        @statistic[rcvdPkSrc](title="packets received per source"; source=rcvdPk; record="groupCount(sourceAddr)"; interpolationmode=none);

        @display("i=device/accesspoint;bgb=,-");
        //        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");  // use adhoc management
        forwarding = default(true);

    submodules:
        
        trafficgen: IPvXTrafGen {
            @display("p=149,287");
        }

        dummyTransport: DummyTransport {
            @display("p=225,287");
        }

    connections:
        trafficgen.ipOut --> dummyTransport.upperLayerIn++;
        trafficgen.ipIn <-- dummyTransport.upperLayerOut++;

        networkLayer.transportOut++ --> dummyTransport.lowerLayerIn;
        networkLayer.transportIn++ <-- dummyTransport.lowerLayerOut;
}

