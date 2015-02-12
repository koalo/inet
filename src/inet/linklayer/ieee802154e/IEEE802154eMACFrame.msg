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
	#include "inet/linklayer/common/MACFrameBase_m.h"
}}


namespace inet;

//
// IEEE802.15.4e
// General MAC Frame Format
// As defined in IEEE802.15.4e-2012 on page 59.
//


enum FrameType {
    BEACON = 0;
    DATA = 1;
    ACK = 2;
    MAC_CMD = 3;
    LLDN = 4;
    MULTIPURPOSE = 5;
    //RESERVED = 6-8;
}

enum AddressingMode {
    PAN_ID_AND_ADDR_NOT_PRESENT = 0;
    SIMPLE_ADDR_8BIT = 1;
    SHORT_ADDR_16BIT = 2;
    EXTENDED_ADDR_64BIT = 3;
}

enum FrameVersion {
    IEEE802154_2003 = 0;
    IEEE802154_2006 = 1;
    IEEE802154_XXXX = 2;
    RESERVED = 3;
}

struct FrameControl {
    short type @enum(FrameType);				// 3 bits
    bool securityEnabled;
    bool framePending;
    bool AR;
    bool PANIdCompression;
    bool reserved;
    bool sequnceNumberSuppression;
    bool IEListPresent;
    short destAddrMode @enum(AddressingMode);	// 2 bits
    short version @enum(FrameVersion);			// 2 bits
    short srcAddrMode @enum(AddressingMode);	// 2 bits
}

class MACFrameBase;

packet IEEE802154eMACFrame extends MACFrameBase {
	
	@customize(true);
	
    // MHR
    FrameControl FrameControl;
    uint8_t SequenceNumber;
    
    // - Addressing Fields
    	// source & destination address -> MACFrameBase
    	uint16_t DestinationPANIdentifier;
        uint16_t SourcePANIdentifier;
    
    // - AuxiliarySecurityHeader
    	//uint8_t AuxiliarySecurityHeader[];
    
    // - Header IEs
        //uint16_t Header__Length_ElementId_Type;
        //uint8_t Header__IEContent[];
    
    // MAC Payload
    // - Payload IEs
        //uint16_t Header__Length_GroupID_Type;
        //uint8_t Header__IEContent[];
    
    //uint8_t FramePayload[]; <-- encapsulate(cPacket)
    
    // MFR
    uint16_t FCS;
    
}
