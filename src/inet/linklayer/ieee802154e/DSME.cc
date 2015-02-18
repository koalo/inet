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

#include "inet/linklayer/ieee802154e/DSME.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/linklayer/ieee802154e/DSMEBeaconAllocationNotificationCmd_m.h"

namespace inet {

Define_Module(DSME);

DSME::DSME() :
                        beaconFrame(nullptr),
                        csmaFrame(nullptr),
                        beaconIntervalTimer(nullptr),
                        nextSlotTimer(nullptr),
                        nextCSMASlotTimer(nullptr),
                        nextGTSlotTimer(nullptr)
{
}

DSME::~DSME() {
    cancelAndDelete(beaconIntervalTimer);
    if (beaconFrame)
        delete beaconFrame;
}


void DSME::initialize(int stage)
{
    cModule *host = getParentModule()->getParentModule();

    CSMA::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        // device specific
        isPANCoordinator = host->par("isPANCoordinator");
        isCoordinator = host->par("isCoordinator");
        isAssociated = isPANCoordinator;
        isBeaconAllocated = isPANCoordinator;

        // DSME configuration
        superframeSpec.beaconOrder = par("beaconOrder");
        superframeSpec.superframeOrder = par("superframeOrder");
        superframeSpec.finalCAPSlot = par("finalCAPSlot");
        dsmeSuperframeSpec.multiSuperframeOrder = par("multiSuperframeOrder");

        double baseSlotDuration = par("baseSlotDuration").longValue() * par("secondsPerSymbol").doubleValue();
        slotDuration = baseSlotDuration * (1 << superframeSpec.superframeOrder);                    // IEEE802.15.4e-2012 p. 37
        slotsPerSuperframe = par("slotsPerSuperframe");
        numCSMASlots = par("numCSMASlots").longValue();
        baseSuperframeDuration = slotsPerSuperframe * baseSlotDuration;
        superframeDuration = slotDuration * slotsPerSuperframe;
        beaconInterval = baseSuperframeDuration * (1 << superframeSpec.beaconOrder);
        numberSuperframes = (1 << (superframeSpec.beaconOrder - superframeSpec.superframeOrder));   // 2^(BO-SO)

        EV_DEBUG << "Beacon Interval: " << beaconInterval << " = #SFrames: " << numberSuperframes << " * '#Slots: ";
        EV << slotsPerSuperframe << " * SlotDuration: " << slotDuration;
        EV << "; baseSuperframeDuration: " << baseSuperframeDuration << ", superframeDuration: " << superframeDuration << endl;

        // Beacon management:
        // PAN Coordinator sends beacon every beaconInterval
        // other devices scan for beacons for that duration
        beaconIntervalTimer = new cMessage("beacon-timer");
        scheduleAt(simTime() + beaconInterval, beaconIntervalTimer);

        // slot scheduling
        currentSlot = 0;
        nextSlotTimer = new cMessage("next-slot-timer");
        nextCSMASlotTimer = new cMessage("csma-slot-timer");
        nextGTSlotTimer = new cMessage("gts-timer");
        scheduleAt(simTime() + slotDuration, nextSlotTimer);


    } else if (stage == INITSTAGE_LINK_LAYER) {
        // PAN Coordinator starts network with beacon
        if (isPANCoordinator) {
            beaconAllocation.SDIndex = 0;
            beaconAllocation.SDBitmapLength = numberSuperframes;
            beaconAllocation.SDBitmap.appendBit(true);
            beaconAllocation.SDBitmap.appendBit(false, numberSuperframes-1);

            PANDescriptor.setBeaconBitmap(beaconAllocation);
            PANDescriptor.setBitLength(92 + numberSuperframes);

        } else if (isCoordinator) {
            beaconAllocation.SDBitmap.appendBit(false, numberSuperframes);
            PANDescriptor.setBeaconBitmap(beaconAllocation);
            PANDescriptor.setBitLength(92 + numberSuperframes);
        }

        // others scan network for beacons and remember beacon allocation included in enhanced beacons
        heardBeacons.SDBitmap.appendBit(false, numberSuperframes);
        neighborHeardBeacons.SDBitmap.appendBit(false, numberSuperframes);
    }
}

