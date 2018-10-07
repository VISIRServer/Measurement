cmake_minimum_required(VERSION 2.8)
include_directories (.. ../util)

ADD_LIBRARY( xmlprotocol STATIC
		producer.h
		producer.cpp
		requestparser.h
		requestparser.cpp
		xmltools.h
		xmltools.cpp
		xmlcommands.h
		xmlcommands.cpp
		)