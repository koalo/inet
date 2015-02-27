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
    #include "inet/linklayer/ieee802154e/DSME_GTSRequestCmd_m.h"
    #include "inet/linklayer/common/MACAddress.h"
}}

namespace inet;

struct DSME_GTS_Management;
struct DSME_SAB_Specification;
class noncobject MACAddress;


//
// IEEE802.15.4e-2012
// 5.3.11.5 DSME-GTS reply command payload
//
packet DSME_GTSReplyCmd {
    byteLength = 3 + 14;
    
    DSME_GTS_Management gtsManagement;
    
    MACAddress destinationAddress;			// 2 byte short-address encapsulated destination in broadcast message
    
    // channelOffset // for channel hopping
    
    DSME_SAB_Specification SABSpec;
}
