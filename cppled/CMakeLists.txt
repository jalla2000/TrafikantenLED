cmake_minimum_required(VERSION 3.10)

# Project name and version
project(TrafikantenLED VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
set(SOURCES
    sprite.cpp
    leddisplay.cpp
    http_fetchers.cpp
    command_line.cpp
    tests.cpp
    main.cpp
)

# Add the executable
add_executable(trafikanten ${SOURCES})

# Link the external libraries
find_package(CURL REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(Curses REQUIRED)

# target_link_libraries(trafikanten PRIVATE CURL::libcurl JsonCpp::JsonCpp ${CURSES_LIBRARIES})
target_link_libraries(trafikanten PRIVATE CURL::libcurl jsoncpp ${CURSES_LIBRARIES})

# Enable warnings and debugging options
target_compile_options(trafikanten PRIVATE -g -Wall -Wextra -Werror -Wno-error=deprecated-declarations)
