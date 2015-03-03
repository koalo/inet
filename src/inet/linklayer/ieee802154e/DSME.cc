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
#include "inet/linklayer/ieee802154e/DSMEBeaconCollisionNotificationCmd_m.h"

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
    cancelAndDelete(nextSlotTimer);
    cancelAndDelete(nextCSMASlotTimer);
    cancelAndDelete(nextGTSlotTimer);
    if (beaconFrame != nullptr)
        delete beaconFrame;
    //if(csmaFrame != nullptr)
    //    delete csmaFrame;     // FIXME this crashes when closing the simulation
    //for (auto it = GTSQueue.begin(); it != GTSQueue.end(); ++it) {
    //    delete (*it);
    //}
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
        isBeaconAllocationSent = false;

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

        // Slotted CSMA
        aUnitBackoffPeriod = slotDuration;
        contentionWindowInit = par("contentionWindow");
        contentionWindow = contentionWindowInit;
        useBrdCstAcks = true;   // DSME GTS management uses broadcast messages with encapsulated dest address

        // GTS
        occupiedGTSs = DSMESlotAllocationBitmap(numberSuperframes, slotsPerSuperframe-numCSMASlots-1, 16); // TODO par channels

        // Beacon management:
        // PAN Coordinator sends beacon every beaconInterval
        // other devices scan for beacons for that duration
        beaconIntervalTimer = new cMessage("beacon-timer");
        scheduleAt(simTime() + beaconInterval, beaconIntervalTimer);

        // slot scheduling
        currentSlot = 0;
        nextSlotTimestamp = simTime() + slotDuration;
        nextSlotTimer = new cMessage("next-slot-timer");
        nextCSMASlotTimer = new cMessage("csma-slot-timer");
        nextGTSlotTimer = new cMessage("gts-timer");
        scheduleAt(nextSlotTimestamp, nextSlotTimer);


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
            beaconAllocation.SDBitmapLength = numberSuperframes;
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
        nextSlotTimestamp = simTime() + slotDuration;
        currentSlot = (currentSlot < slotsPerSuperframe-1) ? currentSlot + 1 : 0;
        // TODO check GTS Slot allocation, switch to channel for RX
        // TODO or transmit from GTSQueue
        // TODO switch back to common channel
        scheduleAt(nextSlotTimestamp, nextSlotTimer);
    } else if (msg == beaconIntervalTimer) {
        // PAN Coordinator sends beacon every beaconInterval
        // Coordinators send beacons after allocating a slot
        if (isBeaconAllocated)
            sendEnhancedBeacon();

        // Unassociated devices have scanned for beaconInterval and may have heard beacons
        else if (!isAssociated)
            endChannelScan();
    } else if (msg == nextGTSlotTimer) {
        // TODO remove?
    } else if (msg == ccaTimer) {
        // slotted CSMA
        // Perform CCA at backoff period boundary, 2 times (contentionWindow), then send at backoff boundary
        EV_DETAIL << "DSME slotted CSMA: TIMER_CCA at backoff period boundary" << endl;
        // backoff period boundary is start time of next slot
        simtime_t nextCSMASlotTimestamp = getNextCSMASlot();
        scheduleAt(nextCSMASlotTimestamp, nextCSMASlotTimer);
    }
    else if (msg == nextCSMASlotTimer) {
        handleCSMASlot();
    } else {
        // TODO handle discared messages, e.g. clear isBeaconAllocationSent

        EV_DEBUG << "HandleSelf CSMA @ slot " << currentSlot << endl;
        CSMA::handleSelfMessage(msg);
    }
}

