cmake_minimum_required(VERSION 2.8)
include_directories (.. ../util)

ADD_LIBRARY( network STATIC
	consumer.cpp
	iobuffer.h
	connection.cpp
	consumer.h
	multiplexer.cpp
	server.cpp
	socket.h
	socket_unix.cpp
	connection.h
	iobuffer.cpp
	multiplexer.h
	server.h
	sockethandler.h
	)
