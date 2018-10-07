cmake_minimum_required(VERSION 2.8)
include_directories (.. ../util)

ADD_LIBRARY( measureserver STATIC
		authentication.cpp
		authentication.h
		authentry.h
		client.cpp
		client.h
		clientmanager.cpp
		clientmanager.h
		maxlists.cpp
		maxlists.h
		module.h
		moduleregistry.cpp
		moduleregistry.h
		protocolservice.cpp
		protocolservice.h
		request.cpp
		request.h
		requestqueue.cpp
		requestqueue.h
		servermain.cpp
		servermain.h
		service.cpp
		service.h
		session.cpp
		session.h
		systemtransactions.cpp
		systemtransactions.h
		transactioncontrol.cpp
		transactioncontrol.h
		transactionrequest.cpp
		transactionrequest.h
		version.cpp
		version.h

)