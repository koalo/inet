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

#ifndef ENHANCEDBEACON_H_
#define ENHANCEDBEACON_H_

#include "inet/linklayer/ieee802154e/IEEE802154eMACFrame_m.h"

namespace inet {

class EnhancedBeacon: public IEEE802154eMACFrame_Base {
public:

    static const char* NAME;

    EnhancedBeacon();

    virtual EnhancedBeacon *dup() const {return new EnhancedBeacon(*this);}
};

} /* namespace inet */

#endif /* ENHANCEDBEACON_H_ */
