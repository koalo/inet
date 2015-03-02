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
#include "inet/linklayer/ieee802154e/IEEE802154eMACFrame_m.h"
}}

namespace inet;
//
// IEEE802.15.4e
// MAC Command Frame Format
// As defined in IEEE802.15.4e-2012 on page 67.
//


// Table 5 on page 95
enum CommandFrameIdentifier {
    ASSOCIATION_REQUEST = 0x01;
    ASSOCIATION_RESPONSE = 0x02;
    //...
    
    DSME_ASSOCIATION_REQUEST = 0x13;
    DSME_ASSOCIATION_RESPONSE = 0x14;
    DSME_GTS_REQUEST = 0x15;
    DSME_GTS_REPLY = 0x16;
    DSME_GTS_NOTIFY = 0x17;
    // ...
    DSME_BEACON_ALLOCATION_NOTIFICATION = 0x1a;
    DSME_BEACON_COLLISION_NOTIFICATION = 0x1b;
    
    // ...
}


class IEEE802154eMACFrame;

packet IEEE802154eMACCmdFrame extends IEEE802154eMACFrame {
    short cmdId @enum(CommandFrameIdentifier);
}