void DSME::handleLowerPacket(cPacket *msg) {
    IEEE802154eMACFrame *macPkt = static_cast<IEEE802154eMACFrame *>(msg);
    const MACAddress& dest = macPkt->getDestAddr();

    //EV_DEBUG << "Received " << macPkt->getName() << " bcast:" << dest.isBroadcast() << endl;
    if(dest.isBroadcast()) {
        if(strcmp(macPkt->getName(), EnhancedBeacon::NAME) == 0) {
            EnhancedBeacon *beacon = static_cast<EnhancedBeacon *>(msg);
            return handleEnhancedBeacon(beacon);
        } else if(strcmp(macPkt->getName(), "beacon-allocation-notification") == 0) {
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            return handleBeaconAllocation(macCmd);
        } else if(strcmp(macPkt->getName(), "gts-reply-cmd") == 0) {
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            return handleGTSReply(macCmd);
        } else if(strcmp(macPkt->getName(), "gts-notify-cmd") == 0) {
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            return handleGTSNotify(macCmd);
        }
    } else if (dest == address) {
        EV_DETAIL << "DSME received packet for me: " << macPkt->getName() << endl;
        if (strcmp(macPkt->getName(), "beacon-collision-notification") == 0) {
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            return handleBeaconCollision(macCmd);
        } else if (strcmp(macPkt->getName(), "gts-request-cmd") == 0) {
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            return handleGTSRequest(macCmd);
        }
    }

    // if not handled yet handle with CSMA
    EV_DEBUG << "HandleLower CSMA @ slot " << currentSlot << endl;
    CSMA::handleLowerPacket(msg);
}

void DSME::handleUpperPacket(cPacket *msg) {
    EV_DETAIL << "DSME packet from upper layer: ";
    if (!isAssociated) {
        EV << " not associated -> discard packet." << endl;
    } else {
        EV << " send with GTS: ";
        // 1) lookup if slot to destination exist
        // 2) check if queue already contains a message to same destination
        // request a slot if one of the above is true (TODO make it configurable)
        if (GTSQueue.size() == 0) {
            EV << "empty Queue" << endl;
            IMACProtocolControlInfo *const cInfo = check_and_cast<IMACProtocolControlInfo *>(msg->removeControlInfo());
            allocateGTSlots(1, false, cInfo->getDestinationAddress());
        } else {
            EV << "queue not empty: " << GTSQueue.size() << endl;
        }

        // create DSME MAC Packet containing given packet
        // push into queue
        IEEE802154eMACFrame *macFrame = new IEEE802154eMACFrame("dsme-gts-frame");
        macFrame->encapsulate(msg);
        GTSQueue.push_back(macFrame);
    }

}

// See CSMA.cc
void DSME::receiveSignal(cComponent *source, simsignal_t signalID, long value) {
    Enter_Method_Silent();
    if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            if (macState == IDLE_1) { // capture transmission end when sent without CSMA state machine
                EV_DEBUG << "DSME-CSMA: Transmission over" << endl;
                radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
            } else {
                // KLUDGE: we used to get a cMessage from the radio (the identity was not important)
                executeMac(EV_FRAME_TRANSMITTED, new cMessage("Transmission over"));
            }
        }
        transmissionState = newRadioTransmissionState;
    }
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




void DSME::allocateGTSlots(uint8_t numSlots, bool direction, MACAddress addr) {
    // select random slot
    GTS randomGTS = occupiedGTSs.getRandomFreeGTS();  // TODO get for nearby superframe
    // create request command
    DSME_GTS_Management man;
    man.type = ALLOCATION;
    man.direction = direction;
    man.prioritizedChannelAccess = false;
    DSME_SAB_Specification sabSpec;
    sabSpec.subBlockLength = occupiedGTSs.getSubBlockLength();
    sabSpec.subBlockIndex = randomGTS.superframeID;            // TODO subBlockIndex bitwise?!
    sabSpec.subBlock = occupiedGTSs.getSubBlock(randomGTS.superframeID);

    DSME_GTSRequestCmd *req = new DSME_GTSRequestCmd("gts-request-allocation");
    req->setGtsManagement(man); // TODO pointer new?
    req->setSABSpec(sabSpec);   //
    req->setNumSlots(numSlots);
    req->setPreferredSuperframeID(randomGTS.superframeID);
    req->setPreferredSlotID(randomGTS.slotID);

    sendGTSRequest(req, addr);
}

