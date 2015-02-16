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
#include "inet/linklayer/ieee802154e/IEEE802154eMACCmdFrame_m.h"
#include "inet/linklayer/ieee802154e/DSMEBeaconAllocationNotificationCmd_m.h"

namespace inet {

Define_Module(DSME);

DSME::DSME() :
                        beaconFrame(nullptr),
                        beaconTimer(nullptr)
{
}

DSME::~DSME() {
    cancelAndDelete(beaconTimer);
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

        // DSME configuration
        superframeSpec.beaconOrder = par("beaconOrder");
        superframeSpec.superframeOrder = par("superframeOrder");
        superframeSpec.finalCAPSlot = par("finalCAPSlot");
        dsmeSuperframeSpec.multiSuperframeOrder = par("multiSuperframeOrder");

        baseSuperframeDuration = par("slotsPerSuperframe").longValue() * par("symbolsPerSlot").longValue() * par("symbolsPerSecond").doubleValue();
        beaconInterval = baseSuperframeDuration * (1 << superframeSpec.beaconOrder);
        numberSuperframes = (1 << (superframeSpec.beaconOrder - superframeSpec.superframeOrder));   // 2^(BO-SO)
        EV_DEBUG << "Beacon Interval: " << beaconInterval << " #SFrames: " << numberSuperframes << endl;

        // Beacon management
        if (isCoordinator) {
            beaconTimer = new cMessage("beacon-timer");
        }

    } else if (stage == INITSTAGE_LINK_LAYER) {
        // PAN Coordinator starts network with beacon
        if (isPANCoordinator) {
            beaconAllocation.SDIndex = 0;
            beaconAllocation.SDBitmapLength = numberSuperframes;
            beaconAllocation.SDBitmap.appendBit(true);
            beaconAllocation.SDBitmap.appendBit(false, numberSuperframes-1);

            PANDescriptor.setBeaconBitmap(beaconAllocation);
            PANDescriptor.setBitLength(92 + numberSuperframes);

            scheduleAt(simTime() + beaconInterval, beaconTimer);
        }
    }
}

void DSME::handleSelfMessage(cMessage *msg) {
    if (msg == beaconTimer) {
        sendEnhancedBeacon();
    }
    else {
        // TODO reschedule any sending actions to beginning of CSMA slot
        // TODO also handleUpperLayer
        CSMA::handleSelfMessage(msg);
    }
}

void DSME::handleLowerPacket(cPacket *msg) {
    IEEE802154eMACFrame_Base *macPkt = static_cast<IEEE802154eMACFrame_Base *>(msg);
    const MACAddress& dest = macPkt->getDestAddr();

    if(dest.isBroadcast()) {
        if(strcmp(macPkt->getName(), EnhancedBeacon::NAME) == 0) {
            EnhancedBeacon *beacon = static_cast<EnhancedBeacon *>(msg);
            return handleEnhancedBeacon(beacon);
        } else if(strcmp(macPkt->getName(), "beacon-allocation-notification") == 0) { // TODO const
            EV_DEBUG << "HURRAY THERE IS A NEW COORDINATOR" << endl;
        }
    }

    // if not handled yet handle with CSMA
    CSMA::handleLowerPacket(msg);
}

void DSME::sendDirect(cPacket *msg) {
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    attachSignal(msg, simTime() + aTurnaroundTime); // TODO turnaroundTime only on statechange, plus parameter is useless?
    sendDown(msg);
}

void DSME::sendCSMA(IEEE802154eMACFrame_Base *msg) {
    setUpControlInfo(msg, msg->getDestAddr());
    /*IMACProtocolControlInfo *cInfo = new IMACProtocolControlInfo();
    cInfo->setDestinationAddress(msg->getDestAddr());
    msg->setControlInfo(cInfo);*/
    CSMA::handleUpperPacket(msg);
}

void DSME::sendEnhancedBeacon() {
    if (beaconFrame != nullptr)
        delete beaconFrame;
    beaconFrame = new EnhancedBeacon();
    DSME_PANDescriptor *descr = new DSME_PANDescriptor(PANDescriptor);
    beaconFrame->encapsulate(descr);
    EV_DEBUG << "Send EnhancedBeacon, bitmap: " << descr->getBeaconBitmap().SDBitmapLength << endl;
    sendDirect(beaconFrame);
    beaconFrame = nullptr;
    // schedule next beacon
    scheduleAt(simTime() + beaconInterval, beaconTimer);
}


void DSME::handleEnhancedBeacon(EnhancedBeacon *beacon) {
    DSME_PANDescriptor *descr = static_cast<DSME_PANDescriptor*>(beacon->decapsulate());
    EV_DEBUG << "Received EnhancedBeacon: " << descr->getBeaconBitmap().SDBitmapLength << endl;

    // Coordinator device request free beacon slots
    if (isCoordinator && !isAssociated) {
        // Lookup free slot and broadcast beacon allocation request
        int i = descr->getBeaconBitmap().getFreeSlot();
        EV_DEBUG << "Coordinator Beacon Allocation request @ " << i << endl;
        if (i >= 0) {
            DSMEBeaconAllocationNotificationCmd *cmd = new DSMEBeaconAllocationNotificationCmd();
            cmd->setBeaconSDIndex(i);
            IEEE802154eMACCmdFrame *beaconAllocCmd = new IEEE802154eMACCmdFrame("beacon-allocation-notification");
            beaconAllocCmd->setCmdId(DSME_BEACON_ALLOCATION_NOTIFICATION);
            beaconAllocCmd->encapsulate(cmd);
            beaconAllocCmd->setDestAddr(MACAddress::BROADCAST_ADDRESS);
            EV_DEBUG << "Sending request: " << beaconAllocCmd->getCmdId() << ": " << cmd->getBeaconSDIndex() << endl;
            sendCSMA(beaconAllocCmd);
        }
    }
}

} //namespace