void DSME::handleSelfMessage(cMessage *msg) {
    if (msg == nextSlotTimer) {
        //EV_DEBUG << "Current Slot: " << currentSlot << endl;
        currentSlot = (currentSlot < slotsPerSuperframe) ? currentSlot + 1 : 0;
        scheduleAt(simTime() + slotDuration, nextSlotTimer);
    } else if (msg == beaconIntervalTimer) {
        // PAN Coordinator sends beacon every beaconInterval
        // Coordinators send beacons after allocating a slot
        if (isBeaconAllocated)
            sendEnhancedBeacon();

        // Unassociated devices have scanned for beaconInterval and may have heard beacons
        else if (!isAssociated)
            endChannelScan();
    }
    else if (msg == nextCSMASlotTimer) {
        // Slotted CSMA, send at beginning of next CSMA Slot
        if (csmaFrame != nullptr)
            sendCSMA(csmaFrame);
    } else if (msg == nextGTSlotTimer) {
        // TODO
    } else {
        // TODO reschedule any sending actions to beginning of CSMA slot
        // TODO also handleUpperLayer
        CSMA::handleSelfMessage(msg);
    }
}

void DSME::handleLowerPacket(cPacket *msg) {
    IEEE802154eMACFrame_Base *macPkt = static_cast<IEEE802154eMACFrame_Base *>(msg);
    const MACAddress& dest = macPkt->getDestAddr();

    //EV_DEBUG << "Received " << macPkt->getName() << " bcast:" << dest.isBroadcast() << endl;
    if(dest.isBroadcast()) {
        if(strcmp(macPkt->getName(), EnhancedBeacon::NAME) == 0) {
            EnhancedBeacon *beacon = static_cast<EnhancedBeacon *>(msg);
            return handleEnhancedBeacon(beacon);
        } else if(strcmp(macPkt->getName(), "beacon-allocation-notification") == 0) { // TODO const
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            return handleBeaconAllocation(macCmd);
        }
    }

    // TODO beaconCollision cancels event and isAlloc

    // if not handled yet handle with CSMA
    CSMA::handleLowerPacket(msg);
}


void DSME::endChannelScan() {
    EV_DETAIL << "EndChannelScan: ";
    // if now beacon was heard -> scan for another beaconinterval
    if (heardBeacons.getAllocatedCount() == 0) {
        EV_DETAIL << "no beacon heard -> continue scanning" << endl;
        scheduleAt(simTime() + beaconInterval, beaconIntervalTimer);
    } else {
        EV_DETAIL << "heard " << heardBeacons.getAllocatedCount() << " beacons so far -> end scan!" << endl;
        // TODO assuming associated without sending association request
        isAssociated = true;
    }
}

void DSME::sendGTS(IEEE802154eMACFrame_Base *msg) {

}

void DSME::sendDirect(cPacket *msg) {
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    attachSignal(msg, simTime() + aTurnaroundTime); // TODO turnaroundTime only on statechange, plus parameter is useless?
    sendDown(msg);
}

void DSME::sendSlottedCSMA(IEEE802154eMACFrame_Base *msg) {
    // get next slot in superframe i or get next superframe
    unsigned nextCSMASlot = (currentSlot < numCSMASlots) ? currentSlot + 1 : 1;     // beacon @ slot 0
    simtime_t nextCSMASlotTime = lastHeardBeaconTimestamp + nextCSMASlot*slotDuration;
    EV_DETAIL << "SendSlottedCSMA @ " << nextCSMASlot << " => " << nextCSMASlotTime << endl;
    //int32_t i = heardBeacons.getNextAllocated(lastBeaconSDIndex);
    scheduleAt(nextCSMASlotTime, nextCSMASlotTimer);
    csmaFrame = msg;
}

void DSME::sendCSMA(IEEE802154eMACFrame_Base *msg) {
    headerLength = 0;                               // otherwise CSMA adds 72 bits
    useMACAcks = false;                             // handle ACKs ourself
    // simulate upperlayer dest addr to CSMA
    SimpleLinkLayerControlInfo *const cCtrlInfo = new SimpleLinkLayerControlInfo();
    cCtrlInfo->setDest(msg->getDestAddr());
    msg->setControlInfo(cCtrlInfo);
    CSMA::handleUpperPacket(msg);
}

void DSME::sendEnhancedBeacon() {
    if (beaconFrame != nullptr)
        delete beaconFrame;
    beaconFrame = new EnhancedBeacon();
    DSME_PANDescriptor *descr = new DSME_PANDescriptor(PANDescriptor);
    descr->getTimeSyncSpec().beaconTimestamp = simTime();
    beaconFrame->encapsulate(descr);

    // schedule next beacon
    scheduleAt(simTime() + beaconInterval, beaconIntervalTimer);

    EV_DETAIL << "Send EnhancedBeacon, bitmap: " << descr->getBeaconBitmap().SDBitmapLength << endl;
    sendDirect(beaconFrame);
    beaconFrame = nullptr;

    // reschedule nextSlotTimer, otherwise rounding error!?
    cancelEvent(nextSlotTimer);
    scheduleAt(simTime() + slotDuration, nextSlotTimer);
}