void DSME::sendGTSRequest(DSME_GTSRequestCmd* gtsRequest, MACAddress addr) {
    IEEE802154eMACCmdFrame *macCmd = new IEEE802154eMACCmdFrame("gts-request-cmd");
    macCmd->setCmdId(DSME_GTS_REQUEST);
    macCmd->encapsulate(gtsRequest);
    macCmd->setDestAddr(addr);
    macCmd->setSrcAddr(address);

    EV_DEBUG << "Sending GTS-request To: " << addr << endl;

    sendCSMA(macCmd, true);
}

void DSME::handleGTSRequest(IEEE802154eMACCmdFrame *macCmd) {
    sendACK(macCmd->getSrcAddr());
    DSME_GTSRequestCmd *req = static_cast<DSME_GTSRequestCmd*>(macCmd->decapsulate());
    EV_DETAIL << "Received GTS-request: " << req->getGtsManagement().type;

    DSME_GTSReplyCmd *reply = new DSME_GTSReplyCmd();
    reply->setGtsManagement(req->getGtsManagement());                   // TODO copy?
    reply->setDestinationAddress(macCmd->getSrcAddr());
    DSME_SAB_Specification replySABSpec;

    switch(req->getGtsManagement().type) {
    case ALLOCATION:
        reply->setName("gts-reply-allocation");
        // select numSlots free slots from intersection of received subBlock and local subBlock
        EV << " - ALLOCATION" << endl;
        replySABSpec = occupiedGTSs.allocateSlots(req->getSABSpec(), req->getNumSlots(), req->getPreferredSuperframeID(), req->getPreferredSlotID());
        // TODO handle errors
        reply->getGtsManagement().status = ALLOCATION_APPROVED;
        break;
    default:
        EV_ERROR << "DSME: GTS Mangement Type " << req->getGtsManagement().type << " not supported yet" << endl;
    }

    reply->setSABSpec(replySABSpec);
    sendGTSReply(reply);
}

void DSME::sendGTSReply(DSME_GTSReplyCmd *gtsReply) {
    sendBroadcastCmd("gts-reply-cmd", gtsReply, DSME_GTS_REPLY);
}

void DSME::handleGTSReply(IEEE802154eMACCmdFrame *macCmd) {
    // send ACK if for me
    DSME_GTSReplyCmd *gtsReply = static_cast<DSME_GTSReplyCmd*>(macCmd->decapsulate());
    if (gtsReply->getDestinationAddress() == address) {
        sendACK(macCmd->getSrcAddr());

        // notify neighbors if allocation succeeded
        // TODO also send if not?
        if (gtsReply->getGtsManagement().status == ALLOCATION_APPROVED) {
            EV_DETAIL << "DSME: GTS Allocation succeeded -> notify" << endl;
            DSME_GTSNotifyCmd *gtsNotify = new DSME_GTSNotifyCmd("gts-notify-allocation");
            gtsNotify->setGtsManagement(gtsReply->getGtsManagement());
            gtsNotify->setSABSpec(gtsReply->getSABSpec());
            gtsNotify->setDestinationAddress(macCmd->getSrcAddr());
            sendGTSNotify(gtsNotify);

            //occupiedGTSs.getGTSs(gtsReply->getSABSpec());
        }
    }

    if (gtsReply->getGtsManagement().status == ALLOCATION_APPROVED) {
        occupiedGTSs.updateSlotAllocation(gtsReply->getSABSpec());
    }
}

void DSME::sendGTSNotify(DSME_GTSNotifyCmd *gtsNotify) {
    sendBroadcastCmd("gts-notify-cmd", gtsNotify, DSME_GTS_NOTIFY);
}

