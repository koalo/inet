/*
 * GeographicNetworkProtocol.h
 *
 *  Created on: 03.02.2015
 *      Author: tonsense
 */

#ifndef GEOGRAPHICNETWORKPROTOCOL_H_
#define GEOGRAPHICNETWORKPROTOCOL_H_

#include "inet/networklayer/generic/GenericNetworkProtocol.h"
#include "inet/networklayer/geographic/INeighborTable.h"

namespace inet {

class INET_API GeographicNetworkProtocol: public GenericNetworkProtocol {

protected:
    INeighborTable *neighborTable;

public:
    GeographicNetworkProtocol();
    virtual ~GeographicNetworkProtocol();

protected:
    virtual void datagramPreRouting(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *destIE, const L3Address& nextHop);
    virtual void routePacket(GenericDatagram *datagram, const InterfaceEntry *destIE, const L3Address& nextHop, bool fromHL);

    virtual GenericDatagram *encapsulate(cPacket *transportPacket, const InterfaceEntry *& destIE);

protected:
    virtual void initialize();

};

}

#endif /* GEOGRAPHICNETWORKPROTOCOL_H_ */
