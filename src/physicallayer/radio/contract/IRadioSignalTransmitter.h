//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_IRADIOSIGNALTRANSMITTER_H_
#define __INET_IRADIOSIGNALTRANSMITTER_H_

#include "IPrintableObject.h"

/**
 * This interface represents a physical device (a part of the radio) which converts
 * packets into electric signals.
 */
class INET_API IRadioSignalTransmitter : public IPrintableObject
{
    public:
        /**
         * Returns the maximum transmission power above which no transmission is
         * ever transmitted. Returns a value in the range [0, +infinity] or NaN
         * if unspecified.
         */
        virtual W getMaxPower() const = 0;

        /**
         * Returns a transmission which describes the radio signal corresponding
         * to the provided mac frame. This function never returns NULL.
         */
        virtual const IRadioSignalTransmission *createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const = 0;
};

#endif