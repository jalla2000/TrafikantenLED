#!/usr/bin/env bash
# Remember to install libcurlpp-dev libjsoncpp-dev
g++ -g -Wall -Wextra -std=c++17 sprite.cpp leddisplay.cpp http_fetchers.cpp main.cpp -lcurl -ljsoncpp -lncurses
