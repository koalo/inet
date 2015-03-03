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
    #include "inet/common/BitVector.h"
}}

namespace inet;

//
// IEEE802.15.4e-2012
// 5.3.11.4 DSME-GTS request command payload
//

enum ManagementType {
    DEALLOCATION = 0;
    ALLOCATION = 1;
    DUPLICATED_ALLOCATION_NOTIFICATION = 2;
    REDUCE = 3;
    RESTART = 4;
    EXPIRATION = 5;
}

enum StatusType {
    ALLOCATION_APPROVED = 0;
    ALLOCATION_DISAPPROVED = 1;
    OTHER_DISAPPROVED = 2;
}

struct DSME_GTS_Management {
    short type @enum(ManagementType);	// 3 bits
    bool direction;						// TX -> 0, RX -> 1;   (only for allocation)
    bool prioritizedChannelAccess;
    short status @enum(StatusType);
}

class noncobject BitVector;

struct DSME_SAB_Specification {
    uint8_t subBlockLength;
    uint16_t subBlockIndex;
    BitVector subBlock;			// one subblock has as many bits as GTS per superframe
    							// p. 114: row = time slot, column = channel
    							//         1 => occupied (only for allocation)
    							// 14 byte without CAP reduction 
}

packet DSME_GTSRequestCmd {
    byteLength = 4 + 3 + 14;
    
    DSME_GTS_Management gtsManagement;
    
    uint8_t numSlots;
    uint16_t preferredSuperframeID;
    uint8_t preferredSlotID;
    
    DSME_SAB_Specification SABSpec;
}
