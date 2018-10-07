cmake_minimum_required(VERSION 2.8)

#SET( BIN_FOLDER "../bin" )

SET(CMAKE_CXX_FLAGS "-Wall -Wnon-virtual-dtor")

FIND_PACKAGE(Expat REQUIRED)
MESSAGE("Found Expat headers in ${EXPAT_INCLUDE_DIR}, library at ${EXPAT_LIBRARIES}")

SUBDIRS( contrib eqcom httpserver scgiserver instruments measureserver network protocol util xmlprotocol xmlserver xmlutil circuittester unixdaemon )