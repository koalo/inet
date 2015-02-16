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

cplusplus {{
   #include "inet/linklayer/ieee802154/BeaconFrame_m.h"
   #include "inet/linklayer/ieee802154e/BeaconBitmap.h"
}}

namespace inet;

//
// IEEE802.15.4e
// DSME PAN descriptor information element -> payload of Enhanced Beacons Frames
// As defined in IEEE802.15.4e-2012 on page 85.
//



struct DSMESuperframeSpecification {
    short multiSuperframeOrder;				// 3 bits
    bool channelDiversityMode;
    bool GACKFlag;
    bool CAPReductionFlag;
    bool deferredBeaconFlag;
}

struct TimeSyncSpecification {
    uint64_t beaconTimestamp;				// 6 bytes
    uint16_t beaconOffsetTimestamp;			// 2 bytes
}


class noncobject BeaconBitmap;
struct SuperframeSpecification;
struct PendingAddressSpecification;

packet DSME_PANDescriptor {
    SuperframeSpecification superframeSpec;
    
    PendingAddressSpecification pendingAddressSpec;
    
    DSMESuperframeSpecification dsmeSuperframeSpec;
    
    TimeSyncSpecification timeSyncSpec;
    
    BeaconBitmap beaconBitmap;
    
    //ChannelHoppingSpecification
    
    //GroupACKSpecification
}
