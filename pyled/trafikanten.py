#!/usr/bin/env python
# -*- coding: utf-8 -*-
import urllib2
import time
import json
import sys
from ledfont import LedFont
from leddisplay import LedDisplay
import random

MINIMUM_REQUEST_INTERVAL = 20
DISPLAY_LINES = 4
DEVICE = '/dev/ttyUSB0'
MODE_ASCEND = 'layout=ascend'
MODE_HSCROLL = 'layout=hscroll'
MODE_STATIC = 'layout=static'
DISPLAY_MODE = MODE_ASCEND
STOPIDS = {}
STOPIDS[u'Thune'] = 3012507
STOPIDS[u'Stabekkbakken'] = 2190106
STOPIDS[u'Oksenøyveien'] = 2190040
STOPIDS[u'Brugata'] = 3010065
STOPIDS[u'Hammersborggata arb samf'] = 3014067
STOPIDS[u'Hammersborggata storgt'] = 3010066
STOPIDS[u'Arendalsgata'] = 3010425
STOPIDS[u'Torshov'] = 3010430


STOPS = {}
STOPS[STOPIDS[u'Thune']] = 'E'

def stopIdToName(stopId):
    for key in STOPIDS:
        if STOPIDS[key] == stopId:
            return key
    return "<nameLookupError!>"

def readConfigPreset():
    global DISPLAY_MODE
    try:
        cfgFile = open('.config')
        config = cfgFile.read()
        if MODE_ASCEND in config:
            DISPLAY_MODE=MODE_ASCEND
        if MODE_HSCROLL in config:
            DISPLAY_MODE=MODE_HSCROLL
        if MODE_STATIC in config:
            DISPLAY_MODE=MODE_STATIC
        if 'port=' in config:
            DEVICE = config.split('port=')[1].split()[0]
            print "Port defined in .config: %s" % DEVICE
        if 'lines=' in config:
            DISPLAY_LINES = int(config.split('lines=')[1].split()[0])
            print "Number of lines defined in .config: %d" % DISPLAY_LINES
        if 'preset=thune' in config:
            STOPS.clear()
            STOPS[STOPIDS[u'Thune']] = 'EW'
            print "Using preset 'Thune'"
        elif 'preset=cisco' in config:
            STOPS.clear()
            STOPS[STOPIDS[u'Oksenøyveien']] = 'E'
            STOPS[STOPIDS[u'Stabekkbakken']] = 'E'
        elif 'preset=brugata' in config:
            STOPS.clear()
            STOPS[STOPIDS[u'Brugata']] = 'NSEW'
            STOPS[STOPIDS[u'Hammersborggata arb samf']] = 'NSEW'
            STOPS[STOPIDS[u'Hammersborggata storgt']] = 'NSW'
        elif 'preset=torshov' in config:
            STOPS.clear()
            #STOPS[STOPIDS[u'Arendalsgata']] = 'NSEW'
            STOPS[STOPIDS[u'Torshov']] = 'NSEW'
        cfgFile.close()
    except IOError:
        print "No config preset found..."

readConfigPreset()
print "Using %s" % DISPLAY_MODE

def dateStrToStruct(datestring):
    return time.gmtime(int(datestring.split('(')[1].split('+')[0])/1000)

def sortBy(buses, paramKey):
    if paramKey == 'ExpectedArrivalTime':
        return sorted(buses, key=lambda bus: bus['ExpectedArrivalTime'])
    if paramKey == 'PublishedLineName':
        return sorted(buses, key=lambda bus: int(bus['PublishedLineName']))

def filterByDirection(buses, directionstr):
    print "filter called with directionstring: %s" % directionstr
    if 'e' not in directionstr.lower():
        buses = filter(lambda bus: int(bus['DirectionRef']) != 1, buses)
    if 'w' not in directionstr.lower() == 'w':
        buses = filter(lambda bus: int(bus['DirectionRef']) != 2, buses)
    return buses

def filterBusesByTimeLeftMoreThan(buses, secondsleft):
    return filter(lambda bus: secondsLeft(bus) > secondsleft, buses)

def filterBusesByTimeLeftLessThan(buses, secondsleft):
    return filter(lambda bus: secondsLeft(bus) < secondsleft, buses)

def compressName(name):
    name = name.split(' o/')[0]
    name = name.split(' via ')[0]
    name = name.replace('terminal', 'term')
    name = name.replace(' stasjon', '')
    name = name.replace('ekspress', 'eksp')
    name = name.split(' eksp')[0]
    return name

