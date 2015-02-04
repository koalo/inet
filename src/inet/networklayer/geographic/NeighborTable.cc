/*
 * NeighboringTable.cpp
 *
 *  Created on: 03.02.2015
 *      Author: luebkert
 */

#include <sstream>

#include "inet/networklayer/geographic/NeighborTable.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(NeighborTable);

NeighborTable::NeighborTable() {
    // TODO Auto-generated constructor stub
}

NeighborTable::~NeighborTable() {
    // TODO Auto-generated destructor stub
}


void NeighborTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    EV_INFO << "NeighborTable" << endl;

    if (stage == INITSTAGE_LOCAL) {

    }
}

}
