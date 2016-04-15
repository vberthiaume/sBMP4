//
//  WaveTableOsc.cpp
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

#include "WaveTableOsc.h"
#include "../JuceLibraryCode/JuceHeader.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// set to a large number (greater than or equal to the length of the lowest octave table) for constant table size; 
//set to 0 for a constant oversampling ratio (each higher ocatave table length reduced by half); set somewhere
//between (64, for instance) for constant oversampling but with a minimum limit
#define constantRatioLimit (99999)    

using namespace std;

// I grabbed (and slightly modified) this code from Rabiner & Gold (1975), After Cooley, Lewis, and Welch; 
void fft(int N, vector<double> &ar, vector<double> &ai) {    
    int i, j, k, L;            /* indexes */
    int M, TEMP, LE, LE1, ip;  /* M = log N */
    int NV2, NM1;
    double t;               /* temp */
    double Ur, Ui, Wr, Wi, Tr, Ti;
    double Ur_old;
    
    // if ((N > 1) && !(N & (N - 1)))   // make sure we have a power of 2
    NV2 = N >> 1;
    NM1 = N - 1;
    TEMP = N; /* get M = log N */
    M = 0;
    while (TEMP >>= 1){
		++M;
	}
    /* shuffle */
    j = 1;
    for (i = 1; i <= NM1; i++) {
        if(i<j) {             /* swap a[i] and a[j] */
            t = ar[j-1];     
            ar[j-1] = ar[i-1];
            ar[i-1] = t;
            t = ai[j-1];
            ai[j-1] = ai[i-1];
            ai[i-1] = t;
        }
        k = NV2;             /* bit-reversed counter */
        while(k < j) {
            j -= k;
            k /= 2;
        }  
        j += k;
    }
    LE = 1.;
    for (L = 1; L <= M; L++) {            // stage L
        LE1 = LE;                         // (LE1 = LE/2) 
        LE *= 2;                          // (LE = 2^L)
        Ur = 1.0;
        Ui = 0.; 
        Wr = cos(M_PI/(float)LE1);
        Wi = -sin(M_PI/(float)LE1); // Cooley, Lewis, and Welch have "+" here
        for (j = 1; j <= LE1; j++) {
            for (i = j; i <= N; i += LE) { // butterfly
                ip = i+LE1;
                Tr = ar[ip-1] * Ur - ai[ip-1] * Ui;
                Ti = ar[ip-1] * Ui + ai[ip-1] * Ur;
                ar[ip-1] = ar[i-1] - Tr;
                ai[ip-1] = ai[i-1] - Ti;
                ar[i-1]  = ar[i-1] + Tr;
                ai[i-1]  = ai[i-1] + Ti;
            }
            Ur_old = Ur;
            Ur = Ur_old * Wr - Ui * Wi;
            Ui = Ur_old * Wi + Ui * Wr;
        }
    }
}

WaveTableOsc::WaveTableOsc(void) {
    phasor = 0.0;		// phase accumulator
    phaseInc = 0.0;		// phase increment
    phaseOfs = 0.5;		// phase offset for PWM
    numWaveTables = 0;	//why is this 0?

	//initialize the array of waveTable structures (which could be replaced by vectors...)
    for (int idx = 0; idx < numWaveTableSlots; idx++) {
        m_oWaveTables[idx].topFreq = 0;
        m_oWaveTables[idx].waveTableLen = 0;
        //m_oWaveTables[idx].waveTable = 0;
    }
}

