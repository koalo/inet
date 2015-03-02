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

#include "inet/linklayer/ieee802154e/EnhancedBeacon.h"

namespace inet {

const char *EnhancedBeacon::NAME = "enhanced-beacon";

EnhancedBeacon::EnhancedBeacon() :
        IEEE802154eMACFrame(NAME)
{
    destAddr_var.setBroadcast();
    setBitLength(100);  // TODO exact length
}

} /* namespace inet */
