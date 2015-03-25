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
#include "inet/linklayer/ieee802154e/DSME_GTSNotifyCmd_m.h"
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
public:
    // types: Guaranteed Time Slot management
    typedef std::vector<std::vector<GTS>> gts_allocation;     // superframes x slots
    typedef std::map<MACAddress, std::list<IEEE802154eMACFrame*>> gts_queue;

protected:
    cModule *hostModule;

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
    unsigned numberTotalSuperframes;
    unsigned numMaxGTSAllocPerDevice;

    unsigned currentSlot;
    unsigned currentSuperframe;
    unsigned currentChannel;
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

    // Guaranteed Time Slot management
    gts_allocation allocatedGTSs;
    DSMESlotAllocationBitmap occupiedGTSs;
    bool gtsAllocationSent;

    gts_queue GTSQueue;
    IEEE802154eMACFrame *lastSendGTSFrame;

    // packets
    EnhancedBeacon *beaconFrame;
    IEEE802154eMACFrame *dsmeAckFrame;

    // timers
    cMessage *beaconIntervalTimer;
    cMessage *preNextSlotTimer;
    cMessage *nextSlotTimer;
    cMessage *nextCSMASlotTimer;
    cMessage *resetGtsAllocationSent;

    // slotted csma uses contentionWindow
    unsigned contentionWindow;
    unsigned contentionWindowInit;


public:
    DSME();
    ~DSME();
    virtual void initialize(int);
    virtual void finish();

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

    /**
     * Deallocate Slots when not needed anymore or received duplicated allocation notification
     * Given the slot allocation bitmap
     * and the MAC command type to send (DSME_GTS_Request/Reply)
     */
    virtual void deallocateGTSlots(DSME_SAB_Specification, uint8_t cmd);
protected:

    /**
     * Statistics @{
     * numHeardBeacons
     * numNeighborHeardBeacons
     * numTxGtsAllocatedReal
     * numRxGtsAllocatedReal
     */
    long numBeaconCollision;
    simtime_t timeAssociated;
    long numTxGtsAllocated;
    long numRxGtsAllocated;
    long numGtsDuplicatedAllocation;
    long numGtsDeallocated;
    long numAllocationRequestsSent;
    long numAllocationDisapproved;
    simtime_t timeFirstAllocationSent;
    simtime_t timeLastAllocationSent;
    long numTxGtsFrames;
    long numRxGtsFrames;
    simtime_t timeLastTxGtsFrame;
    simtime_t timeLastRxGtsFrame;
    long numUpperPacketsDroppedFullQueue;
    /*@}*/

    /**
     * Called after BeaconInterval
     * May trigger another scan on some channel if no becons were heard.
     * Otherwise device now knows on or more Coordinators and may start
     * Association-/ Beacon- /Slot requests
     */
    virtual void endChannelScan();

    /**
     * Switch Channel for transceiving before next slot
     */
    virtual void switchToNextSlotChannelAndRadioMode();

    /**
     * Update GTS allocation for receiving or transmitting for this device
     */
    virtual void updateAllocatedGTS(DSME_SAB_Specification&, bool direction, MACAddress address);

    /**
     * Remove GTS from allocation when they should not be used anymore.
     */
    virtual void removeAllocatedGTS(std::list<GTS>& gtss);

    /**
     * Get number of allocated slots with given direction
     */
    virtual unsigned getNumAllocatedGTS(bool direction);

    /**
     * Get number of allocated slots to specified address and given direction
     */
    virtual unsigned getNumAllocatedGTS(MACAddress address, bool direction);

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
     * Send Reply broadcast to GTS-request as response and to notify neighbors
     */
    virtual void sendGTSReply(DSME_GTSReplyCmd *);

    /**
     * Update slot allocation
     * Request initiator then sends GTS Notify to its neighbors
     */
    virtual void handleGTSReply(IEEE802154eMACCmdFrame *);

    /**
     * Send GTS Notifiy broadcast to notify neighbors about GTS allocation
     */
    virtual void sendGTSNotify(DSME_GTSNotifyCmd *);

    /**
     * Update slot allocation on reception of GTS Notify
     */
    virtual void handleGTSNotify(IEEE802154eMACCmdFrame *);

    /**
     * Check if recently requested GTSs are already allocated by this device.
     * If so, send a request indicating duplicate allocation.
     * @return true if duplicate allocation were found
     */
    virtual bool checkAndHandleGTSDuplicateAllocation(DSME_SAB_Specification& sabSpec, MACAddress addr);

    /**
     * Send IEEE802154eMACCmdFrame broadcast message.
     * await an ACK response from encapsulated packet destination address
     */
    virtual void sendBroadcastCmd(const char *name, cPacket *payload, uint8_t cmdId);

    /**
     * Send given Ack frame to specified address
     */
    virtual void sendAck(MACFrameBase *, MACAddress addr);

    /**
     * Send ACK message for CSMA
     */
    virtual void sendCSMAAck(MACAddress addr);

    /**
     * Send ACK message for DSME-GTS
     */
    virtual void sendDSMEAck(MACAddress addr);

    /**
     * Called on reception of dsmeAckFrame. If matches last sent message, remove from queue.
     */
    virtual void handleDSMEAck(IEEE802154eMACFrame *ack);

    /**
     * @Override
     */
    virtual void handleBroadcastAck(CSMAFrame *ack, CSMAFrame *frame);

    /**
     * Directly send packet without delay and without CSMA
     */
    virtual void sendDirect(cPacket *);

    /**
     * Called on start of every GTSlot.
     * Switch channel for reception or transmit from queue in allocated slots.
     */
    virtual void handleGTS();

    /**
     * Called on reception of a GTS frame. Send Ack and send payload to upper layer.
     */
    virtual void handleGTSFrame(IEEE802154eMACFrame *);

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

    /**
     * Called every slot to display node status in GUI
     */
    virtual void updateDisplay();

    /**
     * Switch Transceiver to given channel [11, 26]
     * TODO this should be done in the Radio class
     */
    virtual void setChannelNumber(unsigned k);

};

} //namespace

#endif
