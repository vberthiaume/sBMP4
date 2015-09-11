/*
==============================================================================
sBMP4: killer subtractive synth!

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

#include "BMP4SynthVoice.h"
#define _USE_MATH_DEFINES
#include <math.h>

Bmp4SynthVoice::Bmp4SynthVoice()
	: m_dOmega(0.0)
	, m_dTailOff(0.0)
    , m_iK(50)
{
}

void Bmp4SynthVoice::renderNextBlock(AudioSampleBuffer& p_oOutputBuffer, int p_iStartSample, int p_iTotalSamples)  {
	if (m_dOmega == 0.0) {
		return;
	}
	for (int iCurSample = 0; iCurSample < p_iTotalSamples; ++iCurSample) {
		//this will be == 1 if we don't have a tail off or = m_dTailOff if we do
		double dTailOffCopy = (m_dTailOff > 0) ? m_dTailOff : 1;

		//this is a virtual call to child functions. Not sure at this point why we need a copy of the tailOff, but not doing that doesn't work
		const float fCurrentSample = getSample(dTailOffCopy);

        for(int i = 0; i < p_oOutputBuffer.getNumChannels(); ++i){
			p_oOutputBuffer.addSample(i, p_iStartSample, fCurrentSample);
		}

		m_dCurrentAngle += m_dOmega;	//m_dOmega here is in radian
		++p_iStartSample;

		if (m_dTailOff > 0) {
			m_dTailOff *= 0.99;
			if (m_dTailOff <= 0.005) {
				clearCurrentNote();
				m_dOmega = 0.0;
				break;
			}
		}
	}
}

void Bmp4SynthVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int /*currentPitchWheelPosition*/)  {
	m_dCurrentAngle = 0.0;
	m_dLevel = velocity * 0.15;
	m_dTailOff = 0.0;

	double dFrequency = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
	double dNormalizedFreq = dFrequency / getSampleRate();
	m_dOmega = dNormalizedFreq * 2.0 * double_Pi;

	m_oCurrentSynthSound = dynamic_cast<SynthesiserSound*>(sound);
}

void Bmp4SynthVoice::stopNote(float /*velocity*/, bool allowTailOff)  {

	if (allowTailOff) {
		// start a tail-off by setting this flag. The render callback will pick up on
		// this and do a fade out, calling clearCurrentNote() when it's finished.

		if (m_dTailOff == 0.0)	// we only need to begin a tail-off if it's not already doing so - the
			// stopNote method could be called more than once.
			m_dTailOff = 1.0;
	} else {
		// we're being told to stop playing immediately, so reset everything..

		clearCurrentNote();
		m_dOmega = 0.0;
	}
}


bool Bmp4SynthVoice::canPlaySound(SynthesiserSound* sound)  {

	if (dynamic_cast <SineWaveSound*> (sound) ||
        dynamic_cast <SquareWaveSound*> (sound) ||
        dynamic_cast <TriangleWaveSound*> (sound) ||
        dynamic_cast <SawtoothWaveSound*> (sound)){
		return true;
	} else {
		return false;
	}
}

JUCE_COMPILER_WARNING("Would probably be way more efficient to use wave tables for all these additive synthesis getSamples in square, triangle and sawtooth")

float Bmp4SynthVoice::getSample(double dTail) {
    if(dynamic_cast <SineWaveSound*> (getCurrentlyPlayingSound().get())){
        return (float)(sin(m_dCurrentAngle) * m_dLevel * dTail);
    } 
    else if(dynamic_cast <SquareWaveSound*> (getCurrentlyPlayingSound().get())){
        float fCurrentSample = 0.0;
        for(int iCurK = 0; iCurK < m_iK; ++iCurK){
            fCurrentSample += static_cast<float> (sin(m_dCurrentAngle * (2 * iCurK + 1)) / (2 * iCurK + 1));
        }
        return fCurrentSample * m_dLevel * dTail;
    } 
    else if(dynamic_cast <TriangleWaveSound*> (getCurrentlyPlayingSound().get())){
        float fCurrentSample = 0.0;
        for(int iCurK = 0; iCurK < m_iK; ++iCurK){
            fCurrentSample += static_cast<float> (sin(M_PI*(2 * iCurK + 1) / 2) * (sin(m_dCurrentAngle * (2 * iCurK + 1)) / pow((2 * iCurK + 1), 2)));
        }
        return (8 / pow(M_PI, 2)) * fCurrentSample * m_dLevel * dTail;
    } 
    else if(dynamic_cast <SawtoothWaveSound*> (getCurrentlyPlayingSound().get())){
        //float fCurrentSample = 0.0;

        //for (int iCurK = 1; iCurK < m_iK; ++iCurK){
        //	fCurrentSample += static_cast<float> (sin((M_PI * iCurK) / 2) * (sin(m_dCurrentAngle * iCurK) / iCurK));
        //}

        //return (2 / M_PI) * fCurrentSample * m_dLevel * dTail;

        float fCurrentSample = 0.0;
        for(int iCurK = 1; iCurK < m_iK; ++iCurK){
            fCurrentSample += static_cast<float> (sin(m_dCurrentAngle * iCurK) / iCurK);
        }
        return 1/2 - (1 / M_PI) * fCurrentSample * m_dLevel * dTail;
    }
}




