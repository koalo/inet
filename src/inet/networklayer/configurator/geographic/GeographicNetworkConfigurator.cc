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
#include "inet/networklayer/generic/GenericNetworkProtocolInterfaceData.h"
#include "inet/physicallayer/contract/IRadio.h"
#include "inet/common/INETDefs.h"
#include <math.h>

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


Coord GeographicNetworkConfigurator::getNodeIEPosition(Node *node, int i_ie) {
    return check_and_cast<physicallayer::IRadio *>(node->interfaceTable->getInterface(i_ie)->getNetworkInterfaceModule()->getSubmodule("radio"))->getAntenna()->getMobility()->getCurrentPosition();
}


void GeographicNetworkConfigurator::addStaticNeighbors(Topology& topology) {
    EV_INFO << "STATIC NEIGHBORS" << endl;

    // map addresses to coordinates and provide that to each neighborTable
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        // TODO make this more professional: use all interfaces!
        L3Address addr = node->interfaceTable->getInterface(0)->getNetworkAddress();
        addrCoordMap[addr] = getNodeIEPosition(node, 0);
        EV_INFO << addr << " @ " << addrCoordMap[addr] << endl;
    }

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
        if (!node->interfaceTable)
            continue;

        // TODO make this more professional: use all interfaces!
        Coord pos = getNodeIEPosition(node, 0);
        node->neighborTable->setPosition(pos);
        node->neighborTable->setInterfaceEntry(node->interfaceTable->getInterface(0));
        node->neighborTable->setAddrCoordMap(addrCoordMap);

        EV_INFO << "addStaticNeighbors to " << node->module->getFullPath() << " at " << pos << endl;
        for (int j = 0; j < node->getNumOutLinks(); j++) {
            Topology::LinkOut *linkOut = node->getLinkOut(j);
            ASSERT(linkOut->getLocalNode() == node);
            Node *remoteNode = (Node *)linkOut->getRemoteNode();
            if (!isinf(linkOut->getWeight())) {
                EV_INFO << "     -> " << remoteNode->module->getFullPath() << " " << linkOut->getWeight();
                IInterfaceTable *destinationInterfaceTable = remoteNode->interfaceTable;

                for (int j = 0; j < destinationInterfaceTable->getNumInterfaces(); j++) {
                    InterfaceEntry *destinationInterfaceEntry = destinationInterfaceTable->getInterface(j);
                    if (!destinationInterfaceEntry->getGenericNetworkProtocolData())
                        continue;
                    L3Address destinationAddress = destinationInterfaceEntry->getGenericNetworkProtocolData()->getAddress();
                    if (!destinationInterfaceEntry->isLoopback() && !destinationAddress.isUnspecified()) {
                        cModule *receiverInterfaceModule = destinationInterfaceEntry->getNetworkInterfaceModule();
                        physicallayer::IRadio *receiverRadio = check_and_cast<physicallayer::IRadio *>(receiverInterfaceModule->getSubmodule("radio"));
                        Coord destPosition = receiverRadio->getAntenna()->getMobility()->getCurrentPosition();
                        node->neighborTable->addNeighbor(destinationAddress, destPosition);
                        EV_INFO << ";   " << destPosition; // destinationAddress.str();
                    }
                    EV_INFO << endl;
                }
            }
        }


    }
}

} /* namespace inet */