def funnifyName(bus):
    busNr = int(bus['PublishedLineName'])
    if busNr == 11 or busNr == 12 or busNr == 13:
        if int(bus['DirectionRef']) == 1:
            return "Hege"
    return bus['DestinationDisplay']

def fetchRealTimeData(stopid=0, compressNames=True, fake=False):
    if stopid == 0:
        return []
    trafikantenUrl = 'http://api-test.trafikanten.no/RealTime/GetRealTimeData/' + str(stopid)
    jsonfile = open('last_fetched.json', 'w')
    if fake:
        jsonstr = jsonfile.read()
    else:
        response = urllib2.urlopen(trafikantenUrl)
        jsonstr = response.read()
        jsonfile.write(jsonstr)
    jsonfile.close()
    if len(jsonstr) > 0:
        buses = json.loads(jsonstr)
    else:
        return []

    for bus in buses:
        bus['DestinationDisplay'] = compressName(bus['DestinationDisplay'])
        #bus['DestinationDisplay'] = funnifyName(bus)
        bus['AimedArrivalTime'] = dateStrToStruct(bus['AimedArrivalTime'])
        bus['AimedDepartureTime'] = dateStrToStruct(bus['AimedDepartureTime'])
        bus['ExpectedArrivalTime'] = dateStrToStruct(bus['ExpectedArrivalTime'])
        bus['ExpectedDepartureTime'] = dateStrToStruct(bus['ExpectedDepartureTime'])
        bus['RecordedAtTime'] = dateStrToStruct(bus['RecordedAtTime'])

    return buses

def secondsLeft(bus):
    waitTime = time.mktime(bus['ExpectedArrivalTime']) - time.mktime(bus['RecordedAtTime'])
    return int(round(waitTime))

def minutesLeft(bus):
    return int(secondsLeft(bus)/60)

def drawOnLine(lineNo, bus):
    display.setPos(x=0, y=8*lineNo)
    display.writeTxt(bus['PublishedLineName'])
    display.move(x=3)
    display.writeTxt(bus['DestinationDisplay'])
    waitTime = time.mktime(bus['ExpectedArrivalTime']) - time.mktime(bus['RecordedAtTime'])
    waitTime = int(round(waitTime)/60)
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

def departListToScrollText(busKey, depList):
    scrollText = busKey + ' '
    for moment in depList:
        scrollText += str(moment) + ','
    scrollText = scrollText[0:-1]
    scrollText += ' min'
    return scrollText

def scrollOnLine(lineNo, buses):
    depts = {}
    #print "Scrolling %d departures buses on line %d" % (len(buses), lineNo)
    for bus in buses:
        descr = bus['PublishedLineName'] + ' ' + bus['DestinationDisplay']
        if descr not in depts.keys():
            depts[descr] = []
        if len(depts[descr]) < 3: # TODO: un-hardcode!
            depts[descr].append(minutesLeft(bus))
    print "Scrolling %d buses. Hiding %d departures" % (len(depts), len(buses)-len(depts))
    busKeys = depts.keys()
    scrollText = ''
    for i in range(0, len(busKeys)):
        scrollText += '   ' + departListToScrollText(busKeys[i], depts[busKeys[i]])
        for i in range(128, -font.widthOfString(scrollText)-len(scrollText), -1):
            display.flush(lineNo)
            display.writeTxt(scrollText, xpos=i, ypos=lineNo*8)
            display.send(line=lineNo)
    return len(depts.keys())

def ascendOnLine(lineNo, buses):
    for bus in buses:
        print "Ascending %s on line %d" % (bus['PublishedLineName'] + " " + bus['DestinationDisplay'], lineNo)
        for i in range(0, 17):
            display.flush(lineNo)
            display.setPos(x=0, y=8*lineNo+(8-i))
            display.writeTxt(bus['PublishedLineName'])
            display.move(x=3)
            display.writeTxt(bus['DestinationDisplay'])
            waitTime = time.mktime(bus['ExpectedArrivalTime']) - time.mktime(bus['RecordedAtTime'])
            waitTime = int(round(waitTime)/60)
            digitpos = 113 - len(str(waitTime)) - font.widthOfString(str(waitTime))
            if waitTime > 0:
                display.writeTxt(str(waitTime), xpos=digitpos)
                display.writeTxt('min', xpos=113)
            else:
                display.writeTxt(u'nå', color='red', xpos=116)
            display.send(lineNo)
            if i == 8:
                time.sleep(1)
            else:
                time.sleep(0.03)

