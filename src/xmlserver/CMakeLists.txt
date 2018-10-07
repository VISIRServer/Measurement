cmake_minimum_required(VERSION 2.8)
include_directories (.. ../util)

ADD_LIBRARY( xmlserver STATIC
		xmlconnection.cpp
		xmlconnection.h
		xmlserver.cpp
		xmlserver.h
		)