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

#include "inet/linklayer/ieee802154e/BeaconBitmap.h"

namespace inet {

int32_t BeaconBitmap::getNextAllocated(uint16_t start) {
    for(uint16_t i=start; i<SDBitmap.getSize(); i++) {
        if (SDBitmap.getBit(i))
            return i;
    }
    for(uint16_t i=0; i<start; i++) {
        if (SDBitmap.getBit(i))
            return i;
    }

    return -1;
}

int32_t BeaconBitmap::getFreeSlot() {
    for(uint16_t i=0; i<SDBitmap.getSize(); i++) {
        if (!SDBitmap.getBit(i))
            return i;
    }

    return -1;
}

uint16_t BeaconBitmap::getAllocatedCount() {
    uint16_t c = 0;
    for(unsigned int i=0; i<SDBitmap.getSize(); i++) {
        if (SDBitmap.getBit(i))
            c++;
    }
    return c;
}

} /* namespace inet */
