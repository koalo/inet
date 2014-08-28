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

#ifndef __INET_SHORTBITVECTOR_H_
#define __INET_SHORTBITVECTOR_H_

#include "INETDefs.h"

namespace inet {

#define MAX_LENGTH 32

/*
 *  It is the optimized version of the BitVector class and it only allows to store 32 bits.
 */
class ShortBitVector
{
    public:
        static const ShortBitVector UNDEF;

    protected:
        unsigned int bitVector;
        bool undef;
        int size;
        void stringToBitVector(const char *str);
        void copy(const ShortBitVector& other);

    public:
        inline unsigned int toDecimal() const
        {
            if (undef)
                throw cRuntimeError("You can't compute the decimal value of an undefined ShortBitVector");
            return bitVector;
        }
        inline unsigned int reverseToDecimal() const
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
        inline void rightShift(int with)
        {
            if (undef)
                throw cRuntimeError("You can't shift an undefined ShortBitVector");
            bitVector = bitVector >> with;
        }
        inline void leftShift(int with)
        {
            if (undef)
                throw cRuntimeError("You can't shift an undefined ShortBitVector");
            bitVector = bitVector << with;
        }
        inline void appendBit(bool value)
        {
            setBit(size, value);
        }
        inline void appendBit(bool value, int n)
        {
            while (n--)
                appendBit(value);
        }
        inline void setBit(int pos, bool value)
        {
            if (pos >= MAX_LENGTH)
                throw cRuntimeError("Out of range with bit position: %d", pos);
            undef = false;
            if (pos + 1 > size)
                size = pos + 1;
            if (value)
                bitVector |= 1 << pos;
            else
                bitVector &= ~(1 << pos);
        }
        inline void toggleBit(int pos)
        {
            if (undef)
                throw cRuntimeError("You can't toggle bits in an undefined ShortBitVector");
            if (pos >= size)
                throw cRuntimeError("Out of range with bit position %d", pos);
            bitVector ^= 1 << pos;
        }
        inline bool getBit(int pos) const
        {
            if (undef)
                throw cRuntimeError("You can't get bits from an undefined ShortBitVector");
            if (pos >= size)
                throw cRuntimeError("Out of range with bit position %d", pos);
            return bitVector & (1 << pos);
        }
        inline bool isUndef() const { return undef; }
        inline bool getBitAllowNegativePos(int pos) const
        {
            if (undef)
                throw cRuntimeError("You can't get bits from an undefined ShortBitVector");
            if (pos < 0)
                return false;
            return getBit(pos);
        }
        inline bool getBitAllowOutOfRange(int pos) const
        {
            if (undef)
                throw cRuntimeError("You can't get bits from an undefined ShortBitVector");
            if (pos >= size)
                return false;
            return getBit(pos);
        }
        inline unsigned int getSize() const { return size; }
        inline unsigned int computeHammingDistance(const ShortBitVector& u) const
        {
            if (u.isUndef() || isUndef())
                throw cRuntimeError("You can't compute the Hamming distance between undefined BitVectors");
            if (getSize() != u.getSize())
                throw cRuntimeError("You can't compute Hamming distance between two vectors with different sizes");
            unsigned int hammingDistance = bitVector ^ u.toDecimal();
            hammingDistance = hammingDistance - ((hammingDistance >> 1) & 0x55555555);
            hammingDistance = (hammingDistance & 0x33333333) + ((hammingDistance >> 2) & 0x33333333);
            return (((hammingDistance + (hammingDistance >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
        }
        friend std::ostream& operator<<(std::ostream& out, const ShortBitVector& bitVector);
        ShortBitVector& operator=(const ShortBitVector& rhs);
        bool operator==(const ShortBitVector& rhs) const;
        bool operator!=(const ShortBitVector& rhs) const;
        std::string toString() const;
        ShortBitVector();
        ShortBitVector(const char *str);
        ShortBitVector(unsigned int num);
        ShortBitVector(unsigned int num, unsigned int fixedSize);
        ShortBitVector(const ShortBitVector& other) { copy(other); }
};

} /* namespace inet */

#endif /* __INET_SHORTBITVECTOR_H_ */
