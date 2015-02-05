//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/networklayer/configurator/geographic/GeographicNetworkConfigurator.h"
#include "inet/networklayer/geographic/StraightestNeighborTable.h"
#include "inet/common/INETDefs.h"

namespace inet {

Define_Module(GeographicNetworkConfigurator);


void GeographicNetworkConfigurator::initialize(int stage)
{
    NetworkConfiguratorBase::initialize(stage);
    if (stage == INITSTAGE_NETWORK_LAYER_3) {
        //long initializeStartTime = clock();
        Topology topology;
        // extract topology into the Topology object, then fill in a LinkInfo[] vector
        TIME(extractTopology(topology));
        // dump the result if requested
        if (par("dumpTopology").boolValue())
            TIME(dumpTopology(topology));
        // calculate shortest paths, and add corresponding static routes
        if (par("addStaticNeighbors").boolValue())
            TIME(addStaticNeighbors(topology));
    }
}


IRoutingTable *GeographicNetworkConfigurator::findRoutingTable(Node *node)
{
    return L3AddressResolver().findGenericRoutingTableOf(node->module);
}

void GeographicNetworkConfigurator::addStaticNeighbors(Topology& topology) {
    EV_INFO << "STATIC NEIGHBORS" << endl;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        // extract source
        Node *node = (Node *)topology.getNode(i);
        cModule *nodeModule = node->module;

        // get neighborTable
        INeighborTable *nt = dynamic_cast<INeighborTable *>(nodeModule->getSubmodule("neighborTable"));
        if (!nt)
            nt = dynamic_cast<INeighborTable *>(nodeModule->getModuleByPath(".neighborTable.geographic"));

        if(!nt)
            continue;

        node->neighborTable = nt;

        EV_INFO << "addStaticNeighbors (" << node->getNumOutLinks() << ") to " << nt << endl;
        for (int j = 0; j < node->getNumOutLinks(); j++) {
            EV_INFO << " -> " << node->getLinkOut(j)->getRemoteNode()->getModuleId() << endl;
        }


    }
}

} /* namespace inet */