WaveTableOsc::WaveTableOsc(const float baseFreq, const int sampleRate, const WaveTypes waveType) 
	: WaveTableOsc() {
	//TODO: understand this
    // calc number of harmonics where the highest harmonic baseFreq and lowest alias an octave higher would meet
    int maxHarms = sampleRate / (3.0 * baseFreq) + 0.5;	//maxHarms = 735

	//TODO: find less opaque way of doing this, check aspma notes
    // round up to nearest power of two
    unsigned int v = maxHarms;
    v--;            // so we don't go up if already a power of 2
    v |= v >> 1;    // roll the highest bit into all lower bits...
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;            // and increment to power of 2
	int overSamp = 2;
    int tableLen = v * 2 * overSamp;  // double for the sample rate, then oversampling, tablelen = 4096

    // for ifft
	vector<double> ar(tableLen);	//is this real amplitude and ai imaginary amplitude?
	vector<double> ai(tableLen);   

	//calculate topFrequency based on Nyquist and base frequency... 
	//TODO: why is base frequency relevant here?
    double topFreq = baseFreq * 2.0 / sampleRate;	//topFreq = 0.00090702947845804993
    double scale = 0.0;
    for (; maxHarms >= 1; maxHarms /= 2) {
		//fill ar with partial amplitudes for a sawtooth. This will be ifft'ed to get a wave
		switch (waveType){
			case triangleWave:
				defineTriangle(tableLen, maxHarms, ar, ai);
				break;
			case sawtoothWave:
				defineSawtooth(tableLen, maxHarms, ar, ai);	
				break;
			case squareWave:
				defineSquare(tableLen, maxHarms, ar, ai);	
				break;
			default:
				jassertfalse;
				break;
		}

		//from the ar partials, make a wave in ai, then store it in osc. keep scale so that we can reuse it for the next maxHarm, so that we have a normalized volume accross wavetables
		scale = makeWaveTable(tableLen, ar, ai, scale, topFreq);
        topFreq *= 2;
		//not sure, doesn't matter, not hit with default values
        if (tableLen > constantRatioLimit){ // variable table size (constant oversampling but with minimum table size)
            tableLen /= 2;
		}
    }
}

// if scale is 0, auto-scales
// returns scaling factor (0.0 if failure), and wavetable in ai array
float WaveTableOsc::makeWaveTable(int len, vector<double> &ar, vector<double> &ai, double scale, double topFreq) {
    fft(len, ar, ai);	//after this, ai contains the wave form, produced by an ifft I assume, and ar contains... noise? see waveTableOscFFtOutput.xlsx in dropbox/sBMP4
    //if no scale was supplied, find maximum sample amplitude, then derive scale
    if (scale == 0.0) {
        // calc normal
        double max = 0;
        for (int idx = 0; idx < len; idx++) {
            double temp = fabs(ai[idx]);
            if (max < temp)
                max = temp;
        }
        scale = 1.0 / max * .999;        
    }
    
    // normalize
    vector<float> wave(len);
    for (int idx = 0; idx < len; idx++)
        wave[idx] = ai[idx] * scale;
        
    if (addWaveTable(len, wave, topFreq))
        scale = 0.0;
    
    return scale;
}



// prepares sawtooth harmonics for ifft
void WaveTableOsc::defineSawtooth(int len, int numHarmonics, vector<double> &ar, vector<double> &ai){
	if(numHarmonics > (len / 2)){
		numHarmonics = (len / 2);
	}
	for(int idx = 0; idx < len; idx++){
		ai[idx] = 0;
		ar[idx] = 0;
	}
	//fill the ar vector, which I presume is the amplitude of real harmonics
	for(int idx = 1, jdx = len - 1; idx <= numHarmonics; idx++, jdx--){
		double temp = -1.0 / idx;	//for sawtooh, harmonic amplitude decreases as their index increases.
		ar[idx] = -temp;			//the firt half will be positive
		ar[jdx] = temp;				//the second half negative... why?
	}
}

// prepares sawtooth harmonics for ifft
void WaveTableOsc::defineSquare(int len, int numHarmonics, vector<double> &ar, vector<double> &ai) {
	if(numHarmonics > (len / 2)){
		numHarmonics = (len / 2);
	}
	for(int idx = 0; idx < len; idx++){
		ai[idx] = 0;
		ar[idx] = 0;
	}
	//fill the ar vector, which I presume is the amplitude of real harmonics
	for(int idx = 1, jdx = len - 1; idx <= numHarmonics; idx++, jdx--){
		double temp = idx & 0x01 ? 1.0 / idx : 0.0;
		ar[idx] = -temp;
		ar[jdx] = temp;
	}
}

// prepares sawtooth harmonics for ifft
void WaveTableOsc::defineTriangle(int len, int numHarmonics, vector<double> &ar, vector<double> &ai){
	if(numHarmonics > (len / 2)){
		numHarmonics = (len / 2);
	}
	for(int idx = 0; idx < len; idx++){
		ai[idx] = 0;
		ar[idx] = 0;
	}
	//fill the ar vector, which I presume is the amplitude of real harmonics

	float sign = 1;
	for(int idx = 1, jdx = len - 1; idx <= numHarmonics; idx++, jdx--){
		double temp = idx & 0x01 ? 1.0 / (idx * idx) * (sign = -sign) : 0.0;
		ar[idx] = -temp;
		ar[jdx] = temp;
	}
}

