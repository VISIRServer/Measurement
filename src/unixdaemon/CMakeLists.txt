cmake_minimum_required(VERSION 2.8)
include_directories (.. ../util)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )

ADD_EXECUTABLE( measureserver_daemon main.cpp )
TARGET_LINK_LIBRARIES( measureserver_daemon

	contrib
	httpserver
	scgiserver
	instruments
	measureserver
	network
	protocol
	util
	xmlprotocol
	xmlserver
	xmlutil
	
	${EXPAT_LIBRARY}
	)