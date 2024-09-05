#!/usr/bin/env bash
# Remember to install libcurlpp-dev libjsoncpp-dev
set -x
g++ -g -Wall -Wextra -Werror -std=c++17 sprite.cpp leddisplay.cpp http_fetchers.cpp command_line.cpp main.cpp -lcurl -ljsoncpp -lncurses
