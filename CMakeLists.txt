cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(moba-connector VERSION 1.0.0)

configure_file(config.h.in config.h)

add_executable(
    moba-connector 

    src/brakevector.cpp
    src/main.cpp
    src/jsonreader.cpp
    src/jsonwriter.cpp
    src/switchingoutputshandler.cpp
    src/watchdog.cpp
)

install(TARGETS moba-connector)

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

target_include_directories(moba-connector PUBLIC "${PROJECT_BINARY_DIR}")

target_link_libraries(moba-connector ncurses)
target_link_libraries(moba-connector mobacommon)
target_link_libraries(moba-connector z)
target_link_libraries(moba-connector ${CMAKE_SOURCE_DIR}/modules/lib-msghandling/libmoba-lib-msghandling.a)
target_link_libraries(moba-connector ${CMAKE_SOURCE_DIR}/modules/lib-cs2interface/libmoba-lib-cs2interface.a)

include_directories(${CMAKE_SOURCE_DIR}/modules/lib-msghandling/src)
include_directories(${CMAKE_SOURCE_DIR}/modules/lib-cs2interface/src)

#AM_CPPFLAGS = \
#    $(GTKMM_CFLAGS) \
#    -I$(srcdir)/../modules/lib-msghandling/src \
#    -I$(srcdir)/../modules/lib-tracklayout/src \
#    -g3 -g -ggdb  -Wall
#
### Process this file with automake to generate Makefile.in
#SUBDIRS = \
#    modules/lib-msghandling/src/moba \
#    modules/lib-tracklayout/src/moba \
#    src
#
