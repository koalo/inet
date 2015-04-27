#!/bin/bash

if [ "$#" -lt 1 ]
	then
	echo $0 [prefix = "-"] run-id
	exit
fi

prefix="-"
scaId=$1
if [ "$#" -gt 1 ]
	then
	prefix=$1
  scaId=$2
fi

cd results

scaFile=$(ls *"${prefix}${scaId}".sca)

echo $scaFile

grep "trafficgen.*rcvdPk:group" "$scaFile" |\
 			sed 's/.*:host\[/scalar Net802154Geographic.host[/' |\
			sed 's/.wlan\[.\]/.trafficgen 	destRcvdPk/'\
			 >> "$scaFile"
