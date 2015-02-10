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

L3Address StraightestNeighborTable::getNextHop(Coord& src, Coord& dest) {
    L3Address nextHop;
    double sqDistSelf = position.sqrdist(dest);     // distance self to destination
    double sqDistNext;
    double sqLineDistNext;
    double sqLineDistCur = INFINITY;
                                                    // Straight line from src to dest
    Coord c = dest - src;                           // distance vector c

    double s;                                       // g(s) = src + s*c    s:scalar
    Coord g;

    for (auto& n : neighbors) {
        Coord p = n.second;
        sqDistNext = dest.sqrdist(p);               // distance of neighbor n to destination

        EV_DEBUG << "sqDist: " << sqDistNext << " < " << sqDistSelf << endl;
        if (sqDistNext < sqDistSelf) {              // only nodes which are closer to destination
            s = (c.x*(p.x-src.x) + c.y*(p.y-src.y)) / (c.squareLength());
            EV_DEBUG << s << " | ";
            g = src + c*s;
            EV_DEBUG << g << " | ";
            sqLineDistNext = g.sqrdist(p);          // distance of neighbor to straig line g
            EV_DEBUG << sqLineDistNext << endl;

            if (sqLineDistNext < sqLineDistCur) {   // smallest distance to line
                sqLineDistCur = sqLineDistNext;
                nextHop = n.first;
            }
        }
    }

    return nextHop;
}

}
