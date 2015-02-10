/*
 * NeighboringTable.h
 *
 *  Created on: 03.02.2015
 *      Author: tonsense
 */

#ifndef STRAIGHTESTNEIGHBORINGTABLE_H_
#define STRAIGHTESTNEIGHBORINGTABLE_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/geographic/INeighborTable.h"

namespace inet {

class INET_API StraightestNeighborTable : public cSimpleModule, public INeighborTable {

protected:
    Coord position;
    InterfaceEntry *interfaceEntry;
    std::map<L3Address, Coord> *addrCoordMap;
    std::vector<std::pair<L3Address, Coord>> neighbors;

public:
    StraightestNeighborTable();
    virtual ~StraightestNeighborTable();


    virtual void setPosition(Coord position);
    virtual void setInterfaceEntry(InterfaceEntry *interfaceEntry);

    virtual void setAddrCoordMap(std::map<L3Address, Coord>& map);
    virtual Coord getCoordByAddr(L3Address&);

    virtual Coord getPosition();
    virtual InterfaceEntry* getInterfaceEntry();

    virtual int getNumNeighbors() { return neighbors.size(); }

    virtual void addNeighbor(L3Address& addr, Coord& coord);

    virtual L3Address getNextHop(Coord& src, Coord& dest);

protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);


};

}

#endif /* STRAIGHTESTNEIGHBORINGTABLE_H_ */

