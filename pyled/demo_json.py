#!/usr/bin/env python3

import urllib2
import json

response = urllib2.urlopen('http://api-test.trafikanten.no/RealTime/GetRealTimeData/3012507')
buses = json.loads(response.read())

print buses
