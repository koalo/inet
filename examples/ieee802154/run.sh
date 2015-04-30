#!/bin/bash

opp_run -r 0..11 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &
opp_run -r 12..23 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &
opp_run -r 24..35 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &
opp_run -r 36..47 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &


exit

#TODO do when oppruns done
for i in {0..47}
do
  ./extractPacketLoss.sh DSME- $i
done
