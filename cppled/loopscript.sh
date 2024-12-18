#!/usr/bin/env bash
touch /tmp/loopscript_ran
date > /tmp/loopscript_last
while true
do
    /home/pi/src/TrafikantenLED/cppled/build/trafikanten --output /dev/ttyUSB0
    echo "Terminated for some reason. Sleeping 10s, then starting again"
    sleep 10
done
