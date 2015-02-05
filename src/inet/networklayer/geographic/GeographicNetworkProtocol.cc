/*
 * GeographicNetworkProtocol.cpp
 *
 *  Created on: 03.02.2015
 *      Author: luebkert
 */

#include "inet/networklayer/geographic/GeographicNetworkProtocol.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(GeographicNetworkProtocol);

GeographicNetworkProtocol::GeographicNetworkProtocol()
    : GenericNetworkProtocol(),
      neighborTable(nullptr)
{
}

GeographicNetworkProtocol::~GeographicNetworkProtocol() {
    // TODO Auto-generated destructor stub
}

void GeographicNetworkProtocol::initialize() {
    GenericNetworkProtocol::initialize();
    neighborTable = getModuleFromPar<INeighborTable>(par("neighborTableModule"), this);
}


void GeographicNetworkProtocol::routePacket(GenericDatagram *datagram, const InterfaceEntry *destIE, const L3Address& nextHop, bool fromHL) {
    EV_INFO << "Geographic Routing!" << endl;
    GenericNetworkProtocol::routePacket(datagram, destIE, nextHop, fromHL);
}


void GeographicNetworkProtocol::datagramPreRouting(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *destIE, const L3Address& nextHop) {
    // get IF out links
    inIE->getNetworkInterfaceModule();
    GenericNetworkProtocol::datagramPreRouting(datagram, inIE, destIE, nextHop);
}


}
