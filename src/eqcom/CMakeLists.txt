cmake_minimum_required(VERSION 2.8)
include_directories (.. ../util)

SET( LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin )
#SET( DEBUG_POSTFIX "_debug" )
#SET( PREFIX "" )

ADD_LIBRARY( eqcom SHARED
	circuit.cpp
	circuit.h
	commands.cpp
	commands.h
	connection.cpp
	connection.h
	control.cpp
	control.h
	dllmain.cpp
	eqlog.cpp
	eqlog.h
	eqmodule.cpp
	eqmodule.h
	experiment.cpp
	experiment.h
	header.cpp
	header.h
	measurement.cpp
	measurement.h
	response.cpp
	response.h
	setupresponse.cpp
	setupresponse.h
	symbols.h
	transactions.cpp
	transactions.h
	)

TARGET_LINK_LIBRARIES( eqcom
	contrib
	instruments
#	measureserver
	network
	protocol
	util
#	xmlutil
	)

SET_TARGET_PROPERTIES( eqcom PROPERTIES 
		LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin
		PREFIX ""
		SUFFIX ".module"
		DEBUG_POSTFIX "_debug"
	 	)