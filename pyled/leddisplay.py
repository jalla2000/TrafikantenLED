#!/usr/bin/env python
# -*- coding: utf-8 -*-

import serial

class LedDisplay():
    def __init__(self, font, lines=4, device='/dev/ttyUSB0'):
        self.connection = serial.Serial(port=device,
                                        #baudrate=921600,
                                        baudrate=460800,
                                        #baudrate=230400,
                                        #baudrate=153600,
                                        #baudrate=115200,
                                        xonxoff=True,
                                        timeout=1)
        self.PIXELS_PER_LINE = 8
        self.font = font
        self.lines = lines
        self.gfxBuffer = None
        self.flush()
        self.writeTxt('-')
        self.send()

    def setByte(self, row, col, data):
        self.gfxBuffer[row][col] = data

    def setPixel(self, xpos, ypos, color='red'):
        col = xpos / 4
        fraction = (xpos % 4)*2
        if color == 'red':
            fraction += 1
        self.gfxBuffer[ypos][col] |= 1<<(7-fraction)
        if color == 'orange':
            self.gfxBuffer[ypos][col] |= 1<<(7-fraction-1)

    def flush(self, line=-1):
        if line == -1:
            self.gfxBuffer = [None]*self.lines*8
            for i in range(0, len(self.gfxBuffer)):
                self.gfxBuffer[i] = [0b00000000]*32
            self.currentX = 0
            self.currentY = 0
        elif line < self.lines:
            for i in range(8*line, 8*(line+1)):
                self.gfxBuffer[i] = [0b00000000]*32

    def move(self, x=0, y=0):
        self.currentX += x
        self.currentY += y

    def setPos(self, x=None, y=None):
        #print "Setting position to", x, y
        if x is not None:
            self.currentX = x
        if y is not None:
            self.currentY = y

    def writeTxt(self, text, color='orange', xpos=None, ypos=None):
        if text.lower() == 'hege':
            color='green'
        if xpos is not None:
            self.currentX = xpos
        if ypos is not None:
            self.currentY = ypos
        #print "Writing '%s' to posistion (%d,%d)" % (text, self.currentX, self.currentY)
        colorfilter = 0b11111111
        if color.lower() == 'green':
            colorfilter = 0b10101010
        elif color.lower() == 'red':
            colorfilter = 0b01010101

        for letter in text:
            for row in range(0, len(self.font[letter])):
                #print "Current x/y=%d/%d" % (self.currentX, self.currentY)
                col = self.currentX / 4
                fraction = self.currentX % 4
                if (row+self.currentY >= 0 and row+self.currentY < self.lines*self.PIXELS_PER_LINE):
                    if (col >= 0 and col < 32):
                        self.gfxBuffer[row+self.currentY][col] |= self.font[letter][row][0]>>(fraction*2) & colorfilter
                    if (col >= -1 and col < 31):
                        self.gfxBuffer[row+self.currentY][col+1] |= self.font[letter][row][0]<<(8-fraction*2) & colorfilter
                        self.gfxBuffer[row+self.currentY][col+1] |= self.font[letter][row][1]>>(fraction*2) & colorfilter
                    if (col >= -2 and col < 30):
                        self.gfxBuffer[row+self.currentY][col+2] |= self.font[letter][row][1]<<(8-fraction*2) & colorfilter
            self.currentX += self.font.widths[letter]+1

    def drawSprite(self, sprite, xpos, ypos, color='green'):
        colorfilter = 0b11111111
        if color.lower() == 'green':
            colorfilter = 0b10101010
        elif color.lower() == 'red':
            colorfilter = 0b01010101

        fraction = (xpos % 4) * 2
        for row in range(0, len(sprite)):
            # check row boundaries
            if (ypos+row >= 0 and ypos+row < self.lines*self.PIXELS_PER_LINE):
                for col in range(0, len(sprite[0])):
                    targetcol = xpos/4+col
                    # check column boundaries
                    if targetcol >= 0 and targetcol < len(self.gfxBuffer[row+ypos]):
                        self.gfxBuffer[row+ypos][targetcol] |= (sprite[row][col] >> fraction) & colorfilter
                    # check column boundaries
                    if targetcol+1 >= 0 and targetcol+1 < len(self.gfxBuffer[row+ypos]):
                        self.gfxBuffer[row+ypos][targetcol+1] |= (sprite[row][col] << (8-fraction)) & colorfilter

    def send(self, line=-1):
        if line == -1:
            dataChunk = '0'
            for row in self.gfxBuffer:
                dataChunk += "".join([chr(entry) for entry in row])
            self.connection.write(dataChunk)
        elif line < self.lines:
            dataChunk = str(line+1)
            for i in range(self.PIXELS_PER_LINE*line, self.PIXELS_PER_LINE*(line+1)):
                dataChunk += "".join([chr(entry) for entry in self.gfxBuffer[i]])
            self.connection.write(dataChunk)

    def __del__(self):
        self.connection.close()
        print "Closing display connection"