void DSME::handleGTSNotify(IEEE802154eMACCmdFrame *macCmd) {
    // send ACK if for me
    DSME_GTSNotifyCmd *gtsNotify = static_cast<DSME_GTSNotifyCmd*>(macCmd->decapsulate());
    if (gtsNotify->getDestinationAddress() == address) {
        sendACK(macCmd->getSrcAddr());
    }
    // update neighbor slot allocation
    EV_DETAIL << "DSME: Received GTS Notify" << endl;
    occupiedGTSs.updateSlotAllocation(gtsNotify->getSABSpec());
}

void DSME::sendGTS(IEEE802154eMACFrame *msg) {

}

void DSME::sendBroadcastCmd(const char *name, cPacket *payload, uint8_t cmdId) {
    IEEE802154eMACCmdFrame *macCmd = new IEEE802154eMACCmdFrame(name);
    macCmd->setCmdId(cmdId);
    macCmd->encapsulate(payload);
    macCmd->setDestAddr(MACAddress::BROADCAST_ADDRESS);
    macCmd->setSrcAddr(address);

    EV_DEBUG << "DSME: Sending " << name << " broadcast" << endl;
    sendCSMA(macCmd, true);
}

void DSME::sendACK(MACAddress addr) {
    if (ackMessage != nullptr)
        delete ackMessage;
    ackMessage = new CSMAFrame("CSMA-Ack");
    ackMessage->setSrcAddr(address);
    ackMessage->setDestAddr(addr);
    ackMessage->setBitLength(ackLength);
    sendDirect(ackMessage);
}

void DSME::handleBroadcastAck(CSMAFrame *ack, CSMAFrame *frame) {
    EV_DEBUG << "Received Ack to Broadcast: " << frame->getName() << endl;
    MACAddress dest;
    if (strcmp(frame->getName(), "gts-reply-cmd") == 0) {
        DSME_GTSReplyCmd *reply = static_cast<DSME_GTSReplyCmd*>(frame->getEncapsulatedPacket()->getEncapsulatedPacket());
        dest = reply->getDestinationAddress();
    } else if (strcmp(frame->getName(), "gts-notify-cmd") == 0) {
        DSME_GTSNotifyCmd *notify = static_cast<DSME_GTSNotifyCmd*>(frame->getEncapsulatedPacket()->getEncapsulatedPacket());
        dest = notify->getDestinationAddress();
    } else {
        return;
    }
    if (dest == ack->getSrcAddr()) {
        nbRecvdAcks++;
        executeMac(EV_ACK_RECEIVED, ack);
    } else {
        EV_DEBUG << "ACK from " << ack->getSrcAddr() << " not from destination " << dest << endl;
    }
}

void DSME::sendDirect(cPacket *msg) {
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    attachSignal(msg, simTime() + aTurnaroundTime); // TODO turnaroundTime only on statechange, plus parameter is useless?
    sendDown(msg);
}

simtime_t DSME::getNextCSMASlot() {
    unsigned slotOffset = (currentSlot < numCSMASlots) ? 0 : slotsPerSuperframe - currentSlot + 1;
    double offset = slotOffset * slotDuration;
    EV_DEBUG << "CurrentSlot: " << currentSlot << ", nextCSMASlot Offset: " << slotOffset << " (" << offset << ")" << endl;
    return nextSlotTimestamp + offset;
}

