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
#include "inet/physicallayer/ieee802154/Ieee802154NarrowbandScalarReceiver.h"
#include "inet/physicallayer/ieee802154/Ieee802154NarrowbandScalarTransmitter.h"
#include <exception>
namespace inet {

Define_Module(DSME);

DSME::DSME() :
        hostModule(nullptr),
        lastSendGTSFrame(nullptr),
        beaconFrame(nullptr),
        dsmeAckFrame(nullptr),
        beaconIntervalTimer(nullptr),
        preNextSlotTimer(nullptr),
        nextSlotTimer(nullptr),
        nextCSMASlotTimer(nullptr),
        resetGtsAllocationSent(nullptr)
{
}

DSME::~DSME() {
    cancelAndDelete(beaconIntervalTimer);
    cancelAndDelete(preNextSlotTimer);
    cancelAndDelete(nextSlotTimer);
    cancelAndDelete(nextCSMASlotTimer);
    cancelAndDelete(resetGtsAllocationSent);
    if (beaconFrame != nullptr)
        delete beaconFrame;
    for (auto dest = GTSQueue.begin(); dest != GTSQueue.end(); ++dest) {
        for (auto frame = dest->second.begin(); frame != dest->second.end(); frame++)
            delete (*frame);
    }
}

void DSME::finish() {
    CSMA::finish();
    recordScalar("numHeardBeacons", heardBeacons.getAllocatedCount());
    recordScalar("numNeighborHeardBeacons", neighborHeardBeacons.getAllocatedCount());
    recordScalar("numBeaconCollision", numBeaconCollision);
    recordScalar("numTxGtsAllocated", numTxGtsAllocated);
    recordScalar("numTxGtsAllocatedReal", getNumAllocatedGTS(GTS::DIRECTION_TX));
    recordScalar("numTxGtsFrames", numTxGtsFrames);
    recordScalar("numRxGtsAllocated", numRxGtsAllocated);
    recordScalar("numRxGtsAllocatedReal", getNumAllocatedGTS(GTS::DIRECTION_RX));
    recordScalar("numRxGtsFrames", numRxGtsFrames);
    recordScalar("numGtsDuplicatedAllocation", numGtsDuplicatedAllocation);
    recordScalar("numGtsDeallocated", numGtsDeallocated);
    recordScalar("numUpperPacketsDroppedFullQueue", numUpperPacketsDroppedFullQueue);
    recordScalar("numAllocationRequestsSent", numAllocationRequestsSent);
    recordScalar("numAllocationDisapproved", numAllocationDisapproved);
    recordScalar("timeFirstAllocationSent", timeFirstAllocationSent);
    recordScalar("timeLastAllocationSent", timeLastAllocationSent);
    recordScalar("timeAssociated", timeAssociated);
    recordScalar("timeLastRxGtsFrame", timeLastRxGtsFrame);
    recordScalar("timeLastTxGtsFrame", timeLastTxGtsFrame);
}

void DSME::initialize(int stage)
{
    hostModule = getParentModule()->getParentModule();

    CSMA::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // statistic
        numBeaconCollision = 0;
        numTxGtsAllocated = 0;
        numRxGtsAllocated = 0;
        numGtsDuplicatedAllocation = 0;
        numGtsDeallocated = 0;
        numTxGtsFrames = 0;
        numRxGtsFrames = 0;
        numUpperPacketsDroppedFullQueue = 0;
        numAllocationRequestsSent = 0;
        numAllocationDisapproved = 0;
        timeFirstAllocationSent = 0.0;
        timeLastAllocationSent = 0.0;
        timeAssociated = 0.0;
        timeLastRxGtsFrame = 0.0;
        timeLastTxGtsFrame = 0.0;

    } else if (stage == INITSTAGE_LINK_LAYER) {

        // device specific
        isPANCoordinator = hostModule->par("isPANCoordinator");
        isCoordinator = hostModule->par("isCoordinator");
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
        numMaxGTSAllocPerDevice = par("maxNumberGTSAllocPerDevice");
        numMaxGTSAllocPerRequest = par("maxNumberGTSAllocPerRequest");
        baseSuperframeDuration = slotsPerSuperframe * baseSlotDuration;
        superframeDuration = slotDuration * slotsPerSuperframe;
        beaconInterval = baseSuperframeDuration * (1 << superframeSpec.beaconOrder);
        numberSuperframes = (1 << (dsmeSuperframeSpec.multiSuperframeOrder - superframeSpec.superframeOrder));
        numberTotalSuperframes = (1 << (superframeSpec.beaconOrder - superframeSpec.superframeOrder));   // 2^(BO-SO)

        EV_DEBUG << "Beacon Interval: " << beaconInterval << " = #SFrames: " << numberSuperframes;
        EV << ", #totalSFrames: " << numberTotalSuperframes << " * #Slots: ";
        EV << slotsPerSuperframe << " * SlotDuration: " << slotDuration;
        EV << "; baseSuperframeDuration: " << baseSuperframeDuration << ", superframeDuration: " << superframeDuration << endl;

        // Slotted CSMA
        aUnitBackoffPeriod = slotDuration;
        contentionWindowInit = par("contentionWindow");
        contentionWindow = contentionWindowInit;
        useBrdCstAcks = true;   // DSME GTS management uses broadcast messages with encapsulated dest address

        // GTS
        unsigned gtsPerSuperframe = slotsPerSuperframe-numCSMASlots-1;
        occupiedGTSs = DSMESlotAllocationBitmap(numberSuperframes, gtsPerSuperframe, 16); // TODO par channels
        allocatedGTSs.insert(allocatedGTSs.begin(), numberSuperframes, std::vector<GTS>(gtsPerSuperframe, GTS::UNDEFINED));
        gtsAllocationSent = false;

        // Beacon management:
        // PAN Coordinator sends beacon every beaconInterval
        // other devices scan for beacons for that duration
        beaconIntervalTimer = new cMessage("beacon-timer");
        scheduleAt(simTime() + beaconInterval, beaconIntervalTimer);

        // slot scheduling
        currentSlot = 0;
        currentSuperframe = 0;
        currentChannel = 11;        // TODO config common channel
        nextSlotTimestamp = simTime() + slotDuration;
        preNextSlotTimer = new cMessage("pre-next-slot-timer");
        nextSlotTimer = new cMessage("next-slot-timer");
        nextCSMASlotTimer = new cMessage("csma-slot-timer");
        resetGtsAllocationSent = new cMessage("reset-gts-allocation-sent");
        scheduleAt(nextSlotTimestamp, nextSlotTimer);

        // PAN Coordinator starts network with beacon
        if (isPANCoordinator) {
            beaconAllocation.SDIndex = 0;
            beaconAllocation.SDBitmapLength = numberTotalSuperframes;
            beaconAllocation.SDBitmap.appendBit(true);
            beaconAllocation.SDBitmap.appendBit(false, numberTotalSuperframes-1);

            PANDescriptor.setBeaconBitmap(beaconAllocation);
            PANDescriptor.setBitLength(PANDescriptor.getBitLength() + numberTotalSuperframes);

        } else if (isCoordinator) {
            beaconAllocation.SDBitmapLength = numberTotalSuperframes;
            beaconAllocation.SDBitmap.appendBit(false, numberTotalSuperframes);
            PANDescriptor.setBeaconBitmap(beaconAllocation);
            PANDescriptor.setBitLength(PANDescriptor.getBitLength() + numberTotalSuperframes);
        }

        // common channel for all
        setChannelNumber(currentChannel);

        // others scan network for beacons and remember beacon allocation included in enhanced beacons
        heardBeacons.SDBitmap.appendBit(false, numberTotalSuperframes);
        neighborHeardBeacons.SDBitmap.appendBit(false, numberTotalSuperframes);
    }
}

