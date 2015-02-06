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

StraightestNeighborTable::StraightestNeighborTable() :
        position(nullptr)
{
}

StraightestNeighborTable::~StraightestNeighborTable() {

}

void StraightestNeighborTable::setPosition(Coord *position) {
    this->position = position;
}

void StraightestNeighborTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);
}

void StraightestNeighborTable::addNeighbor(L3Address& addr, Coord& coord) {
    neighbors.push_back(std::pair<L3Address, Coord>(addr, coord));
}

L3Address* StraightestNeighborTable::getNextHop(Coord& src, Coord& dest) {
    return nullptr;
}

}
