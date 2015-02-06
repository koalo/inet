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
    // TODO get dest coord from datagram? --> create GeographicDatagram!
    // TODO get srcCoord from datagram??
    // TODO set output interface (destIE) -> needs to be in neighborTable or somewhat
    Coord srcCoord;
    Coord destCoord;
    InterfaceEntry *outIE = interfaceTable->getInterface(0);  // TODO make this more professional!
    L3Address nextNeighbor = *neighborTable->getNextHop(srcCoord, destCoord);
    EV_INFO << "Geographic Routing! " << destIE << " " << nextNeighbor << " " << !nextNeighbor.isUnspecified() << endl;
    GenericNetworkProtocol::routePacket(datagram, outIE, nextNeighbor, fromHL);
}


void GeographicNetworkProtocol::datagramPreRouting(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *destIE, const L3Address& nextHop) {
    // get IF out links
    inIE->getNetworkInterfaceModule();
    GenericNetworkProtocol::datagramPreRouting(datagram, inIE, destIE, nextHop);
}


}