void DSME::handleSelfMessage(cMessage *msg) {
    if (msg == preNextSlotTimer) {
        // Switch to channel for next slot
        switchToNextSlotChannelAndRadioMode();
    }
    else if (msg == nextSlotTimer) {
        // update current slot and superframe information
        nextSlotTimestamp = simTime() + slotDuration;
        currentSlot = (currentSlot < slotsPerSuperframe-1) ? currentSlot + 1 : 0;
        if (currentSlot == 0)
            currentSuperframe = (currentSuperframe < numberSuperframes-1) ? currentSuperframe + 1: 0;
        scheduleAt(nextSlotTimestamp-sifs, preNextSlotTimer);
        scheduleAt(nextSlotTimestamp, nextSlotTimer);

        if (ev.isGUI())
            updateDisplay();

        // if is GTS handle RX/TX if allocated
        if (currentSlot > numCSMASlots) {
            handleGTS();
        }
    }
    else if (msg == beaconIntervalTimer) {
        // Unassociated devices have scanned for beaconInterval and may have heard beacons
        if (!isAssociated)
            endChannelScan();

        // PAN Coordinator sends beacon every beaconInterval
        // Coordinators send beacons after allocating a slot
        if (isBeaconAllocated)
            sendEnhancedBeacon();
    }
    else if (msg == ccaTimer) {
        // slotted CSMA
        // Perform CCA at backoff period boundary, 2 times (contentionWindow), then send at backoff boundary
        EV_DETAIL << "DSME slotted CSMA: TIMER_CCA at backoff period boundary" << endl;
        // backoff period boundary is start time of next slot
        simtime_t nextCSMASlotTimestamp = getNextCSMASlot();
        scheduleAt(nextCSMASlotTimestamp, nextCSMASlotTimer);
    }
    else if (msg == nextCSMASlotTimer) {
        handleCSMASlot();
    }
    else if (msg == resetGtsAllocationSent) {
        gtsAllocationSent = false;
    }
    else if (msg == backoffTimer) {
        // TODO remove this ugly workaround
        // When sending an ACK and directly send reply using CSMA and backoffTimer = 0
        // CSMA will break the transmission of the ACK.
        // In detail: the endReception() method of the radio will not remove the controlInfo from the timer message
        // Which will cause an error later on sending the next packet: (cMessage)endTransmission: setControlInfo(): message already has control info attached.
        if (radio->getTransmissionState() == IRadio::TRANSMISSION_STATE_TRANSMITTING) {
            EV_WARN << "FIXME: CSMA backofftimer while transmitting -> reschedule in 400us (Ack transmit time)" << endl;
            scheduleAt(simTime() + 0.0004, backoffTimer);
        } else {
            EV_DEBUG << "HandleSelf CSMA @ slot " << currentSlot << endl;
            CSMA::handleSelfMessage(msg);
        }
    }
    else {
        // TODO handle discared messages, e.g. clear isBeaconAllocationSent

        EV_DEBUG << "HandleSelf CSMA @ slot " << currentSlot << endl;
        CSMA::handleSelfMessage(msg);
    }
}

