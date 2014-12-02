//
// Copyright (C) 2014 Florian Meier
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "DummyTransport.h"

#include "inet/networklayer/common/IPSocket.h"

namespace inet {

Define_Module(DummyTransport);

DummyTransport::DummyTransport()
: TransportProtocolBase() {
}

DummyTransport::~DummyTransport() {
}

void DummyTransport::initialize(int stage) {
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        upperLayerInGateId = findGate("upperLayerIn",0);
        upperLayerOutGateId = findGate("upperLayerOut",0);
        lowerLayerInGateId = findGate("lowerLayerIn");
        lowerLayerOutGateId = findGate("lowerLayerOut");
    }

    if (stage == INITSTAGE_NETWORK_LAYER_2) {
        IPSocket socket(gate("lowerLayerOut"));
        socket.registerProtocol(IP_PROT_NONE);
    }
}

void DummyTransport::handleUpperPacket(cPacket *packet) {
    send(packet, lowerLayerOutGateId);
}

void DummyTransport::handleLowerPacket(cPacket *packet) {
    send(packet, upperLayerOutGateId);
}

bool DummyTransport::isUpperMessage(cMessage *message) {
    return message->getArrivalGate()->isName("upperLayerIn");
}

void DummyTransport::handleUpperCommand(cMessage *message) {
    delete message;
}

bool DummyTransport::isLowerMessage(cMessage *message) {
    return message->getArrivalGate()->isName("lowerLayerIn");
}

} // namespace inet

