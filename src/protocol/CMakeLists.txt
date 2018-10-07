cmake_minimum_required(VERSION 2.8)
include_directories (.. ../util)

ADD_LIBRARY( protocol STATIC
		auth.cpp
		auth.h
		basic_types.h
		protocol.cpp
		protocol.h
		)