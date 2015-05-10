#!/bin/bash

opp_run -r 0..55 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &
opp_run -r 56..127 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &
opp_run -r 128..167 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &
#opp_run -r 36..47 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &


exit

#TODO do when oppruns done
for i in {0..47}
do
  ./extractPacketLoss.sh DSME- $i
done
