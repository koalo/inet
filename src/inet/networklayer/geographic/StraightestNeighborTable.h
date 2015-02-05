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
public:
    StraightestNeighborTable();
    virtual ~StraightestNeighborTable();

protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
};

}

#endif /* STRAIGHTESTNEIGHBORINGTABLE_H_ */

