cmake_minimum_required(VERSION 2.8)
include_directories (.. ../util)

ADD_LIBRARY( scgiserver STATIC
		scgiconnection.cpp
		scgiconnection.h
		scgirequest.cpp
		scgirequest.h
		scgiserver.cpp
		scgiserver.h
		)