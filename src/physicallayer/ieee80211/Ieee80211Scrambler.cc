//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "Ieee80211Scrambler.h"
#include "ShortBitVector.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211Scrambler);

void Ieee80211Scrambler::initialize(int stage)
{

}

BitVector Ieee80211Scrambler::scrambling(const BitVector& bits) const
{
    ShortBitVector shiftRegisters("1111111");
    BitVector scrambledBits;
    for (unsigned int i = 0; i < 128; i++)
    {
        // Our generator polynomial is x^7 + x^4 + 1 so we add bits at positions (7-1) and (4-1).
        bool registerSum = eXOR(shiftRegisters.getBit(6), shiftRegisters.getBit(3));
        shiftRegisters.leftShift(1);
        shiftRegisters.setBit(0, registerSum);
        std::cout << (int)registerSum << " ";
        //bool outputSum = eXOR(registerSum, bits.getBit(i));
        //scrambledBits.appendBit(outputSum);
    }
    std::cout << endl;
    return scrambledBits;
}

} /* namespace physicallayer */
} /* namespace inet */