void DSME::handleLowerPacket(cPacket *msg) {
    IEEE802154eMACFrame *macPkt = static_cast<IEEE802154eMACFrame *>(msg);
    const MACAddress& dest = macPkt->getDestAddr();

    if(dest.isBroadcast()) {
        if(strcmp(macPkt->getName(), EnhancedBeacon::NAME) == 0) {
            EnhancedBeacon *beacon = static_cast<EnhancedBeacon *>(msg);
            handleEnhancedBeacon(beacon);
        } else if(strcmp(macPkt->getName(), "beacon-allocation-notification") == 0) {
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            handleBeaconAllocation(macCmd);
        } else if(strcmp(macPkt->getName(), "gts-reply-cmd") == 0) {
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            handleGTSReply(macCmd);
        } else if(strcmp(macPkt->getName(), "gts-notify-cmd") == 0) {
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            handleGTSNotify(macCmd);
        } else {
            // if not handled yet handle with CSMA
            EV_DEBUG << "HandleLower Broadcast CSMA @ slot " << currentSlot << endl;
            return CSMA::handleLowerPacket(msg);
        }
    } else if (dest == address) {
        EV_DETAIL << "DSME received packet for me: " << macPkt->getName() << endl;
        if (strcmp(macPkt->getName(), "dsme-gts-frame") == 0) {
            handleGTSFrame(macPkt);
        } else if (strcmp(macPkt->getName(), "dsme-ack") == 0) {
            handleDSMEAck(macPkt);
        } else if (strcmp(macPkt->getName(), "beacon-collision-notification") == 0) {
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            handleBeaconCollision(macCmd);
        } else if (strcmp(macPkt->getName(), "gts-request-cmd") == 0) {
            IEEE802154eMACCmdFrame *macCmd = static_cast<IEEE802154eMACCmdFrame *>(macPkt->decapsulate()); // was send with CSMA
            handleGTSRequest(macCmd);
        } else {
            // TODO dont do this ugly workaround
            // Problem: when receiving CSMA-Ack, but not expecting it, CSMA tries to decapsulate a message which crashes the application
            // The only sendCSMA occurance which does not expect an ACK is the BeaconAllocationRequest.
            // If that gets called after another CSMARequest was executed and CSMA is in Backoff state,
            // this problem may occur.
            if (!useMACAcks && strcmp(msg->getName(), "CSMA-Ack") == 0) {
                useMACAcks = true;
                std::cerr << "CSMA-Ack without expecting it!" << endl;
                CSMA::handleLowerPacket(msg);
                useMACAcks = false;
                return;
            } else {
                // if not handled yet handle with CSMA
                EV_DEBUG << "HandleLower CSMA @ slot " << currentSlot << endl;
                return CSMA::handleLowerPacket(msg);
            }
        }
    }

    delete msg;
}

void DSME::handleUpperPacket(cPacket *msg) {
    // extract destination
    IMACProtocolControlInfo *const cInfo = check_and_cast<IMACProtocolControlInfo *>(msg->removeControlInfo());
    MACAddress dest = cInfo->getDestinationAddress();
    unsigned numPackets = GTSQueue[dest].size();

    EV_DETAIL << "DSME packet from upper layer: ";
    if (!isAssociated) {
        EV << " not associated -> push into queue." << endl;
        // TODO allocate slots after association
    } else {
        EV << " send with GTS: ";
        // 1) lookup if slot to destination exist
        // 2) check if queue already contains a message to same destination
        //  -> request same amount of slots as queue holds packets
        //     up to the maximum allowed per device
        unsigned numSlots = getNumAllocatedGTS(dest, GTS::DIRECTION_TX);
        if (!gtsAllocationSent) {
            if (numSlots == 0 || numSlots < numPackets) {
                EV << "No allocated Slots or less than packets in queue " << numSlots << " / " << numPackets << endl;
                unsigned numSlotsAlloc = numPackets - numSlots;
                EV << "Allocate " << numSlotsAlloc << " of " << numMaxGTSAllocPerDevice << " GTS. ";
                if (numMaxGTSAllocPerDevice < numSlots + numSlotsAlloc) {
                    EV << "Trying to allocate too many slots, reducing to: ";
                    numSlotsAlloc = numMaxGTSAllocPerDevice - numSlots;
                    EV << numSlotsAlloc << endl;
                }
                if (numSlotsAlloc > 0) {
                    numSlotsAlloc = std::min(numMaxGTSAllocPerRequest, numSlotsAlloc);
                    allocateGTSlots(numSlotsAlloc, GTS::DIRECTION_TX, dest);
                }
            } else {
                EV << "Enough allocated Slots to handle queue " << numSlots << " / " << numPackets << endl;
            }
        } else {
            EV << "Slot allocation already sent -> wait for it" << endl;
        }
    }

    // create DSME MAC Packet containing given packet
    if (numPackets < queueLength) {
        // push into queue
        IEEE802154eMACFrame *macFrame = new IEEE802154eMACFrame("dsme-gts-frame");
        macFrame->encapsulate(msg);
        macFrame->setDestAddr(dest);
        macFrame->setSrcAddr(address);
        GTSQueue[dest].push_back(macFrame);
    } else {
        EV_WARN << "DSME upperpacket: queue full" << endl;
        numUpperPacketsDroppedFullQueue++;
        if (ev.isGUI())
            hostModule->bubble("Dropped packet, queue full :(");
        delete msg;
    }
}

