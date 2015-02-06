/*
 * NeighboringTable.h
 *
 *  Created on: 03.02.2015
 *      Author: luebkert
 */

#ifndef STRAIGHTESTNEIGHBORINGTABLE_H_
#define STRAIGHTESTNEIGHBORINGTABLE_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/geographic/INeighborTable.h"

namespace inet {

class INET_API StraightestNeighborTable : public cSimpleModule, public INeighborTable {

protected:
    Coord *position;
    std::vector<std::pair<L3Address, Coord>> neighbors;

public:
    StraightestNeighborTable();
    virtual ~StraightestNeighborTable();


    virtual void setPosition(Coord *position);

    virtual int getNumNeighbors() { return neighbors.size(); }

    virtual void addNeighbor(L3Address& addr, Coord& coord);

    virtual L3Address* getNextHop(Coord& src, Coord& dest);

protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);


};

}

#endif /* STRAIGHTESTNEIGHBORINGTABLE_H_ */

