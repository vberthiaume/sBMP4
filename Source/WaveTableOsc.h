/*
 ==============================================================================
 sBMP4: killer subtractive synth!
 
 Copyright (C) 2016  BMP4
 
 Developer: Vincent Berthiaume, building on code from Nigel Redmon, see original license below.
 
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

//
//  WaveTableOsc.h
//
//  Created by Nigel Redmon on 5/15/12
//  EarLevel Engineering: earlevel.com
//  Copyright 2012 Nigel Redmon
//
//  For a complete explanation of the wavetable oscillator and code,
//  read the series of articles by the author, starting here:
//  www.earlevel.com/main/2012/05/03/a-wavetable-oscillator—introduction/
//
//  License:
//
//  This source code is provided as is, without warranty.
//  You may copy and distribute verbatim copies of this document.
//  You may modify and use this source code to create binary code for your own purposes, free or commercial.
//

#ifndef Test_WaveTableOsc_h
#define Test_WaveTableOsc_h

#define doLinearInterp 1
#include <vector>
#include "constants.h"

//TODO: use vectors instead of an array
typedef struct {
    double topFreq;
    int waveTableLen;
    std::vector<float> waveTable;
} waveTable;

const int numWaveTableSlots = 32;

class WaveTableOsc {
	void fft(int N);
	void defineSawtoothPartials(int len, int numHarmonics);
	void defineSquarePartials(  int len, int numHarmonics);
	void defineTrianglePartials(int len, int numHarmonics);
	float makeWaveTable(int len, double scale, double topFreq);
	int addWaveTable(	int len, std::vector<float> waveTableIn, double topFreq);

    double phasor;      // phase accumulator
    double phaseInc;    // phase increment
    double phaseOfs;    // phase offset for PWM
    
    // list of wavetables
    int numWaveTables;
    waveTable m_oWaveTables[numWaveTableSlots];
	std::vector<double> m_vPartials;	//is this real amplitude and ai imaginary amplitude?
	std::vector<double> m_vWave; 
    
public:
	WaveTableOsc(const int, const WaveTypes);
    void  setFrequency(double inc);
    void  setPhaseOffset(double offset);
    void  updatePhase(void);
    float getOutput(void);
    float getOutputMinusOffset(void);
};


// note: if you don't keep this in the range of 0-1, you'll need to make changes elsewhere
inline void WaveTableOsc::setFrequency(double inc) {
    phaseInc = inc;
}

// note: if you don't keep this in the range of 0-1, you'll need to make changes elsewhere
inline void WaveTableOsc::setPhaseOffset(double offset) {
    phaseOfs = offset;
}

inline void WaveTableOsc::updatePhase() {
    phasor += phaseInc;
    
    if (phasor >= 1.0)
        phasor -= 1.0;
}

#endif
