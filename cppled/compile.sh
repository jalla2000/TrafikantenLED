#!/usr/bin/env bash
# Remember to install libboost-dev libcurlpp-dev libjsoncpp-dev
g++ -g -Wall -Wextra -std=c++14 sprite.cpp leddisplay.cpp trafikanten.cpp -lcurl -ljsoncpp -lncurses
