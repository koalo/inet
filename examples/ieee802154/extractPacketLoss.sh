#!/bin/bash

if [ "$#" -lt 1 ]
	then
	echo $0 run-id
	exit
fi

#numPackets=100

cd results

scaFile=$(ls *"${1}".sca)

grep "trafficgen.*rcvdPk:group" "$scaFile" |\
 			sed 's/.*:host\[/scalar Net802154Geographic.host[/' |\
			sed 's/.wlan\[.\]/.trafficgen 	destRcvdPk/'\
			 >> "$scaFile"
