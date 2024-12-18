#!/usr/bin/env bash
cd /home/pi/src/TrafikantenLED/cppled
echo "Starting screen"
/usr/bin/screen -dmS ledaemon ./loopscript.sh