// See CSMA.cc
void DSME::receiveSignal(cComponent *source, simsignal_t signalID, long value) {
    Enter_Method_Silent();
    if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            // capture transmission end when sent without CSMA state machine
            if (macState == IDLE_1 || currentSlot == 0 || currentSlot > numCSMASlots) {
                EV_DEBUG << "DSME-(CSMA): Transmission over. @" << currentSlot << " mac:" << macState << endl;
                radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
            } else {
                // KLUDGE: we used to get a cMessage from the radio (the identity was not important)
                executeMac(EV_FRAME_TRANSMITTED, new cMessage("Transmission over"));
            }
        }
        transmissionState = newRadioTransmissionState;
    }
}



void DSME::switchToNextSlotChannelAndRadioMode() {
    // TODO common channel as parameter
    if (currentSlot == slotsPerSuperframe-1)
        setChannelNumber(11);
    else if (currentSlot >= numCSMASlots) {
        unsigned nextGTS = currentSlot - numCSMASlots;
        GTS &gts = allocatedGTSs[currentSuperframe][nextGTS];
        if (gts != GTS::UNDEFINED) {
            setChannelNumber(11 + gts.channel);
            radio->setRadioMode((gts.direction) ? IRadio::RADIO_MODE_RECEIVER : IRadio::RADIO_MODE_TRANSMITTER);
            return;
        }
    }
    // default receive
    radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
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
        timeAssociated = simTime();
        if (ev.isGUI())
            hostModule->bubble("Associated now :)");
    }
}




void DSME::allocateGTSlots(uint8_t numSlots, bool direction, MACAddress addr) {
    // select random slot
    GTS randomGTS = occupiedGTSs.getRandomFreeGTS(allocatedGTSs);  // TODO get for nearby superframe
    if (randomGTS == GTS::UNDEFINED) {
        EV_WARN << "DSME allocateGTS: no free slot found" << endl;
        return;
    }
    //randomGTS.superframeID = 0; // force duplicated allocation
    //randomGTS.slotID = 0;       // force duplicated allocation
    // create request command
    DSME_GTS_Management man;
    man.type = ALLOCATION;
    man.direction = direction;
    man.prioritizedChannelAccess = false;
    DSME_SAB_Specification sabSpec;
    sabSpec.subBlockLength = occupiedGTSs.getSubBlockLength();
    sabSpec.subBlockIndex = randomGTS.superframeID;            // TODO subBlockIndex in units or in bits?!
    sabSpec.subBlock = occupiedGTSs.getSubBlock(randomGTS.superframeID);

    DSME_GTSRequestCmd *req = new DSME_GTSRequestCmd("gts-request-allocation");
    req->setGtsManagement(man);
    req->setSABSpec(sabSpec);
    req->setNumSlots(numSlots);
    req->setPreferredSuperframeID(randomGTS.superframeID);
    req->setPreferredSlotID(randomGTS.slotID);

    sendGTSRequest(req, addr);
    gtsAllocationSent = true;

    numAllocationRequestsSent++;
    if (timeFirstAllocationSent == 0.0)
        timeFirstAllocationSent = simTime();
    timeLastAllocationSent = simTime();
}

