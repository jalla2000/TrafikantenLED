#!/usr/bin/env python
# -*- coding: utf-8 -*-
import urllib2
import time
import json
from ledfont import LedFont
from leddisplay import LedDisplay

def dateStrToStruct(datestring):
    return time.gmtime(int(datestring.split('(')[1].split('+')[0])/1000)

def sortBy(buses, paramKey):
    if paramKey == 'ExpectedArrivalTime':
        return sorted(buses, key=lambda bus: bus['ExpectedArrivalTime'])
    if paramKey == 'LineRef':
        return sorted(buses, key=lambda bus: int(bus['LineRef']))

def compressName(name):
    name = name.split(' o/')[0]
    name = name.split(' via ')[0]
    name = name.replace('terminal', 'term')
    name = name.replace(' stasjon', '')
    name = name.replace('ekspress', 'eksp')
    name = name.split(' eksp')[0]
    return name

def secondsLeft(bus):
    waitTime = time.mktime(bus['ExpectedArrivalTime']) - time.mktime(bus['RecordedAtTime'])
    return int(round(waitTime))

def minutesLeft(bus):
    return int(secondsLeft(bus)/60)

def drawOnLine(lineNo, bus):
    display.setPos(x=0, y=8*lineNo)
    display.writeTxt(bus['LineRef'])
    display.move(x=3)
    display.writeTxt(bus['DestinationDisplay'])
    waitTime = minutesLeft(bus)
    digitpos = 113
    for letter in str(waitTime):
        digitpos -= 1
        digitpos -= font.widths[letter]
    if waitTime > 0:
        display.writeTxt(str(waitTime), xpos=digitpos)
        display.writeTxt('min', xpos=113)
    else:
        display.writeTxt(u'nå', color='orange', xpos=116)
    display.send(lineNo)

DISPLAY_LINES = 4
DEVICE = '/dev/ttyUSB0'
font = LedFont()
display = LedDisplay(font, lines=DISPLAY_LINES, device=DEVICE)

while True:
    display.flush()
    try:
        buses = []

        trafikantenUrl = 'http://api-test.trafikanten.no/RealTime/GetRealTimeData/' + '3010065' #brugata
        response = urllib2.urlopen(trafikantenUrl)
        jsonstr = response.read()
        buses = json.loads(jsonstr)

        for bus in buses:
            bus['DestinationDisplay'] = compressName(bus['DestinationDisplay'])
            bus['AimedArrivalTime'] = dateStrToStruct(bus['AimedArrivalTime'])
            bus['AimedDepartureTime'] = dateStrToStruct(bus['AimedDepartureTime'])
            bus['ExpectedArrivalTime'] = dateStrToStruct(bus['ExpectedArrivalTime'])
            bus['ExpectedDepartureTime'] = dateStrToStruct(bus['ExpectedDepartureTime'])
            bus['RecordedAtTime'] = dateStrToStruct(bus['RecordedAtTime'])

        buses = sortBy(buses, 'ExpectedArrivalTime')

        drawOnLine(0, buses[0])
        drawOnLine(1, buses[1])
        drawOnLine(2, buses[2])
        display.send()
        for bus in buses[3:]:
            # print "scrolling bus: " + bus['DestinationDisplay']
            text = bus['LineRef'] + ' ' + bus['DestinationDisplay'] + ' ' + str(minutesLeft(bus)) + ' min'
            for i in range(128, -128, -1):
                display.flush(line=3)
                display.writeTxt(text, xpos=i, ypos=8*3)
                display.send(line=3)


    except IOError:
        print "Failed to fetch data"
        display.flush()
        display.writeTxt("Offline...", xpos=0, ypos=0, color='red')
        display.send()
