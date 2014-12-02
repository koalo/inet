//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 Andras Varga
//
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




package inet.transportlayer.dummy;

import inet.transportlayer.common.TransportProtocolBase;


//
// DummyTransport protocol implementation, for IPv4 and IPv6.
//
// The DummyTransport protocol header is represented by the class ~DummyTransportPacket.
//
// <b>Communication with clients (applications)</b>
//
// The module can (should) be connected to several applications.
// For sending an DummyTransport packet, the application should attach an ~DummyTransportControlInfo
// object to the payload, and send it to ~DummyTransport.
// ~DummyTransport will also attach an ~DummyTransportControlInfo object to any payload
// message in sends up to the application.
//
// For receiving DummyTransport packets, the connected applications should first
// "bind" to the given DummyTransport port. This can be done by sending an
// arbitrary message with message kind DummyTransport_C_BIND and an ~DummyTransportControlInfo
// attached with srcPort filled in.
//
// When DummyTransport receives an ICMP error (~ICMPMessage or ~ICMPv6Message)
// that refers to an DummyTransport socket, it reports the error to the corresponding
// application by sending a message with kind DummyTransport_I_ERROR.
//
// <b>Communication with the IP (IPv4/IPv6) layer</b>
//
// The ~DummyTransport model relies on sending and receiving ~IPv4ControlInfo/~IPv6ControlInfo
// objects attached to ~DummyTransportPacket objects as control info.
//
// @see ~DummyTransportPacket, ~DummyTransportControlInfo, ~DummyTransportCommandCode
//
simple DummyTransport extends TransportProtocolBase
{
    parameters:
	    @class(DummyTransport);
	    //@namespace(inet);
        @display("i=block/transport");

    gates:
    //    input appIn[] @labels(DummyTransportControlInfo/down);
    //    input ipIn @labels(DummyTransportPacket,IPv4ControlInfo/up,IPv6ControlInfo/up);
    //    output appOut[] @labels(DummyTransportControlInfo/up);
    //    output ipOut @labels(DummyTransportPacket,IPv4ControlInfo/down,IPv6ControlInfo/down);
}
