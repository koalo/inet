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

package inet.linklayer.ieee802154e;

import inet.linklayer.csma.CSMA;

//
// IEEE802.15.4e
// Deterministic and synchronous multi-channel extension
//
simple DSME extends CSMA
{

    parameters:
        
        double secondsPerSymbol = default(0.000016);		// IEEE802.15.4-2011 page 147
        													// @2460MHz DSSS O-QPSK
        int baseSlotDuration = 60;							// symbols
        int slotsPerSuperframe = 16;
        int numCSMASlots = 8;
        
        // see DSME_PANDescriptor.msg and BeaconFrame.msg
        int beaconOrder = default(6);				// 4 bits
    	int superframeOrder = default(3);			// 4 bits
    	int multiSuperframeOrder = default(5);		// 3 bits
    	
    	int finalCAPSlot = default(8);				// 4 bits
    	
    	// Maximum number GTS which a Device is allowed to allocate per destination
    	// e.g. NumberOfGTSPerMultisuperframe / NumberOfNeighbors
    	int maxNumberGTSAllocPerDevice = default(7);
    	
    	// Slotted CSMA-CA
    	int contentionWindowLength = default(2);
        
    	@class(DSME);
    
}
