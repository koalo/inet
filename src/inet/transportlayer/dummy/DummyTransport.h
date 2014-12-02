//
// Copyright (C) 2014 Florian Meier
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

#ifndef __INET_DummyTransport_H
#define __INET_DummyTransport_H

#include <map>
#include <list>

#include "inet/transportlayer/common/TransportProtocolBase.h"

namespace inet {

const int DummyTransport_HEADER_BYTES = 8;

/**
 * Implements the DummyTransport protocol: encapsulates/decapsulates user data into/from DummyTransport.
 *
 * More info in the NED file.
 */
class INET_API DummyTransport : public TransportProtocolBase
{
  public:
    DummyTransport();
    virtual ~DummyTransport();

    virtual void handleUpperPacket(cPacket *packet);
    virtual void handleLowerPacket(cPacket *packet);

    virtual bool isUpperMessage(cMessage *message);
    virtual bool isLowerMessage(cMessage *message);

    virtual void handleUpperCommand(cMessage *message);

    virtual void initialize(int stage);

  private:
    /** @brief Gate ids */
    //@{
    int upperLayerInGateId;
    int upperLayerOutGateId;
    int lowerLayerInGateId;
    int lowerLayerOutGateId;
    //@}

};

} // namespace inet

#endif // ifndef __INET_DummyTransport_H

