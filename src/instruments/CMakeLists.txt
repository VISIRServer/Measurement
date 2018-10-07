cmake_minimum_required(VERSION 2.8)
include_directories (.. ../util)

ADD_LIBRARY( instruments STATIC
		channel.cpp
		channel.h
		circuitlist.cpp
		circuitlist.h
		circuitsolver3.cpp
		circuitsolver3.h
		circuitsolverinternal.h
		circuitsymbols2.cpp
		circuitsymbols2.h
		compdefreader.cpp
		compdefreader.h
		connectionpoint.cpp
		connectionpoint.h
		digitalmultimeter.cpp
		digitalmultimeter.h
		functiongenerator.cpp
		functiongenerator.h
		instrument.cpp
		instrument.h
		instrumentblock.cpp
		instrumentblock.h
		instruments.vcproj
		listalgorithm.cpp
		listalgorithm.h
		listcomponent.h
		listcomponent.cpp
		listparser.cpp
		listparser.h
		listproducer.cpp
		listproducer.h
		measurement.cpp
		measurement.h
		netlist2.cpp
		netlist2.h
		nodeinterpreter.cpp
		nodeinterpreter.h
		oscilloscope.cpp
		oscilloscope.h
		signalanalyzer.cpp
		signalanalyzer.h
		signalanalyzerchannel.cpp
		signalanalyzerchannel.h
		signalanalyzertrace.cpp
		signalanalyzertrace.h
		trigger.cpp
		trigger.h
		tripledc.cpp
		tripledc.h
		validate.cpp
		validate.h
		visitor.h

		)
