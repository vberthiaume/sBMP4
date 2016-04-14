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

#ifndef sBMP4_Sounds_h
#define sBMP4_Sounds_h

#include "../JuceLibraryCode/JuceHeader.h"
#include "constants.h"
#include "PluginProcessor.h"
#include "WaveTableOsc.h"

//==============================================================================
//Synth sounds

enum sounds{
     soundSine
    ,soundSquare
    ,soundTriangle
    ,soundSawtooth
    ,soundTotalCount
};

class SineWaveSound : public SynthesiserSound
{
public:
	SineWaveSound() {}

	bool appliesToNote(int /*midiNoteNumber*/) override  { return true; }
	bool appliesToChannel(int /*midiChannel*/) override  { return true; }
};

class SquareWaveSound : public SynthesiserSound
{
public:
	SquareWaveSound() {}

	bool appliesToNote(int /*midiNoteNumber*/) override  { return true; }
	bool appliesToChannel(int /*midiChannel*/) override  { return true; }
};

class TriangleWaveSound : public SynthesiserSound
{
public:
	TriangleWaveSound() {}

	bool appliesToNote(int /*midiNoteNumber*/) override  { return true; }
	bool appliesToChannel(int /*midiChannel*/) override  { return true; }
};

class SawtoothWaveSound : public SynthesiserSound
{
public:
	SawtoothWaveSound() {}

	bool appliesToNote(int /*midiNoteNumber*/) override  { return true; }
	bool appliesToChannel(int /*midiChannel*/) override  { return true; }
};


class Bmp4SynthVoice : public SynthesiserVoice
{
public:
	Bmp4SynthVoice();

	// this is where we determine which unique sound this voice can play
    bool canPlaySound(SynthesiserSound* sound);

    virtual float getSample(double dTail);

    float getSampleWaveTable(double dTail);
	
	void setProcessor(sBMP4AudioProcessor* p_processor);

    float getSampleAdditiveSynthesis(double dTail);

	void renderNextBlock(AudioSampleBuffer& p_oOutputBuffer, int p_iStartSample, int p_iTotalSamples) override;

	void startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int /*currentPitchWheelPosition*/) override;

	void stopNote(float /*velocity*/, bool allowTailOff) override;

	void pitchWheelMoved(int /*newValue*/) override {
		// can't be bothered implementing this for the demo!
	}

	void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override {
		// not interested in controllers in this case.
	}

protected:
	double m_dCurrentAngle, m_dOmega, m_dLevel, m_dTailOff;
    int m_iCurSound;
	WaveTableOsc m_oWaveTableOsc;
	sBMP4AudioProcessor* m_processor;
};

#endif //sBMP4_Sounds_h