WaveTableOsc::~WaveTableOsc(void) {
    //for (int idx = 0; idx < numWaveTableSlots; idx++) {
    //    float *temp = m_oWaveTables[idx].waveTable;
    //    if (temp != NULL)
    //        delete [] temp;
    //}
}


//
// addWaveTable
//
// add wavetables in order of lowest frequency to highest
// topFreq is the highest frequency supported by a wavetable
// wavetables within an oscillator can be different lengths
//
// returns 0 upon success, or the number of wavetables if no more room is available
//
int WaveTableOsc::addWaveTable(int len, std::vector<float> waveTableIn, double topFreq) {
    if (numWaveTables < numWaveTableSlots) {
		m_oWaveTables[numWaveTables].waveTable = vector<float>(len);
        m_oWaveTables[numWaveTables].waveTableLen = len;
        m_oWaveTables[numWaveTables].topFreq = topFreq;
        
        // fill in wave
        for (long idx = 0; idx < len; idx++){
            m_oWaveTables[numWaveTables].waveTable[idx] = waveTableIn[idx];
		}

		++numWaveTables;
        return 0;
    }
    return numWaveTables;
}

//
// getOutput
//
// returns the current oscillator output
//
float WaveTableOsc::getOutput() {
    // grab the appropriate wavetable
    int waveTableIdx = 0;
	//TODO: why is phaseInc compared to the topFreq?
    while ((phaseInc >= m_oWaveTables[waveTableIdx].topFreq) && (waveTableIdx < (numWaveTables - 1))) {
        ++waveTableIdx;
    }
    waveTable *waveTable = &m_oWaveTables[waveTableIdx];

#if !doLinearInterp
    // truncate
    return waveTable->waveTable[int(phasor * waveTable->waveTableLen)];
#else
    // linear interpolation
    double temp = phasor * waveTable->waveTableLen;
    int intPart = temp;
    double fracPart = temp - intPart;
    float samp0 = waveTable->waveTable[intPart];
    if (++intPart >= waveTable->waveTableLen)
        intPart = 0;
    float samp1 = waveTable->waveTable[intPart];
    
    return samp0 + (samp1 - samp0) * fracPart;
#endif
}


//
// getOutputMinusOffset
//
// for variable pulse width: initialize to sawtooth,
// set phaseOfs to duty cycle, use this for osc output
//
// returns the current oscillator output
//
float WaveTableOsc::getOutputMinusOffset() {
    // grab the appropriate wavetable
    int waveTableIdx = 0;
    while ((this->phaseInc >= this->m_oWaveTables[waveTableIdx].topFreq) && (waveTableIdx < (this->numWaveTables - 1))) {
        ++waveTableIdx;
    }
    waveTable *waveTable = &this->m_oWaveTables[waveTableIdx];
    
#if !doLinearInterp
    // truncate
    double offsetPhasor = this->phasor + this->phaseOfs;
    if (offsetPhasor >= 1.0)
        offsetPhasor -= 1.0;
    return waveTable->waveTable[int(this->phasor * waveTable->waveTableLen)] - waveTable->waveTable[int(offsetPhasor * waveTable->waveTableLen)];
#else
    // linear
    double temp = this->phasor * waveTable->waveTableLen;
    int intPart = temp;
    double fracPart = temp - intPart;
    float samp0 = waveTable->waveTable[intPart];
    if (++intPart >= waveTable->waveTableLen)
        intPart = 0;
    float samp1 = waveTable->waveTable[intPart];
    float samp = samp0 + (samp1 - samp0) * fracPart;
    
    // and linear again for the offset part
    double offsetPhasor = this->phasor + this->phaseOfs;
    if (offsetPhasor > 1.0)
        offsetPhasor -= 1.0;
    temp = offsetPhasor * waveTable->waveTableLen;
    intPart = temp;
    fracPart = temp - intPart;
    samp0 = waveTable->waveTable[intPart];
    if (++intPart >= waveTable->waveTableLen)
        intPart = 0;
    samp1 = waveTable->waveTable[intPart];
    
    return samp - (samp0 + (samp1 - samp0) * fracPart);
#endif
}
