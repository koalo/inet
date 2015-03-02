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
#include "inet/linklayer/ieee802154e/DSME_GTSRequestCmd_m.h"
#include "inet/linklayer/ieee802154e/DSME_GTSReplyCmd_m.h"
#include "inet/linklayer/ieee802154e/BeaconBitmap.h"
#include "inet/linklayer/ieee802154e/GTS.h"
#include "inet/linklayer/ieee802154e/DSMESlotAllocationBitmap.h"
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
    bool isBeaconAllocationSent;
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

    // Guaranteed Time Slots
    std::pair<MACAddress, std::set<GTS>> allocatedGTSsTX;
    std::pair<MACAddress, std::set<GTS>> allocatedGTSsRX;
    DSMESlotAllocationBitmap occupiedGTSs;
    std::list<IEEE802154eMACFrame*> GTSQueue;

    // packets
    EnhancedBeacon *beaconFrame;
    IEEE802154eMACFrame *csmaFrame;

    // timers
    cMessage *beaconIntervalTimer;
    cMessage *nextSlotTimer;
    cMessage *nextCSMASlotTimer;
    cMessage *nextGTSlotTimer;

    // slotted csma uses contentionWindow
    unsigned contentionWindow;
    unsigned contentionWindowInit;


public:
    DSME();
    ~DSME();
    virtual void initialize(int);

    virtual void handleSelfMessage(cMessage *);
    virtual void handleUpperPacket(cPacket *);
    virtual void handleLowerPacket(cPacket *msg);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value);

    /**
     * Allocate new GTS Slots to specified Address
     * @param num amount of slots to allocate
     * @param direction 0 -> TX, 1 -> RX
     * @param addr address to allocate slots for
     */
    virtual void allocateGTSlots(uint8_t numSlots, bool direction, MACAddress addr);

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
    virtual void sendGTS(IEEE802154eMACFrame *);

    /**
     * Gets time and channel of next GTSlot for address
     * @param addr Address to send to
     */
    GTS getNextGTSlot(MACAddress addr);

    /**
     * Send a GTS-request to device
     * To de-/allocate slots or notify about duplicate allocation
     */
    virtual void sendGTSRequest(DSME_GTSRequestCmd*, MACAddress addr);

    /**
     * Called on reception of a GTS-request.
     * handles de-/allocation and duplication requests
     */
    virtual void handleGTSRequest(IEEE802154eMACCmdFrame *);

    /**
     * Reply to GTS-request
     */
    virtual void sendGTSReply(DSME_GTSReplyCmd *);

    /**
     * Allocate slot on status "succeed" and send GTSNotifiy
     */
    virtual void handleGTSReply(IEEE802154eMACCmdFrame *);

    /**
     * Broadcast GTS Notifiy
     */
    //virtual void sendGTSNotify();

    /**
     * Update slot allocation on reception of GTS Notify
     */
    //virtual void handleGTSNotify();

    /**
     * Send ACK message
     */
    virtual void sendACK(MACAddress);

    virtual void handleBroadcastAck(CSMAFrame *ack, CSMAFrame *frame);

    /**
     * Directly send packet without delay and without CSMA
     */
    virtual void sendDirect(cPacket *);

    /**
     * Gets time of next CSMA slot
     */
    simtime_t getNextCSMASlot();

    /**
     * Handle start of CSMA Slot for slotted-CSMA,
     * perform CCA or directly send message
     */
    void handleCSMASlot();

    /**
     * Send packet directly using CSMA
     */
    virtual void sendCSMA(IEEE802154eMACFrame *, bool requestACK);

    /**
     * Gets called when CSMA Message was sent down to the PHY
     */
    virtual void onCSMASent();

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

    /**
     * Send beacon collision notification
     */
    virtual void sendBeaconCollisionNotification(uint16_t beaconSDIndex, MACAddress addr);

    /**
     * Handle reception of beacon collision notification.
     * Update beacon allocation
     */
    virtual void handleBeaconCollision(IEEE802154eMACCmdFrame *);

};

} //namespace

#endif
