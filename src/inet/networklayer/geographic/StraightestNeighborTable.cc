/*
 * NeighboringTable.cpp
 *
 *  Created on: 03.02.2015
 *      Author: luebkert
 */

#include "inet/networklayer/geographic/StraightestNeighborTable.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(StraightestNeighborTable);

StraightestNeighborTable::StraightestNeighborTable() {
    // TODO Auto-generated constructor stub
}

StraightestNeighborTable::~StraightestNeighborTable() {
    // TODO Auto-generated destructor stub
}


void StraightestNeighborTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    EV_INFO << "StraightestNeighborTable" << endl;

    if (stage == INITSTAGE_LOCAL) {

    }
}

}