void DSME::deallocateGTSlots(DSME_SAB_Specification sabSpec, uint8_t cmd) {
    // check slots to deallocate:
    // is there at least one and do all belong to the same address?
    auto gtss = occupiedGTSs.getGTSsFromAllocation(sabSpec);
    bool foundGts = false;
    bool gtsDifferentAddresses = false;
    if (gtss.size() > 0) {
        MACAddress dest = MACAddress::UNSPECIFIED_ADDRESS;
        for (auto gts = gtss.begin(); gts != gtss.end(); ) {
            GTS allocGts = allocatedGTSs[gts->superframeID][gts->slotID];
            if (allocGts != GTS::UNDEFINED) {
                if (dest == MACAddress::UNSPECIFIED_ADDRESS)
                    dest = allocGts.address;
                else if (dest == allocGts.address)
                    foundGts = true;
                else
                    gtsDifferentAddresses = true;
                gts++;
            } else {
                EV_WARN << "DSME deallocateGTSlots: slot " << gts->superframeID << "/" << gts->slotID << " not allocated" << endl;
                sabSpec.subBlock.setBit(occupiedGTSs.getSubBlockIndex(*gts), false);
                gts = gtss.erase(gts);
            }
        }

        if (gtss.size() < 2 && dest != MACAddress::UNSPECIFIED_ADDRESS)
            foundGts = true;

        if (gtsDifferentAddresses) {
            // TODO handle multiple requests or also send (INVALID_PARAMETER)?!
            EV_WARN << "DSME deallocateGTSlots: slots belong to different addresses!" << endl;
            return;
        } else {
            EV_DETAIL << "DSME deallocatedGTSlots: " << sabSpec.subBlock.toString() << endl;
            DSME_GTS_Management man;
            man.type = DEALLOCATION;
            if (foundGts) {
                man.status = ALLOCATION_APPROVED;   // = SUCCESS
            } else {
                man.status = OTHER_DISAPPROVED;
                EV_WARN << "DSME deallocatedGTSlots: no allocated Slots were found -> DISAPPROVED" << endl;
            }

            switch(cmd) {
            case DSME_GTS_REQUEST:{
                // remove from allocatedGTSs and send request to corresponding device
                removeAllocatedGTS(gtss);
                DSME_GTSRequestCmd *req = new DSME_GTSRequestCmd("gts-request-deallocation");
                req->setGtsManagement(man);
                req->setSABSpec(sabSpec);
                sendGTSRequest(req, dest);
            } break;
            case DSME_GTS_REPLY: {
                // remove from allocatedGTSs and send request to corresponding device
                removeAllocatedGTS(gtss);
                DSME_GTSReplyCmd *reply = new DSME_GTSReplyCmd("gts-reply-deallocation");
                reply->setGtsManagement(man);
                reply->setSABSpec(sabSpec);
                reply->setDestinationAddress(dest);
                sendGTSReply(reply);
            } break;
            default:
                EV_ERROR << "UNKOWN DSME_GTS_CMD: " << (unsigned) cmd << endl;
            }
        }

    } else {
        EV_WARN << "DSME deallocateGTSlots: empty SAB" << endl;
    }
}

unsigned DSME::getNumAllocatedGTS(bool direction) {
    unsigned num = 0;
    for (auto sf = allocatedGTSs.begin(); sf < allocatedGTSs.end(); sf++) {
        for (auto gts = sf->begin(); gts < sf->end(); gts++) {
            if (gts->address != MACAddress::UNSPECIFIED_ADDRESS
                    && gts->direction == direction)
                num++;
        }
    }
    return num;
}

unsigned DSME::getNumAllocatedGTS(MACAddress address, bool direction) {
    unsigned num = 0;
    for (auto sf = allocatedGTSs.begin(); sf < allocatedGTSs.end(); sf++) {
        for (auto gts = sf->begin(); gts < sf->end(); gts++) {
            if (gts->address == address && gts->direction == direction)
                num++;
        }
    }
    return num;
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
    // immediately send ACK
    sendCSMAAck(macCmd->getSrcAddr());

    DSME_GTSRequestCmd *req = static_cast<DSME_GTSRequestCmd*>(macCmd->decapsulate());
    EV_DETAIL << "Received GTS-request: " << req->getGtsManagement().type;

    switch(req->getGtsManagement().type) {
    case ALLOCATION: {
        DSME_GTSReplyCmd *reply = new DSME_GTSReplyCmd();
        reply->setGtsManagement(req->getGtsManagement());
        reply->setDestinationAddress(macCmd->getSrcAddr());
        DSME_SAB_Specification replySABSpec;
        reply->setName("gts-reply-allocation");

        // select numSlots free slots from intersection of received subBlock and local subBlock
        EV << " - ALLOCATION" << endl;
        replySABSpec = occupiedGTSs.allocateSlots(req->getSABSpec(), req->getNumSlots(), req->getPreferredSuperframeID(), req->getPreferredSlotID(), allocatedGTSs);
        if (replySABSpec.subBlock.isZero()) {
            EV_DEBUG << "NOT APPROVED" << endl;
            reply->getGtsManagement().status = ALLOCATION_DISAPPROVED;
        } else {
            EV_DEBUG << "APPROVED" << endl;
            reply->getGtsManagement().status = ALLOCATION_APPROVED;
            // allocate for opposite direction of request
            updateAllocatedGTS(replySABSpec, !reply->getGtsManagement().direction, macCmd->getSrcAddr());

        }

        //EV_WARN << "DUPLICATED DEBUG slot 000"<< endl;
        //replySABSpec.subBlock.setBit(0, true); // force duplicate allocation

        reply->setSABSpec(replySABSpec);
        sendGTSReply(reply);
        break;}
    case DUPLICATED_ALLOCATION_NOTIFICATION:
        EV << " - DUPLICATED_ALLOCATION_NOTIFICATION" << endl;
        deallocateGTSlots(req->getSABSpec(), DSME_GTS_REQUEST);
        break;
    case DEALLOCATION:
        EV << " - DEALLOCATION" << endl;
        deallocateGTSlots(req->getSABSpec(), DSME_GTS_REPLY);
        break;
    default:
        EV_ERROR << "DSME: GTS Mangement Type " << req->getGtsManagement().type << " not supported yet" << endl;
    }

    delete req;
    delete macCmd;
}

void DSME::sendGTSReply(DSME_GTSReplyCmd *gtsReply) {
    sendBroadcastCmd("gts-reply-cmd", gtsReply, DSME_GTS_REPLY);
}

