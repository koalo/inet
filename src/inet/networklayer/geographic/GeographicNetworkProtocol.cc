/*
 * GeographicNetworkProtocol.cpp
 *
 *  Created on: 03.02.2015
 *      Author: luebkert
 */

#include "inet/networklayer/geographic/GeographicNetworkProtocol.h"
#include "inet/networklayer/geographic/GeographicDatagram.h"
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

GenericDatagram* GeographicNetworkProtocol::encapsulate(cPacket *transportPacket, const InterfaceEntry *& destIE) {
    GenericDatagram* genericDatagram = GenericNetworkProtocol::encapsulate(transportPacket, destIE);
    GeographicDatagram* datagram = new GeographicDatagram(*genericDatagram);
    L3Address srcAddr = datagram->getSourceAddress();
    if (srcAddr.isUnspecified())
        srcAddr = neighborTable->getInterfaceEntry()->getNetworkAddress(); // TODO same old problem with multiple IEs
    L3Address destAddr = datagram->getDestinationAddress();
    EV_INFO << "Geographic encapsulate: " << srcAddr << " @ " << neighborTable->getCoordByAddr(srcAddr) << endl;
    datagram->setSourceCoord(neighborTable->getCoordByAddr(srcAddr));
    datagram->setDestinationCoord(neighborTable->getCoordByAddr(destAddr));
    return datagram;
}

void GeographicNetworkProtocol::routePacket(GenericDatagram *datagram, const InterfaceEntry *destIE, const L3Address& nextHop, bool fromHL) {
    GeographicDatagram* geographicDatagram = static_cast<GeographicDatagram*>(datagram);
    InterfaceEntry *outIE = neighborTable->getInterfaceEntry();

    L3Address nextNeighbor = *neighborTable->getNextHop(geographicDatagram->getSrcCoord(), geographicDatagram->getDestCoord());

    EV_INFO << "Geographic Routing @ " << neighborTable->getPosition() << " || src: " << geographicDatagram->getSrcCoord() << ", dest: " << geographicDatagram->getDestCoord() << endl;
    GenericNetworkProtocol::routePacket(datagram, outIE, nextNeighbor, fromHL);
}


void GeographicNetworkProtocol::datagramPreRouting(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *destIE, const L3Address& nextHop) {
    // get IF out links
    inIE->getNetworkInterfaceModule();
    GenericNetworkProtocol::datagramPreRouting(datagram, inIE, destIE, nextHop);
}


}
