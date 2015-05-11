#!/bin/bash

opp_run -r 0..41 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &
opp_run -r 42..83 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &
opp_run -r 84..125 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &
opp_run -r 126..167 -u Cmdenv -c GeographicDSME -n ..:../../src -l ../../src/INET -f omnetpp.ini >/dev/null &


exit

#TODO do when oppruns done
for i in {0..47}
do
  ./extractPacketLoss.sh DSME- $i
done
