/*
 * GeographicNetworkProtocol.h
 *
 *  Created on: 03.02.2015
 *      Author: luebkert
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
    virtual void routePacket(GenericDatagram *datagram, const InterfaceEntry *destIE, const L3Address& nextHop, bool fromHL);

    virtual void datagramPreRouting(GenericDatagram *datagram, const InterfaceEntry *inIE, const InterfaceEntry *destIE, const L3Address& nextHop);
    virtual void datagramLocalOut(GenericDatagram *datagram, const InterfaceEntry *destIE, const L3Address& nextHop);

protected:
    virtual void initialize();

};

}

#endif /* GEOGRAPHICNETWORKPROTOCOL_H_ */
