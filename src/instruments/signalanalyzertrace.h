/**** BEGIN LICENSE BLOCK ****
 * This file is a part of the VISIR(TM) (Virtual Systems in Reality)
 * Software package.
 * 
 * VISIR(TM) is used to open laboratories for remote operation and control
 * as a supplement and a complement to local use.
 * 
 * VISIR(TM) is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. No liability
 * can be imposed for any impact on any equipment by the software. See
 * the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **** END LICENSE BLOCK ****/

/*
 * Copyright (c) 2004-2009 Johan Zackrisson
 * All Rights Reserved.
 */

#ifndef __SIGNALANALYZERTRACE_H__
#define __SIGNALANALYZERTRACE_H__

#include "instrument.h"

#include <setget.h>
#include <vector>
#include <string>

class SignalAnalyzerTrace
{
public:
	SET_GET(int,			Channel,	mChannel);
	SET_GET(std::string,	Measure,	mMeasure);
	SET_GET(std::string,	Format,		mFormat);
	SET_GET(std::string,	XSpacing,	mXSpacing);
	SET_GET(int,			AutoScale,	mAutoScale);
	SET_GET(std::string,	Scale,		mScale);
	SET_GET(double,			ScaleDiv,	mScaleDiv);
	SET_GET(std::string,	VoltUnit,	mVoltUnit);

	// <output only>
	SET_GET(double,			YTop,		mYTop);
	SET_GET(double,			YBottom,	mYBottom);
	SET_GET(double,			XLeft,		mXLeft);
	SET_GET(double,			XRight,		mXRight);
	SET_GET(std::string,	DispXUnit,	mDispXUnit);
	SET_GET(std::string,	DispYUnit,	mDispYUnit);
	// </output only>

	typedef std::vector<double> tGraph;
	void			SetGraph(tGraph graph)	{ mGraph = graph; }
	const tGraph&	GetGraph()				{ return mGraph; }

	// maybe these should be a array of graphs instead
	void			SetGraphY(tGraph graph)	{ mGraphY = graph; }
	const tGraph&	GetGraphY()				{ return mGraphY; }

	bool	Validate();
	void	CopyFrom(SignalAnalyzerTrace* pTrace);
			SignalAnalyzerTrace();
private:
	int			mChannel;
	std::string	mMeasure;
	std::string	mFormat;
	std::string	mXSpacing;
	int			mAutoScale;
	std::string	mScale;
	double		mScaleDiv;
	std::string	mVoltUnit;

	// out params
	double	mYTop;
	double	mYBottom;
	double	mXLeft;
	double	mXRight;
	tGraph	mGraph;
	tGraph	mGraphY;

	std::string	mDispXUnit, mDispYUnit;
};

#endif
