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

#ifndef __INET_DSME_H_
#define __INET_DSME_H_

#include <omnetpp.h>

#include "inet/linklayer/csma/CSMA.h"
#include "inet/linklayer/ieee802154e/DSME_PANDescriptor_m.h"
#include "inet/linklayer/ieee802154e/BeaconBitmap.h"
#include "inet/linklayer/ieee802154e/EnhancedBeacon.h"

namespace inet {

/**
 * IEEE802.15.4e
 * Deterministic and synchronous multi-channel extension
 */
class DSME : public CSMA
{
protected:
    bool isPANCoordinator;
    bool isCoordinator;

    double baseSuperframeDuration;
    double beaconInterval;

    SuperframeSpecification superframeSpec;
    PendingAddressSpecification pendingAddressSpec;
    DSMESuperframeSpecification dsmeSuperframeSpec;
    TimeSyncSpecification timeSyncSpec;
    BeaconBitmap beaconAllocation;
    DSME_PANDescriptor PANDescriptor;

    EnhancedBeacon *beaconFrame;

    // timers
    cMessage *beaconTimer;


public:
    DSME();
    ~DSME();
    virtual void initialize(int);

    virtual void handleSelfMessage(cMessage *);
    virtual void handleLowerPacket(cPacket *msg);

protected:

    virtual void sendDirect(cPacket *);
    virtual void sendCSMA(cPacket *);
    virtual void sendEnhancedBeacon();
};

} //namespace

#endif
