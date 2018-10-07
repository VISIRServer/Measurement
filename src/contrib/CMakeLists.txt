cmake_minimum_required(VERSION 2.8)
# include_directories ()

ADD_LIBRARY( contrib STATIC
		base64.cpp
		base64.h
#		crashinfo.cpp
#		crashinfo.h
		md5.c
		md5.h
		)