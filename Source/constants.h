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

#include <math.h>

//#ifndef USE_SIMPLEST_LP
//#define USE_SIMPLEST_LP 1
//#endif

//these should probably be a macro, to prevent use of #includes in some places
const bool k_bUseSampledSound = false;
const bool  k_bUseWaveTables = true;


//--------useful functions
//convert p_tValue01 from range [0,1] to human-readable range [p_tMinHr, p_tMaxHr]
template <typename T>
static T const& convert01ToHr (T const& p_tValue01, T const& p_tMinHr, T const& p_tMaxHr) { 
    return p_tValue01*(p_tMaxHr-p_tMinHr) + p_tMinHr; 
} 

//convert p_tValue01 from human-readable range [p_tMinHr, p_tMaxHr] to range [0,1] 
template <typename T>
static T const& convertHrTo01 (T const& p_tValueHr, T const& p_tMinHr, T const& p_tMaxHr) { 
    return (p_tValueHr - p_tMinHr) / (p_tMaxHr-p_tMinHr); 
} 

static bool areSame(double a, double b){
    return fabs(a - b) < .0001;//std::numeric_limits<double>::epsilon();
}

//-------enums
enum Parameters{
     paramGain = 0
    ,paramDelay
    ,paramWave
	,paramFilterFr
	,paramLfoFr
	,paramQ
	,paramLfoOn
	,paramSubOscOn
    ,paramTotalNum
};

enum WaveTypes{
	triangleWave
	, sawtoothWave
	, squareWave
	, totalWaveTypes
};

const float k_fDefaultGain		= 1.0f;
const float k_fDefaultDelay		= 0.0f;
const float k_fDefaultWave		= 0.0f;

//----FILTER FR
const float k_fDefaultFilterFr	= 0.0f;

//----FILTER Q
const float k_fMinQHr			= 0.01f;
const float k_fMaxQHr			= 20.f;
const float k_fDefaultQHr		= 5.f;
const float k_fDefaultQ01		= convertHrTo01(k_fDefaultQHr, k_fMinQHr, k_fMaxQHr);

//----LFO
const float k_fMinLfoFr			= 0.5f;
const float k_fMaxLfoFr			= 40.f;
const float k_fDefaultLfoFrHr	= 2.;
const float k_fDefaultLfoFr01	= convertHrTo01(k_fDefaultLfoFrHr, k_fMinLfoFr, k_fMaxLfoFr);

const float k_fDefaultLfoOn		= 0.;

const int   k_iSimpleFilterLF = 600;
const int   k_iSimpleFilterHF = 20000;// 12000;
const int   k_iNumberOfVoices = 10;

//-------stuff related to wavetables
const int   k_iOverSampleFactor	= 2;     /* oversampling factor (positive integer) */
const float k_iBaseFrequency	= 20.f;  /* starting frequency of first table */

//-------stuff related to size of GUI things
const int k_iXMargin		= 20;
const int k_iYMargin		= 25;
const int k_iKeyboardHeight	= 70;
const int k_iSliderWidth	= 75;
const int k_iSliderHeight	= 40;
const int k_iLabelHeight	= 20;
const int k_iLogoW			= 75;
const int k_iLogoH			= 30;

const int k_iNumberOfHorizontalSliders	= 4;
const int k_iNumberOfVerticaltalSliders = 2;

#endif