void DSME::updateAllocatedGTS(DSME_SAB_Specification& sabSpec, bool direction, MACAddress address) {
    std::list<GTS> gtss = occupiedGTSs.getGTSsFromAllocation(sabSpec);
    EV_DEBUG << "Allocated slots for " << address << " (" << direction << "): ";
    EV << gtss.size() << " in " << sabSpec.subBlock.toString() << endl;
    for(auto it = gtss.begin(); it != gtss.end(); it++) {
        EV << it->superframeID << "/" << it->slotID;
        if (allocatedGTSs[it->superframeID][it->slotID] == GTS::UNDEFINED) {
            it->direction = direction;
            it->address = address;
            allocatedGTSs[it->superframeID][it->slotID] = *it;
            EV << "@" << (int)it->channel << "  ";
            if (direction)
                numRxGtsAllocated++;
            else
                numTxGtsAllocated++;
        } else {
            EV << " already allocated! ";
            sabSpec.subBlock.setBit(occupiedGTSs.getSubBlockIndex(*it), false);
        }
    }
    EV << endl;
}

void DSME::removeAllocatedGTS(std::list<GTS>& gtss) {
    for (auto gts = gtss.begin(); gts != gtss.end(); gts++) {
        EV_DEBUG << "DSME removing allocated slot: " << gts->superframeID << "/" << gts->slotID << endl;
        allocatedGTSs[gts->superframeID][gts->slotID] = GTS::UNDEFINED;
        numGtsDeallocated++;
    }
}

void DSME::handleGTSReply(IEEE802154eMACCmdFrame *macCmd) {
    DSME_GTSReplyCmd *gtsReply = static_cast<DSME_GTSReplyCmd*>(macCmd->decapsulate());
    MACAddress other = macCmd->getSrcAddr();

    DSME_GTS_Management man = gtsReply->getGtsManagement();

    bool scheduleResetGtsAllocationSent = false;

    if (man.status == ALLOCATION_APPROVED) {
        if (gtsReply->getDestinationAddress() == address) {
            sendCSMAAck(other);

            DSME_GTSNotifyCmd *gtsNotify = new DSME_GTSNotifyCmd("gts-notify-allocation");
            DSME_SAB_Specification& sabSpec = gtsReply->getSABSpec();
            gtsNotify->setGtsManagement(gtsReply->getGtsManagement());
            gtsNotify->setDestinationAddress(macCmd->getSrcAddr());

            if (man.type == ALLOCATION) {
                EV_DETAIL << "DSME: GTS Allocation succeeded -> notify" << endl;
                // allocate GTS
                bool direction = gtsReply->getGtsManagement().direction;
                updateAllocatedGTS(sabSpec, direction, other);
                occupiedGTSs.updateSlotAllocation(sabSpec, man.type);
                scheduleResetGtsAllocationSent = true;
            } else if (man.type == DEALLOCATION) {
                // TODO already removed allocated slots on request, do anything?
            }

            gtsNotify->setSABSpec(sabSpec);
            sendGTSNotify(gtsNotify);
        }

        // neighbors update their SAB (occupiedGTSs)
        else {
            if (man.type == ALLOCATION) {
                checkAndHandleGTSDuplicateAllocation(gtsReply->getSABSpec(), other);
            }

            occupiedGTSs.updateSlotAllocation(gtsReply->getSABSpec(), man.type);
        }
    } else if (man.status == ALLOCATION_DISAPPROVED && gtsReply->getDestinationAddress() == address) {
        numAllocationDisapproved++;
        scheduleResetGtsAllocationSent = true;
        EV_WARN << "DSME: GTS reply with status DISAPPROVED and type " << man.type << endl;
    } else {
        EV_WARN << "DSME: GTS reply other_disaproved or allocation disapproved not for me -> ignored" << endl;
    }

    if (scheduleResetGtsAllocationSent) {
        if (resetGtsAllocationSent->isScheduled())
            cancelEvent(resetGtsAllocationSent);
        scheduleAt(simTime() + beaconInterval, resetGtsAllocationSent);
    }

    delete gtsReply;
    delete macCmd;
}

void DSME::sendGTSNotify(DSME_GTSNotifyCmd *gtsNotify) {
    sendBroadcastCmd("gts-notify-cmd", gtsNotify, DSME_GTS_NOTIFY);
}

void DSME::handleGTSNotify(IEEE802154eMACCmdFrame *macCmd) {
    EV_DETAIL << "DSME: Received GTS Notify" << endl;
    // send ACK if for me
    DSME_GTSNotifyCmd *gtsNotify = static_cast<DSME_GTSNotifyCmd*>(macCmd->decapsulate());
    MACAddress other = macCmd->getSrcAddr();
    DSME_GTS_Management man = gtsNotify->getGtsManagement();
    if (gtsNotify->getDestinationAddress() == address) {
        sendCSMAAck(other);
        if (man.type == ALLOCATION)
            occupiedGTSs.updateSlotAllocation(gtsNotify->getSABSpec(), man.type);
    }
    // update neighbor slot allocation
    else {
        if (man.type == ALLOCATION) {
            checkAndHandleGTSDuplicateAllocation(gtsNotify->getSABSpec(), other);
        }

        occupiedGTSs.updateSlotAllocation(gtsNotify->getSABSpec(), man.type);
    }

    delete gtsNotify;
    delete macCmd;
}

