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

#include "ShortBitVector.h"
#include <algorithm>

namespace inet {

const ShortBitVector ShortBitVector::UNDEF = ShortBitVector();

ShortBitVector::ShortBitVector()
{
    undef = true;
    size = 0;
    bitVector = 0;
}

std::ostream& operator<<(std::ostream& out, const ShortBitVector& bitVector)
{
    if (bitVector.isUndef())
        out << "**UNDEFINED BITVECTOR**";
    else
    {
        if (bitVector.getBit(0))
            out << "1";
        else
            out << "0";
        for (int i = 1; i < bitVector.getSize(); i++)
        {
            if (bitVector.getBit(i))
                out << " 1";
            else
                out << " 0";
        }
    }
    return out;
}

std::string ShortBitVector::toString() const
{
    if (undef)
        throw cRuntimeError("You can't convert an undefined ShortBitVector to a string");
    std::string str;
    for (unsigned int i = getSize() - 1; i >= 0; i--)
    {
        if (getBit(i))
            str += "1";
        else
            str += "0";
    }
    return str;
}

ShortBitVector::ShortBitVector(const char* str)
{
    undef = false;
    size = 0;
    bitVector = 0;
    stringToBitVector(str);
}

void ShortBitVector::stringToBitVector(const char *str)
{
    int strSize = strlen(str);
    for (int i = strSize - 1; i >= 0; i--)
    {
        if (str[i] == '1')
            appendBit(true);
        else if (str[i] == '0')
            appendBit(false);
        else
            throw cRuntimeError("str must represent a binary number");
    }
}

int ShortBitVector::computeHammingDistance(const ShortBitVector& u) const
{
    if (u.isUndef() || isUndef())
        throw cRuntimeError("You can't compute the Hamming distance between undefined BitVectors");
    if (getSize() != u.getSize())
        throw cRuntimeError("You can't compute Hamming distance between two vectors with different sizes");
    int hammingDistance = 0;
    for (unsigned int i = 0; i < getSize(); i++)
        if (u.getBit(i) != getBit(i))
            hammingDistance++;
    return hammingDistance;
}

ShortBitVector::ShortBitVector(unsigned int num)
{
    undef = false;
    if (num < 0)
        throw cRuntimeError("num = %d must be a positive integer", num);
    if (num == 0)
        size = 1;
    else
        size = (int)(log(num) / log(2)) + 1;
    bitVector = num;
}

ShortBitVector& ShortBitVector::operator=(const ShortBitVector& rhs)
{
    if (this == &rhs)
        return *this;
    copy(rhs);
    return *this;
}

bool ShortBitVector::operator==(const ShortBitVector& rhs) const
{
    if (rhs.isUndef() && isUndef())
        return true;
    if (rhs.isUndef() || isUndef())
        throw cRuntimeError("You can't compare undefined BitVectors");
    return rhs.bitVector == bitVector;
}

bool ShortBitVector::operator!=(const ShortBitVector& rhs) const
{
    return !(rhs == *this);
}

unsigned int ShortBitVector::reverseToDecimal() const
{
    unsigned int dec = 0;
    unsigned int powerOfTwo = 1;
    for (int i = getSize() - 1; i >= 0; i--)
    {
        if (getBit(i))
            dec += powerOfTwo;
        powerOfTwo *= 2;
    }
    return dec;
}

ShortBitVector::ShortBitVector(unsigned int num, unsigned int fixedSize)
{
    undef = false;
    if (fixedSize > MAX_LENGTH)
        throw cRuntimeError("fixedSize = %d must be less then 32", fixedSize);
    size = fixedSize;
    if (num < 0)
        throw cRuntimeError("num = %d must be a positive integer", num);
    bitVector = num;
}

void ShortBitVector::copy(const ShortBitVector& other)
{
    undef = other.undef;
    size = other.size;
    bitVector = other.bitVector;
}

} /* namespace inet */