void DSME::handleEnhancedBeacon(EnhancedBeacon *beacon) {
    DSME_PANDescriptor *descr = static_cast<DSME_PANDescriptor*>(beacon->decapsulate());
    lastHeardBeaconTimestamp = descr->getTimeSyncSpec().beaconTimestamp;
    lastHeardBeaconSDIndex = descr->getBeaconBitmap().SDIndex;
    EV_DETAIL << "Received EnhancedBeacon @ " << lastHeardBeaconSDIndex << "(" << lastHeardBeaconTimestamp << ")" << endl;

    // time sync -> reschedule nextSlotTimer
    EV_DETAIL << "Timesync, next slot now @ " << lastHeardBeaconTimestamp + slotDuration << endl;
    cancelEvent(nextSlotTimer);
    scheduleAt(lastHeardBeaconTimestamp + slotDuration, nextSlotTimer);

    // update heardBeacons and neighborHeardBeacons
    heardBeacons.SDBitmap.setBit(descr->getBeaconBitmap().SDIndex, true);
    neighborHeardBeacons.SDBitmap |= descr->getBeaconBitmap().SDBitmap;
    EV_DEBUG << "heardBeacons: " << heardBeacons.getAllocatedCount() << ", ";
    EV << "neighborsBeacons: " << neighborHeardBeacons.getAllocatedCount() << endl;

    // Coordinator device request free beacon slots
    if (isCoordinator && !isBeaconAllocated) {
        // Lookup free slot and broadcast beacon allocation request
        int32_t i = descr->getBeaconBitmap().getFreeSlot();
        EV_DEBUG << "Coordinator Beacon Allocation request @ " << i << endl;
        if (i >= 0) {
            sendBeaconAllocationNotification(i);
        }
    }
}

void DSME::sendBeaconAllocationNotification(uint16_t beaconSDIndex) {
    IEEE802154eMACCmdFrame *beaconAllocCmd = new IEEE802154eMACCmdFrame("beacon-allocation-notification");
    DSMEBeaconAllocationNotificationCmd *cmd = new DSMEBeaconAllocationNotificationCmd("beacon-allocation-notification-payload");
    cmd->setBeaconSDIndex(beaconSDIndex);
    beaconAllocCmd->setCmdId(DSME_BEACON_ALLOCATION_NOTIFICATION);
    beaconAllocCmd->encapsulate(cmd);
    beaconAllocCmd->setDestAddr(MACAddress::BROADCAST_ADDRESS);
    //simtime_t nextSlotStart = lastBeaconTimestamp + slotDuration;
    EV_DEBUG << "Sending beaconAllocationRequest @ " << cmd->getBeaconSDIndex();

    //scheduleAt(nextSlotStart, nextCSMASlotTimer);   // TODO let sendCSMA decide
    sendSlottedCSMA(beaconAllocCmd);

    // Update PANDDescrition
    PANDescriptor.getBeaconBitmap().SDIndex = beaconSDIndex;
    isBeaconAllocated = true;

    // schedule BeaconInterval to allocated slot
    cancelEvent(beaconIntervalTimer);
    int16_t superframeOffset = (beaconSDIndex > lastHeardBeaconSDIndex)
            ? beaconSDIndex - lastHeardBeaconSDIndex
            : lastHeardBeaconSDIndex - beaconSDIndex;
    simtime_t beaconStartTime = lastHeardBeaconTimestamp + superframeDuration * superframeOffset;
    scheduleAt(beaconStartTime, beaconIntervalTimer);
    EV << ", SFoffset: " << superframeOffset << " => beacon @ " << beaconStartTime << endl;
}

void DSME::handleBeaconAllocation(IEEE802154eMACCmdFrame *macCmd) {
    // TODO check beacon slot allocation, notify if collision
    DSMEBeaconAllocationNotificationCmd *beaconAlloc = static_cast<DSMEBeaconAllocationNotificationCmd*>(macCmd->decapsulate());
    EV_DEBUG << "HURRAY THERE IS A NEW COORDINATOR @ " << beaconAlloc->getBeaconSDIndex() << endl;
    if (heardBeacons.SDBitmap.getBit(beaconAlloc->getBeaconSDIndex())) {
        EV_DETAIL << "Beacon Slot is not free -> collision !" << endl;
    } else {
        heardBeacons.SDBitmap.setBit(beaconAlloc->getBeaconSDIndex(), true);
        EV_DETAIL << "HeardBeacons: " << heardBeacons.getAllocatedCount() << endl;
        // TODO when to remove heardBeacons in case of collision elsewhere?
    }
}

} //namespace
