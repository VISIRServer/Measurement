cmake_minimum_required(VERSION 2.8)
# include_directories ()

SOURCE_GROUP(Log FILES
		logmodule.cpp
		logmodule.h
		syslog.cpp
		syslog.h
		)

ADD_LIBRARY( util STATIC
		basic_exception.h
		config.cpp
		config.h
		dynlib.cpp
		dynlib.h
		logmodule.cpp
		logmodule.h
		observable.cpp
		observable.h
		quantize.h
		scoped_ptr.h
		serializer.cpp
		serializer.h
		setget.h
		stringop.h
		syslog.cpp
		syslog.h
		timer.cpp
		timer.h
		)