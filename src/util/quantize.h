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
 * Copyright (c) 2009 Johan Zackrisson
 * All Rights Reserved.
 */

#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

#include <limits>
#include <vector>

namespace Quantizer
{

template <typename To>
std::vector<To> Quantize(const std::vector<double>& in, double& gain, double& offset)
{
	double min = std::numeric_limits<double>::max();
	double max = std::numeric_limits<double>::min();

	for(size_t i = 0; i<in.size(); i++)
	{
		double sample = in[i];
		if (sample < min) min = sample;
		if (sample > max) max = sample;
	}

	gain = (max - min) / std::numeric_limits<To>::max();
	offset = min;

	std::vector<To> out;
	out.resize(in.size());
	for(size_t i = 0; i<in.size(); i++)
	{
		if (gain != 0.0)
		{
			double sample = (in[i] - offset) / gain;
			out[i] = To(sample + 0.5); // need rounding?
		}
		else
		{
			out[i] = 0;
		}
	}
	return out;
}

template <typename From>
std::vector<double> Expand(const std::vector<From>& in, double gain, double offset)
{
	std::vector<double> out;
	out.resize(in.size());
	for(size_t i = 0; i<in.size(); i++)
	{
		out[i] = (double(in[i]) * gain) + offset;
	}
	return out;
}

} // end of namespace

#endif