/*
 * NeighboringTable.h
 *
 *  Created on: 03.02.2015
 *      Author: luebkert
 */

#ifndef NEIGHBORINGTABLE_H_
#define NEIGHBORINGTABLE_H_

#include "inet/common/INETDefs.h"

namespace inet {

class INET_API NeighborTable : public cSimpleModule {
public:
    NeighborTable();
    virtual ~NeighborTable();

protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
};

}

#endif /* NEIGHBORINGTABLE_H_ */

