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

#include "inet/linklayer/ieee802154e/GTS.h"

namespace inet {

const GTS GTS::UNDEFINED = GTS(0xff, 0xff, 0xff);         // TODO ugly workaround, also there are no 256 channels...

GTS::GTS(uint8_t superframeID, uint8_t slotID, uint8_t channel) {
    this->superframeID = superframeID;
    this->slotID = slotID;
    this->channel = channel;
}

bool GTS::operator<(const GTS& other) {
    return (this->slotID < other.slotID);
}

} /* namespace inet */
