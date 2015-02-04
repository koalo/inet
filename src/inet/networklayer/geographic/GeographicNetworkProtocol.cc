/*
 * GeographicNetworkProtocol.cpp
 *
 *  Created on: 03.02.2015
 *      Author: luebkert
 */

#include "inet/networklayer/geographic/GeographicNetworkProtocol.h"

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
    // neighboringTable = getModuleFromPar<NeighboringTable>(par("neighboringTableModule"), this);
}

}
