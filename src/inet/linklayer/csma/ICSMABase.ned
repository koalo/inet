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

package inet.linklayer.csma;

import inet.linklayer.contract.IMACProtocol;

//
// Based on inet.linklayer.csma.CSMA !
//
moduleinterface ICSMABase extends IMACProtocol
{
	parameters:
        string address; // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.

        // length of MAC header
        int headerLength @unit(bit);
        int mtu @unit("B");
        // size of the MAC queue (maximum number of packets in Tx buffer)
        int queueLength;
        // bit rate 
        double bitrate @unit(bps);

        // Clear Channel Assessment detection time
        double ccaDetectionTime @unit(s); // 8 symbols
        // Time to setup radio to reception state
        double rxSetupTime @unit(s);
        // Time to switch radio from Rx to Tx state
        double aTurnaroundTime @unit(s);    // 12 symbols

        // Send/Expect MAC acks for unicast traffic?
        bool useMACAcks;
        // Maximum number of frame retransmission,
        // only used when usage of MAC acks is enabled.
        int macMaxFrameRetries;
        // Time to wait for an acknowledgement after transmitting a unicast frame.
        // Only used when usage of MAC acks is enabled.
        // Value is calculated from 1+12+10+12 symbols, which is defined for nonbeacon-enabled PAN.
        // In the non-beacon-enabled case, the receiver responds at aTurnaroundTime 
        // (i.e. the time for the sender and receiver to both be guaranteed to have 
        // switched from Tx to Rx and vice verse).  This gives the value 192us + 
        // 352us = 544us (there's been some discussion about the "extra" 1 symbol == 16us) 
        // [section 7.5.6.4.2 of the specification].
        double macAckWaitDuration @unit(s);
        // Complete MAC ack message length (in bits) 
        // (! headerLength is not added to this),
        // only used when usage of MAC acks is enabled.
        double ackLength @unit(bit);
        // Simple interframe space (12 symbols). Time to wait between receiving a frame and acknowledging it.
        // Should be bigger than the maximum time for switching between Tx and Rx at the receiver.
        // Only used when usage of MAC acks is enabled.
        double sifs @unit(s);

        //Backoff method to use: constant, linear or exponential
        string backoffMethod;
        // maximum number of extra backoffs (excluding the first unconditional one) before frame drop
        int macMaxCSMABackoffs;
        // base unit for all backoff calculations
        double aUnitBackoffPeriod @unit(s);
        // # of backoff periods of the initial contention window
        // (for linear and constant backoff method only)
        int contentionWindow;
        // minimum backoff exponent (for exponential backoff method only)
        double macMinBE;
        // maximum backoff exponent (for exponential backoff method only)
        double macMaxBE;
}
