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

void DSME::initialize()
{
    CSMA::initialized();
}

void DSME::initialize(int stage)
{
    CSMA::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // timers
        beaconTimer = new cMessage("beacon-timer");

    } else if (stage == INITSTAGE_LINK_LAYER) {
        // TODO if coordinator
        if(0 == getParentModule()->getParentModule()->getIndex())
            scheduleAt(simTime() + 0.01, beaconTimer);  // TODO some magic time
    }
}

void DSME::handleSelfMessage(cMessage *msg) {
    if (msg == beaconTimer) {
        sendEnhancedBeacon();
    }
    else
        CSMA::handleSelfMessage(msg);
}

void DSME::sendDirect(cPacket *msg) {
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    attachSignal(msg, simTime() + aTurnaroundTime); // TODO turnaroundTime only on statechange, plus parameter is useless?
    sendDown(msg);
}

void DSME::sendCSMA(cPacket *msg) {

}

void DSME::sendEnhancedBeacon() {
    EV_DEBUG << "DSME its BEACON TIME" << endl;
    if (beaconFrame != nullptr)
        delete beaconFrame;
    beaconFrame = new EnhancedBeacon();
    beaconFrame->setBitLength(100);
    sendDirect(beaconFrame);
    beaconFrame = nullptr;
    // schedule next beacon
    scheduleAt(simTime() + 0.01, beaconTimer);
}

} //namespace
