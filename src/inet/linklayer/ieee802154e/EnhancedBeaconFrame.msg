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
// IEEE802.15.4e
// Enhanced Beacon Frame
// As defined in IEEE802.15.4e-2012 on page 64.
//
message EnhancedBeaconFrame {
    
    // MHR
    int16_t FrameControl;
    int8_t SequenceNumber;
    
    // - Addressing Fields
        int16_t SourcePANIdentifier;
        int64_t SourceAddress;
    
    // - AuxiliarySecurityHeader
    	int8_t AuxiliarySecurityHeader[];
    
    // - Header IEs
        int16_t Header__Length_ElementId_Type;
        int8_t Header__IEContent[];
    
    // MAC Payload
    // - Payload IEs
        int16_t Header__Length_GroupID_Type;
        int8_t Header__IEContent[];
    
    int8_t BeaconPayload[];
    
    // MFR
    int16_t FCS;
    
}
