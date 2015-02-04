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
    EV_INFO << "Geographic NW Protocol" << endl;
}

GeographicNetworkProtocol::~GeographicNetworkProtocol() {
    // TODO Auto-generated destructor stub
}

void GeographicNetworkProtocol::initialize() {
    GenericNetworkProtocol::initialize();
    EV_INFO << "GeographicNWProtocol -> initialize" << endl;
    neighborTable = getModuleFromPar<NeighborTable>(par("neighborTableModule"), this);
}


void GeographicNetworkProtocol::routePacket(GenericDatagram *datagram, const InterfaceEntry *destIE, const L3Address& nextHop, bool fromHL) {
    EV_INFO << "Geographic Routing!" << endl;
    //GenericNetworkProtocol::routePacket(datagram, destIE, nextHop, fromHL);
}


void GeographicNetworkProtocol::datagramPreRouting(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *destIE, const L3Address& nextHop)
{
    // route packet
    EV_INFO << "Geographic Routing!" << endl;
    if (!datagram->getDestinationAddress().isMulticast())
        routePacket(datagram, destIE, nextHop, false);
    else
        routeMulticastPacket(datagram, destIE, inIE);
}

void GeographicNetworkProtocol::datagramLocalOut(GenericDatagram *datagram, const InterfaceEntry *destIE, const L3Address& nextHop)
{
    // route packet
    EV_INFO << "Geographic Routing!" << endl;
    if (!datagram->getDestinationAddress().isMulticast())
        routePacket(datagram, destIE, nextHop, true);
    else
        routeMulticastPacket(datagram, destIE, nullptr);
}

}