bool DSME::checkAndHandleGTSDuplicateAllocation(DSME_SAB_Specification& sabSpec, MACAddress addr) {
    bool duplicateFound = false;

    DSME_SAB_Specification duplicateSABSpec;
    duplicateSABSpec.subBlockIndex = sabSpec.subBlockIndex;
    duplicateSABSpec.subBlockLength = sabSpec.subBlockLength;
    duplicateSABSpec.subBlock = BitVector(0, sabSpec.subBlock.getSize());

    auto gtss = occupiedGTSs.getGTSsFromAllocation(sabSpec);
    for (auto gts = gtss.begin(); gts != gtss.end(); gts++) {
        if (*gts == allocatedGTSs[gts->superframeID][gts->slotID]) {
            EV_DEBUG << "DSME: GTS duplicate allocation: " << gts->superframeID << "/" << gts->slotID << "@" << gts->channel << endl;
            duplicateFound = true;
            uint16_t index = occupiedGTSs.getSubBlockIndex(*gts);
            duplicateSABSpec.subBlock.setBit(index, true);
            EV << " -> SBIndex=" << index << endl;
        }
    }

    if (duplicateFound) {
        numGtsDuplicatedAllocation++;
        DSME_GTSRequestCmd *dupReq = new DSME_GTSRequestCmd("gts-request-duplication");
        DSME_GTS_Management man;
        man.type = DUPLICATED_ALLOCATION_NOTIFICATION;
        dupReq->setGtsManagement(man);
        dupReq->setSABSpec(duplicateSABSpec);
        sendGTSRequest(dupReq, addr);
    }

    return duplicateFound;
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

void DSME::sendAck(MACFrameBase *msg, MACAddress addr) {
    msg->setSrcAddr(address);
    msg->setDestAddr(addr);
    sendDirect(msg);
}

void DSME::sendCSMAAck(MACAddress addr) {
    if (ackMessage != nullptr)
        delete ackMessage;
    ackMessage = new CSMAFrame("CSMA-Ack");
    ackMessage->setBitLength(ackLength);
    sendAck(ackMessage, addr);
    ackMessage = nullptr;
}

void DSME::sendDSMEAck(MACAddress addr) {
    if (dsmeAckFrame != nullptr)
        delete dsmeAckFrame;
    dsmeAckFrame = new IEEE802154eMACFrame("dsme-ack");
    sendAck(dsmeAckFrame, addr);
    dsmeAckFrame = nullptr;
}

void DSME::handleDSMEAck(IEEE802154eMACFrame *ack) {
    EV_DEBUG << "DSME ACK received" << endl;
    hostModule->bubble("ACK received");
    if (lastSendGTSFrame != nullptr) {
        MACAddress dest = lastSendGTSFrame->getDestAddr();
        if (ack->getSrcAddr() == dest) {
            // also delete, beacuse dup'ed before
            delete lastSendGTSFrame; // TODO check if frame equals frame in queue?
            lastSendGTSFrame = nullptr;
            GTSQueue[dest].pop_front();
            EV_DEBUG << "DSME received expected Ack, removed packet from queue " << dest << ", remaining: " << GTSQueue[dest].size() << endl;
        } else {
            EV_ERROR << "DSME received unexpected Ack (src and dest address differ)" << endl;
        }
    } else {
        EV_ERROR << "DSME received unexpected Ack (did not send anything lately)" << endl;
    }
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
    attachSignal(msg, simTime() + aTurnaroundTime); // TODO turnaroundTime only on mode change, plus parameter is useless?
    sendDown(msg);
}

void DSME::handleGTS() {
    unsigned currentGTS = currentSlot - numCSMASlots - 1;
    GTS &gts = allocatedGTSs[currentSuperframe][currentGTS];
    if (gts != GTS::UNDEFINED) {
        EV_DEBUG << "DSME: handleGTS @ " << currentSuperframe << "/" << currentGTS << ": ";
        EV << "direction:" << gts.direction << " channel:" << (unsigned)gts.channel << endl;

        if (gts.direction) {
            EV << "Waiting to receive from: " << gts.address << endl;
        } else {
            EV << "send to: " << gts.address << " | " << GTSQueue[gts.address].size() << " packets in queue" << endl;
            // transmit from GTSQueue
            if (GTSQueue[gts.address].size() > 0) {
                lastSendGTSFrame = GTSQueue[gts.address].front();
                if (nullptr == lastSendGTSFrame->getOwner()) { // TODO this might happen if ACK sent but not received? How to avoid??
                    EV_ERROR << simTime() << " handleGTS, transmit, queue.front has no owner! removing, queue.size=" << GTSQueue[gts.address].size() << endl;
                    GTSQueue[gts.address].pop_front();
                } else {
                    sendDirect(lastSendGTSFrame->dup());
                    numTxGtsFrames++;
                    timeLastTxGtsFrame = simTime();
                }
            }
        }
    }
}

void DSME::handleGTSFrame(IEEE802154eMACFrame *macPkt) {
    numRxGtsFrames++;
    timeLastRxGtsFrame = simTime();
    hostModule->bubble("GTS received");
    sendDSMEAck(macPkt->getSrcAddr());
    sendUp(decapsMsg(macPkt));
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
                scheduleAt(nextCSMASlotTimestamp + ccaDetectionTime, nextCSMASlotTimer);
            } else {
                // then send direct at next slot!
                EV << "Contention Windows where idle -> send next CSMA Slot!" << endl;
                scheduleAt(nextCSMASlotTimestamp, nextCSMASlotTimer);
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
    headerLength = 0;         // otherwise CSMA adds 72 bits
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
    currentSlot = 0;
    currentSuperframe = lastHeardBeaconSDIndex % numberSuperframes;

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
    delete descr;
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
    PANDescriptor.getBeaconBitmap().SDBitmap = heardBeacons.SDBitmap;

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
    EV_DETAIL << "BeaconAllocation @ " << beaconAlloc->getBeaconSDIndex() << ". ";
    if (heardBeacons.SDBitmap.getBit(beaconAlloc->getBeaconSDIndex())) {
        EV_DETAIL << "Beacon Slot is not free -> collision !" << endl;
        sendBeaconCollisionNotification(beaconAlloc->getBeaconSDIndex(), macCmd->getSrcAddr());
        if (ev.isGUI())
            hostModule->bubble("BeaconAllocCollision");
    } else {
        heardBeacons.SDBitmap.setBit(beaconAlloc->getBeaconSDIndex(), true);
        EV_DETAIL << "HeardBeacons: " << heardBeacons.getAllocatedCount() << endl;
        // TODO when to remove heardBeacons in case of collision elsewhere?
        if (ev.isGUI())
            hostModule->bubble("BeaconAlloc");
    }

    delete beaconAlloc;
    delete macCmd;
}

void DSME::sendBeaconCollisionNotification(uint16_t beaconSDIndex, MACAddress addr) {
    numBeaconCollision++;
    IEEE802154eMACCmdFrame *beaconCollisionCmd = new IEEE802154eMACCmdFrame("beacon-collision-notification");
    DSMEBeaconCollisionNotificationCmd *cmd = new DSMEBeaconCollisionNotificationCmd("beacon-collision-notification-payload");
    cmd->setBeaconSDIndex(beaconSDIndex);
    beaconCollisionCmd->setCmdId(DSME_BEACON_COLLISION_NOTIFICATION);
    beaconCollisionCmd->encapsulate(cmd);
    beaconCollisionCmd->setDestAddr(addr);
    beaconCollisionCmd->setSrcAddr(address);

    EV_DEBUG << "Sending beaconCollisionNotification @ " << cmd->getBeaconSDIndex() << " To: " << addr << endl;

    sendCSMA(beaconCollisionCmd, true);
}

void DSME::handleBeaconCollision(IEEE802154eMACCmdFrame *macCmd) {
    DSMEBeaconCollisionNotificationCmd *cmd = static_cast<DSMEBeaconCollisionNotificationCmd*>(macCmd->decapsulate());
    EV_DETAIL << "DSME handleBeaconCollision: removing beacon schedule @ " << cmd->getBeaconSDIndex() << endl;
    isBeaconAllocated = false;
    neighborHeardBeacons.SDBitmap.setBit(cmd->getBeaconSDIndex(), true);
    sendCSMAAck(macCmd->getSrcAddr());
    delete cmd;
    delete macCmd;
}

void DSME::updateDisplay() {
    char buf[10];
    sprintf(buf, "%02u/%02u/%02u", currentSuperframe, currentSlot, currentChannel);
    hostModule->getDisplayString().setTagArg("t",0,buf);
}


void DSME::setChannelNumber(unsigned k) {
    // see IEEE802.15.4-2011 p. 148 for center frequencies of channels

    // FIXME somehow always need to change channel otherwise reception ignored after a while
    //if (currentChannel == k)
    //    return;
    if (k < 11) {
        EV_ERROR << "Channel number must be greater than 11" << endl;
        k = 11;
    }
    else if (k > 26) {
        EV_ERROR << "Channel number must be less than 26" << endl;
        k = 26;
    }

    currentChannel = k;
    Hz centerFrequency = Hz((2405 + 5 * (k - 11)) * 1e06);

    NarrowbandTransmitterBase *transmitter = const_cast<NarrowbandTransmitterBase *>(check_and_cast<const NarrowbandTransmitterBase *>(radio->getTransmitter()));
    NarrowbandReceiverBase *receiver = const_cast<NarrowbandReceiverBase *>(check_and_cast<const NarrowbandReceiverBase *>(radio->getReceiver()));
    EV << "DSME: changing channel from centerFreq: " << transmitter->getCarrierFrequency() << " to: " << centerFrequency << endl;
    transmitter->setCarrierFrequency(centerFrequency);
    receiver->setCarrierFrequency(centerFrequency);
    //ieee80211Radio also does:
    //endReceptionTimer = nullptr;
    //emit(radioChannelChangedSignal, newChannelNumber);
    emit(radio->listeningChangedSignal, 0);
}

} //namespace
