// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_GeographicDatagram_H
#define __INET_GeographicDatagram_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/geographic/GeographicDatagram_m.h"

namespace inet {

/**
 * Represents a geographic datagram. More info in the GeographicDatagram.msg file
 * (and the documentation generated from it).
 */
class INET_API GeographicDatagram : public GeographicDatagram_Base
{
  public:
    GeographicDatagram(const char *name = nullptr, int kind = 0) : GeographicDatagram_Base(name, kind) {}
    GeographicDatagram(const GeographicDatagram& other) : GeographicDatagram_Base(other.getName()) { operator=(other); }
    GeographicDatagram& operator=(const GeographicDatagram& other) { GeographicDatagram_Base::operator=(other); return *this; }
    virtual GeographicDatagram *dup() const { return new GeographicDatagram(*this); }

    GeographicDatagram(const GenericDatagram& other) : GeographicDatagram_Base(other.getName()) { operator=(other); }
    GeographicDatagram& operator=(const GenericDatagram& other) { GenericDatagram_Base::operator=(other); return *this; }

};

} // namespace inet

#endif // ifndef __INET_GeographicDatagram_H