void DSME::handleCSMASlot() {
    // Slotted CSMA: perform CCA at slot boundary or send directly if channel was idle long enough
    EV_DETAIL << "DSME Slotted CSMA (" << currentSlot << "): ";
    if (contentionWindow > 0) {
        bool isIdle = radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE;
        if (isIdle) {
            // Do this 2 times (default), always at next CSMA slot
            contentionWindow--;
            simtime_t nextCSMASlotTimestamp = getNextCSMASlot();
            if (contentionWindow > 0) {
                EV << "Cotention Window: " << contentionWindow << endl;
                scheduleAt(nextCSMASlotTimestamp + ccaDetectionTime, nextCSMASlotTimer); // TODO timestamp correct?
            } else {
                // then send direct at next slot!
                EV << "Contention Windows where idle -> send next CSMA Slot!" << endl;
                scheduleAt(nextCSMASlotTimestamp, nextCSMASlotTimer);  // TODO timestamp correct?
            }
        } else {
            // backoff again
            EV << "Channel not Idle -> backoff" << endl;
            contentionWindow = contentionWindowInit;
            CSMA::handleSelfMessage(ccaTimer);
        }
    } else {
        EV << "Sending directly at slot boundary!" << endl;
        // TODO dont do this ugly workaround, where CSMA statemachine is exploited
        simtime_t tmp = aTurnaroundTime;
        aTurnaroundTime = 0;
        CSMA::handleSelfMessage(ccaTimer);
        aTurnaroundTime = tmp;
        // message was sent, do some status updates and reset contentionWindow
        onCSMASent();
        contentionWindow = contentionWindowInit;
    }
}

void DSME::sendCSMA(IEEE802154eMACFrame *msg, bool requestACK = false) {
    headerLength = 0;                               // otherwise CSMA adds 72 bits
    useMACAcks = requestACK;
    // simulate upperlayer dest addr to CSMA
    SimpleLinkLayerControlInfo *const cCtrlInfo = new SimpleLinkLayerControlInfo();
    cCtrlInfo->setDest(msg->getDestAddr());
    cCtrlInfo->setSrc(this->address);
    msg->setControlInfo(cCtrlInfo);
    CSMA::handleUpperPacket(msg);
}

void DSME::onCSMASent() {
    EV_DEBUG << "DSME onCSMASent: ";
    if (isCoordinator && isBeaconAllocationSent) {
        EV << "Coordinator now assumes beacon is allocated" << endl;
        isBeaconAllocated = true;
        isBeaconAllocationSent = false;
        // beacon interval is already scheduled
    } else {
        EV << "nothing to do" << endl;
    }
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

    EV_DETAIL << "Send EnhancedBeacon, bitmap: " << descr->getBeaconBitmap().SDBitmapLength;
    EV << ": " << descr->getBeaconBitmap().SDBitmap.toString() << endl;
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
    if (isCoordinator) {
        PANDescriptor.getBeaconBitmap().SDBitmap.setBit(descr->getBeaconBitmap().SDIndex, true);
    }

    EV_DEBUG << "heardBeacons: " << heardBeacons.getAllocatedCount() << ": " << heardBeacons.SDBitmap.toString() << ", ";
    EV << "neighborsBeacons: " << neighborHeardBeacons.getAllocatedCount() << ": " << neighborHeardBeacons.SDBitmap.toString() << endl;

    // Coordinator device request free beacon slots
    if (isCoordinator && !isBeaconAllocated && !isBeaconAllocationSent) {
        // Lookup free slot within all neighbors and broadcast beacon allocation request
        BeaconBitmap allBeacons = heardBeacons;
        allBeacons.SDBitmap |= neighborHeardBeacons.SDBitmap;
        int32_t i = allBeacons.getRandomFreeSlot();
        if (i >= 0) {
            EV_DEBUG << "Coordinator Beacon Allocation request @ random=" << i << endl;
            sendBeaconAllocationNotification(i);
        }
    }

    // TODO if isAllocationsent and allocated Index now is allocated -> cancel!
}

