cmake_minimum_required(VERSION 2.8)
include_directories (..)

SOURCE_GROUP(DOM FILES
		domparser.h
		domparser.cpp
		)

SOURCE_GROUP(Soap FILES
		soapresponse.h
		soapresponse.cpp
		soapresponseparser.h
		soapresponseparser.cpp
		)

ADD_LIBRARY( xmlutil STATIC
		domparser.h
		domparser.cpp
		soapresponse.h
		soapresponse.cpp
		soapresponseparser.h
		soapresponseparser.cpp
		xmlparser.h
		xmlparser.cpp
		)
