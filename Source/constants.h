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

 //#ifndef USE_LFO
 //#define USE_LFO 1
 //#endif

#ifndef USE_LFO_IN_VOICE
#define USE_LFO_IN_VOICE 1
#endif


enum Parameters{
     paramGain = 0
    ,paramDelay
    ,paramWave
	,paramFilterFr
	,paramLfoFr
    ,paramTotalNum
};

const float defaultGain		= 1.0f;
const float defaultDelay	= 0.0f;
const float defaultWave		= 0.0f;
const float defaultFilterFr = 0.0f;

const int   s_iSimpleFilterLF = 600;
const int   s_iSimpleFilterHF = 20000;// 12000;

const int   s_iNumberOfVoices = 5;
#if	WIN32
const bool  s_bUseSimplestLp = false;
#else
const bool  s_bUseSimplestLp = true;
#endif
//-------stuff related to wavetables
const bool  s_bUseWaveTables = false;
const int   kTotalWaveFrames = 4096;		// samples (must be power of 2 here)

static bool areSame(double a, double b){
    return fabs(a - b) < .0001;//std::numeric_limits<double>::epsilon();
}

#endif
