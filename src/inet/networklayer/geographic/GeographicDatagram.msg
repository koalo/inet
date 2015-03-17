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


cplusplus {{
#include "inet/common/geometry/common/Coord.h"
#include "inet/networklayer/generic/GenericDatagram.h"
}}

namespace inet;

packet GenericDatagram;

class noncobject Coord;

//
// Geographic network datagram also include coordinates of source and destination
//
packet GeographicDatagram extends GenericDatagram {
    @customize(true);
    byteLength = 8 + 8 + 3; 				// Addrs, Coords, Protocol
    Coord sourceCoord @getter(getSrcCoord);
    Coord destinationCoord @getter(getDestCoord);
}
