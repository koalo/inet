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

namespace inet;

//
// IEEE802.15.4 Beacon Frame Format (IEEE802.15.4-2011 page 61)
//


struct SuperframeSpecification {
    short beaconOrder;				// 4 bits
    short superframeOrder;			// 4 bits
    short finalCAPSlot;				// 4 bits
    bool batteryLifeExtension;
    bool reserved;
    bool PANCoordinator;
    bool associationPermit;
}

struct PendingAddressSpecification {
    short numShortAddrs;			// 3 bits
    bool reserved0;
    short numExtAddrs;				// 3 bits
    bool reserved1;
}

packet BeaconFrame {
    // TODO
}
