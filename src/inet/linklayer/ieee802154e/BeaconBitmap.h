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

#ifndef BEACONBITMAP_H_
#define BEACONBITMAP_H_

#include "inet/common/BitVector.h"

namespace inet {

class BeaconBitmap {
public:
    uint16_t SDIndex;           // current duration in beacon interval
    uint16_t SDBitmapLength;    // length of bitmap in bytes
    BitVector SDBitmap;         // bitmap
};

} /* namespace inet */

#endif /* BEACONBITMAP_H_ */
