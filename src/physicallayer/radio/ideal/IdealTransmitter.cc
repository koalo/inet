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

#include "IdealTransmitter.h"
#include "IdealTransmission.h"
#include "IMobility.h"

using namespace radio;

Define_Module(IdealTransmitter);

void IdealTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        bitrate = bps(par("bitrate"));
        maxCommunicationRange = m(par("maxCommunicationRange"));
        maxInterferenceRange = m(par("maxInterferenceRange"));
        maxDetectionRange = m(par("maxDetectionRange"));
    }
}

void IdealTransmitter::printToStream(std::ostream &stream) const
{
    stream << "ideal transmitter, bitrate = " << bitrate << ", "
           << "maxCommunicationRange = " << maxCommunicationRange << ", "
           << "maxInterferenceRange = " << maxInterferenceRange << ", "
           << "maxDetectionRange = " << maxDetectionRange;
}

const ITransmission *IdealTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const simtime_t duration = (b(macFrame->getBitLength()) / bitrate).get();
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new IdealTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, maxCommunicationRange, maxInterferenceRange, maxDetectionRange);
}