void DSME::sendBeaconAllocationNotification(uint16_t beaconSDIndex) {
    IEEE802154eMACCmdFrame *beaconAllocCmd = new IEEE802154eMACCmdFrame("beacon-allocation-notification");
    DSMEBeaconAllocationNotificationCmd *cmd = new DSMEBeaconAllocationNotificationCmd("beacon-allocation-notification-payload");
    cmd->setBeaconSDIndex(beaconSDIndex);
    beaconAllocCmd->setCmdId(DSME_BEACON_ALLOCATION_NOTIFICATION);
    beaconAllocCmd->encapsulate(cmd);
    beaconAllocCmd->setSrcAddr(address);
    beaconAllocCmd->setDestAddr(MACAddress::BROADCAST_ADDRESS);

    EV_DEBUG << "Sending beaconAllocationRequest @ " << cmd->getBeaconSDIndex() << endl;

    sendCSMA(beaconAllocCmd);
    isBeaconAllocationSent = true;

    // Update PANDDescrition
    PANDescriptor.getBeaconBitmap().SDIndex = beaconSDIndex;
    PANDescriptor.getBeaconBitmap().SDBitmap = heardBeacons.SDBitmap;      // TODO remove allocationbitmap?

    // schedule BeaconInterval to allocated slot
    cancelEvent(beaconIntervalTimer);
    int16_t superframeOffset = (beaconSDIndex > lastHeardBeaconSDIndex)
            ? beaconSDIndex - lastHeardBeaconSDIndex
            : lastHeardBeaconSDIndex - beaconSDIndex;
    simtime_t beaconStartTime = lastHeardBeaconTimestamp + superframeDuration * superframeOffset;
    scheduleAt(beaconStartTime, beaconIntervalTimer);
    EV_DEBUG << "BeaconAlloc SFoffset: " << superframeOffset << " => beacon @ " << beaconStartTime << endl;
}

void DSME::handleBeaconAllocation(IEEE802154eMACCmdFrame *macCmd) {
    // TODO check if self has sent allocation to same slot -> abort CSMA transmission
    DSMEBeaconAllocationNotificationCmd *beaconAlloc = static_cast<DSMEBeaconAllocationNotificationCmd*>(macCmd->decapsulate());
    EV_DEBUG << "HURRAY THERE IS A NEW COORDINATOR @ " << beaconAlloc->getBeaconSDIndex() << endl;
    if (heardBeacons.SDBitmap.getBit(beaconAlloc->getBeaconSDIndex())) {
        EV_DETAIL << "Beacon Slot is not free -> collision !" << endl;
        sendBeaconCollisionNotification(beaconAlloc->getBeaconSDIndex(), macCmd->getSrcAddr());
    } else {
        heardBeacons.SDBitmap.setBit(beaconAlloc->getBeaconSDIndex(), true);
        EV_DETAIL << "HeardBeacons: " << heardBeacons.getAllocatedCount() << endl;
        // TODO when to remove heardBeacons in case of collision elsewhere?
    }

    // TODO Coordinator which has sent allocation notification to same slot should cancel if possible

}

void DSME::sendBeaconCollisionNotification(uint16_t beaconSDIndex, MACAddress addr) {
    IEEE802154eMACCmdFrame *beaconCollisionCmd = new IEEE802154eMACCmdFrame("beacon-collision-notification");
    DSMEBeaconCollisionNotificationCmd *cmd = new DSMEBeaconCollisionNotificationCmd("beacon-collision-notification-payload");
    cmd->setBeaconSDIndex(beaconSDIndex);
    beaconCollisionCmd->setCmdId(DSME_BEACON_COLLISION_NOTIFICATION);
    beaconCollisionCmd->encapsulate(cmd);
    beaconCollisionCmd->setDestAddr(addr);

    EV_DEBUG << "Sending beaconCollisionNotification @ " << cmd->getBeaconSDIndex() << " To: " << addr << endl;

    sendCSMA(beaconCollisionCmd, true);
}

void DSME::handleBeaconCollision(IEEE802154eMACCmdFrame *macCmd) {
    DSMEBeaconCollisionNotificationCmd *cmd = static_cast<DSMEBeaconCollisionNotificationCmd*>(macCmd->decapsulate());
    EV_DETAIL << "DSME handleBeaconCollision: removing beacon schedule @ " << cmd->getBeaconSDIndex();
    isBeaconAllocated = false;
    neighborHeardBeacons.SDBitmap.setBit(cmd->getBeaconSDIndex(), true);
}


} //namespace