def ascendOnLines(lineNo, buses, lineSpan):
    lineSpan -= 1
    busCount = len(buses)
    print "Ascending %d buses on line %d to line %d" % (len(buses), lineNo, lineNo+lineSpan)
    for i in range(8*(lineSpan+1), -8*busCount-1, -1):
        display.flush()
        for j in range(0, len(buses)):
            display.setPos(x=0, y=lineNo*8+i+j*8)
            display.writeTxt(buses[j]['PublishedLineName'])
            display.move(x=3)
            display.writeTxt(buses[j]['DestinationDisplay'])
            waitTime = minutesLeft(buses[j])
            digitpos = 113 - len(str(waitTime)) - font.widthOfString(str(waitTime))
            if waitTime > 0:
                display.writeTxt(str(waitTime), xpos=digitpos)
                display.writeTxt('min', xpos=113)
            else:
                display.writeTxt(u'nå', color='red', xpos=116)
        for j in range(lineNo, lineNo+lineSpan+1):
            display.send(j)
        if i % 16 == 0:
            time.sleep(1)

def dumpBusList(buses):
    for bus in buses:
        print "%s %s (direction=%s) %d min (%d sec)" % (bus['PublishedLineName'], bus['DestinationDisplay'], bus['DirectionRef'], minutesLeft(bus), secondsLeft(bus))


font = LedFont()
display = LedDisplay(font, lines=DISPLAY_LINES, device=DEVICE)

display.writeTxt("Line 1", xpos=0, ypos=0, color='red')
display.writeTxt("Line 2", xpos=0, ypos=8, color='green')
display.writeTxt("Line 3", xpos=0, ypos=16, color='orange')
display.writeTxt("Line 4", xpos=0, ypos=24, color='green')
display.send()
time.sleep(1)
for i in range(100, -1, -4):
    display.flush(DISPLAY_LINES-1)
    display.writeTxt("Scrolling", xpos=i, ypos=(DISPLAY_LINES-1)*8, color='green')
    display.send(DISPLAY_LINES-1)

while True:
    display.flush()
    scrollCount = 0
    lastRequestTime = time.time()
    print ""
    try:
        buses = []
        for stopId in STOPS:
            print "Performing HTTP request for",
            print "<" + stopIdToName(stopId) + ">"
            buses += filterByDirection(fetchRealTimeData(stopId), STOPS[stopId])
            dumpBusList(buses)
        #buses = filterBusesByTimeLeftMoreThan(buses, 120)
        if len(buses) > 0:
            timeSorted = sortBy(buses, 'ExpectedArrivalTime')
            print "Fetched %s buses, displaying using %s" % (len(buses), DISPLAY_MODE)
            if DISPLAY_MODE == MODE_STATIC:
                for i in range(0, min(DISPLAY_LINES, len(timeSorted))):
                    drawOnLine(i, timeSorted[i])
                display.send()
            elif DISPLAY_MODE == MODE_HSCROLL:
                print "Horizontal"
                for i in range(0, min(DISPLAY_LINES-1, len(timeSorted))):
                    drawOnLine(i, timeSorted[i])
                display.send()
                rest = timeSorted[DISPLAY_LINES-1:]
                rest = sortBy(rest, 'PublishedLineName')
                scrollCount = scrollOnLine(DISPLAY_LINES-1, rest)
            elif DISPLAY_MODE == MODE_ASCEND:
                print "Using ascent"
                #for i in range(8*4, -8*4*3, -1):
                #    display.flush()
                #    for j in range(0, 8):
                #        display.writeTxt('Welcome to ', xpos=5, ypos=i+10*j, color="red")
                #        display.writeTxt('NARNIAFEST', color=random.sample(["red", "green", "orange"], 1)[0])
                #    display.send()
                busesSoon = filterBusesByTimeLeftLessThan(timeSorted, 60*20)
                if len(busesSoon) < 20:
                    timeSorted = timeSorted[0:20]
                else:
                    timeSorted = busesSoon
                ascendOnLines(0, timeSorted, DISPLAY_LINES)
                ageOfLastRequest = time.time() - lastRequestTime
                if MINIMUM_REQUEST_INTERVAL - ageOfLastRequest > 4:
                    for i in range(0, min(DISPLAY_LINES, len(timeSorted))):
                        drawOnLine(i, timeSorted[i])
        else:
            display.writeTxt("Se tidtabell", color='red')
            print "Fetched empty list of departures"
            display.send()
            time.sleep(5)
    except IOError:
        print "Failed to fetch data"
        display.writeTxt("Offline...", color='red')
    ageOfLastRequest = time.time() - lastRequestTime
    delay = MINIMUM_REQUEST_INTERVAL - ageOfLastRequest
    print "%d seconds since last request. Sleeping %d seconds..." % (ageOfLastRequest, max(0, delay))
    if delay > 0:
        time.sleep(delay)
