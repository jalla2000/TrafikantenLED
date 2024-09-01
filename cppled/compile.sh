#!/usr/bin/env bash
# Remember to install libboost-dev libcurlpp-dev libjsoncpp-dev
g++ -g -Wall -Wextra -std=c++14 sprite.cpp leddisplay.cpp http_fetchers.cpp main.cpp -lcurl -ljsoncpp -lncurses
