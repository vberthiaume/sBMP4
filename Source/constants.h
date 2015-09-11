/*
 ==============================================================================
 sBMP4: kilker subtractive synth!
 
 Copyright (C) 2014  BMP4
 
 Developer: Vincent Berthiaume
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#ifndef sBMP4_Header_h
#define sBMP4_Header_h

enum Parameters{
     paramGain = 0
    ,paramDelay
    ,paramWave
	,paramFilterFr
    ,paramTotalNum
};

const float defaultGain		= 1.0f;
const float defaultDelay	= 0.0f;
const float defaultWave		= 0.0f;
const float defaultFilterFr = 0.0f;

const int   s_iSimpleFilterLF = 600;
const int   s_iSimpleFilterHF = 12000;

const int s_iNumberOfVoices = 5;
const float s_bUseSimplestLp = false;

static bool areSame(double a, double b)
{
    return fabs(a - b) < .0001;//std::numeric_limits<double>::epsilon();
}

#endif
