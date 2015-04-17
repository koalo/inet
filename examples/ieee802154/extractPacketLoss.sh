#!/bin/bash

numPackets=100
scaFile=GeographicDSME-0.sca

cd results
	
grep rcvdPkSrc GeographicDSME-0.sca | sed 's/.*:host\[/scalar Net802154Geographic.host[/' | sed 's/.wlan\[.\]/.trafficgen 	destRcvdPk/' >> $scaFile