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

StraightestNeighborTable::StraightestNeighborTable()
{
}

StraightestNeighborTable::~StraightestNeighborTable() {
}

void StraightestNeighborTable::setPosition(Coord position) {
    this->position = position;
}

void StraightestNeighborTable::setInterfaceEntry(InterfaceEntry *interfaceEntry) {
    this->interfaceEntry = interfaceEntry;
}

void StraightestNeighborTable::setAddrCoordMap(std::map<L3Address,Coord>& map) {
    this->addrCoordMap = &map;
}

Coord StraightestNeighborTable::getCoordByAddr(L3Address& addr) {
    return (*this->addrCoordMap)[addr];
}

Coord StraightestNeighborTable::getPosition() {
    return this->position;
}

InterfaceEntry* StraightestNeighborTable::getInterfaceEntry() {
    return this->interfaceEntry;
}

void StraightestNeighborTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);
}

void StraightestNeighborTable::addNeighbor(L3Address& addr, Coord& coord) {
    neighbors.push_back(std::pair<L3Address, Coord>(addr, coord));
}

L3Address* StraightestNeighborTable::getNextHop(Coord& src, Coord& dest) {
    L3Address *nextHop = nullptr;
    for (auto& n : neighbors) {
        nextHop =  &n.first;
    }
    return nextHop;
}

}
