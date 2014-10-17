#!/usr/bin/env python
# -*- coding: utf-8 -*-
import urllib2
import time
import json
import sys
from ledfont import LedFont
from leddisplay import LedDisplay
import random

font = LedFont()
display = LedDisplay(font, lines=4, device="/dev/ttyUSB0")

color = 'orange'
display.writeTxt("LL 20 Galgeberg 8min  LL 20 Galgeberg 8min", xpos=0, ypos=0, color=color)
display.writeTxt("LL 20 Galgeberg 8min  LL 20 Galgeberg 8min", xpos=0, ypos=8, color=color)
display.writeTxt("LL 20 Galgeberg 8min  LL 20 Galgeberg 8min", xpos=0, ypos=16, color=color)
display.writeTxt("LL 20 Galgeberg 8min  LL 20 Galgeberg 8min", xpos=0, ypos=24, color=color)
display.send()
