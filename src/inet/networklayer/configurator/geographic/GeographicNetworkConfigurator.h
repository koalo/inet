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

#ifndef GEOGRAPHICNETWORKCONFIGURATOR_H_
#define GEOGRAPHICNETWORKCONFIGURATOR_H_

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"
#include "inet/networklayer/generic/GenericRoutingTable.h"

namespace inet {

class INET_API GeographicNetworkConfigurator: public NetworkConfiguratorBase {

    protected:
      // parameters
      bool addStaticNeighborsParameter;

      // internal state
      Topology topology;


    protected:
      virtual void initialize(int stage);

      /**
       *
       */
      virtual void addStaticNeighbors(Topology& topology);


      /**
       * TODO make this obsolete, is needed to extractTopology, beacuse "isBridgeNode()" checks for routingTable
       */
      virtual IRoutingTable *findRoutingTable(Node *node);
};

} /* namespace inet */

#endif /* GEOGRAPHICNETWORKCONFIGURATOR_H_ */
