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
#include "inet/linklayer/ieee802154e/IEEE802154eMACFrame_m.h"
#include "inet/linklayer/ieee802154e/IEEE802154eMACCmdFrame_m.h"
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

    bool isAssociated;
    bool isBeaconAllocated;

    double slotDuration;
    double baseSuperframeDuration;
    double superframeDuration;
    double beaconInterval;

    unsigned numCSMASlots;
    unsigned numberSuperframes;

    unsigned currentSlot;
    simtime_t nextSlotTimestamp;
    unsigned slotsPerSuperframe;

    simtime_t lastHeardBeaconTimestamp;
    uint16_t lastHeardBeaconSDIndex;
    BeaconBitmap heardBeacons;
    BeaconBitmap neighborHeardBeacons;

    SuperframeSpecification superframeSpec;
    PendingAddressSpecification pendingAddressSpec;
    DSMESuperframeSpecification dsmeSuperframeSpec;
    TimeSyncSpecification timeSyncSpec;
    BeaconBitmap beaconAllocation;
    DSME_PANDescriptor PANDescriptor;

    // packets
    EnhancedBeacon *beaconFrame;
    IEEE802154eMACFrame_Base *csmaFrame;

    // timers
    cMessage *beaconIntervalTimer;
    cMessage *nextSlotTimer;
    cMessage *nextCSMASlotTimer;
    cMessage *nextGTSlotTimer;


public:
    DSME();
    ~DSME();
    virtual void initialize(int);

    virtual void handleSelfMessage(cMessage *);
    virtual void handleLowerPacket(cPacket *msg);

protected:

    /**
     * Called after BeaconInterval
     * May trigger another scan on some channel if no becons were heard.
     * Otherwise device now knows on or more Coordinators and may start
     * Association-/ Beacon- /Slot requests
     */
    virtual void endChannelScan();

    /**
     * Send packet at next available GTS
     */
    virtual void sendGTS(IEEE802154eMACFrame_Base *);

    /**
     * Directly send packet without delay and without CSMA
     */
    virtual void sendDirect(cPacket *);

    /**
     * Slotted CSMA-CA
     */
    //virtual simtime_t scheduleBackoff();

    /**
     * Send packet at next available CSMA slot
     */
    //virtual void sendSlottedCSMA(IEEE802154eMACFrame_Base *);

    /**
     * Gets time of next CSMA slot
     */
    simtime_t getNextCSMASlot();

    /**
     * Send packet directly using CSMA
     */
    virtual void sendCSMA(IEEE802154eMACFrame_Base *);

    /**
     * Send an enhanced Beacon directly
     */
    virtual void sendEnhancedBeacon();

    /**
     * Called on reception of an EnhancedBeacon
     */
    virtual void handleEnhancedBeacon(EnhancedBeacon *);

    /**
     * Send beacon allocation notification
     */
    virtual void sendBeaconAllocationNotification(uint16_t beaconSDIndex);

    /**
     * Called on reception of an BeaconAllocationNotification
     */
    virtual void handleBeaconAllocation(IEEE802154eMACCmdFrame *);
};

} //namespace

#endif
