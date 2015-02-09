/*
 * INeighborTable.h
 *
 *  Created on: 05.02.2015
 *      Author: tonsense
 */

#ifndef INEIGHBORTABLE_H_
#define INEIGHBORTABLE_H_

#include "inet/common/geometry/common/Coord.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/InterfaceEntry.h"

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API INeighborTable {

public:

    virtual void setPosition(Coord position) = 0;
    virtual void setInterfaceEntry(InterfaceEntry *interfaceEntry) = 0;

    virtual void setAddrCoordMap(std::map<L3Address,Coord>& map) = 0;
    virtual Coord getCoordByAddr(L3Address&) = 0;

    virtual Coord getPosition() = 0;
    virtual InterfaceEntry* getInterfaceEntry() = 0;

    virtual void addNeighbor(L3Address& addr, Coord& coord) = 0;
    virtual int getNumNeighbors() = 0;

    virtual L3Address* getNextHop(Coord& src, Coord& dest) = 0;

};

}



#endif /* INEIGHBORTABLE_H_ */
