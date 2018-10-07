cmake_minimum_required(VERSION 2.8)
include_directories (.. ../util)

ADD_LIBRARY( httpserver STATIC
		httpconnection.cpp
		httpconnection.h
		httprequest.cpp
		httprequest.h
		httpserver.cpp
		httpserver.h
		)