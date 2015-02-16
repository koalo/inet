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

        // DSME configuration
        superframeSpec.beaconOrder = par("beaconOrder");
        superframeSpec.superframeOrder = par("superframeOrder");
        superframeSpec.finalCAPSlot = par("finalCAPSlot");
        dsmeSuperframeSpec.multiSuperframeOrder = par("multiSuperframeOrder");

        baseSuperframeDuration = 16*60 * par("symbolsPerSecond").doubleValue();     // 16 slots, 60 symbols per slot, symbols per second
        beaconInterval = baseSuperframeDuration * (1 << superframeSpec.beaconOrder);
        EV_DEBUG << "Beacon Interval: " << beaconInterval << endl;

        // timers
        beaconTimer = new cMessage("beacon-timer");

    } else if (stage == INITSTAGE_LINK_LAYER) {
        // PAN Coordinator starts network with beacon
        if (isPANCoordinator)
            scheduleAt(simTime() + beaconInterval, beaconTimer);  // TODO some magic time
    }
}

void DSME::handleSelfMessage(cMessage *msg) {
    if (msg == beaconTimer) {
        sendEnhancedBeacon();
    }
    else
        CSMA::handleSelfMessage(msg);
}

void DSME::handleLowerPacket(cPacket *msg) {
    IEEE802154eMACFrame_Base *macPkt = static_cast<IEEE802154eMACFrame_Base *>(msg);
    const MACAddress& src = macPkt->getSrcAddr();
    const MACAddress& dest = macPkt->getDestAddr();

    if(dest.isBroadcast()) {
        if(strcmp(macPkt->getName(), EnhancedBeacon::NAME) == 0) {
            EnhancedBeacon *beacon = static_cast<EnhancedBeacon *>(msg);
            EV_DEBUG << "Received EnhancedBeacon" << endl;
            return;
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

void DSME::sendCSMA(cPacket *msg) {

}

void DSME::sendEnhancedBeacon() {
    if (beaconFrame != nullptr)
        delete beaconFrame;
    beaconFrame = new EnhancedBeacon();
    beaconFrame->setBitLength(100);
    EV_DEBUG << "Send beacon " << beaconFrame->getDestAddr() << endl;
    sendDirect(beaconFrame);
    beaconFrame = nullptr;
    // schedule next beacon
    scheduleAt(simTime() + beaconInterval, beaconTimer);
}

} //namespace
