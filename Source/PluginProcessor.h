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

#ifndef __PLUGINPROCESSOR_H_526ED7A9__
#define __PLUGINPROCESSOR_H_526ED7A9__

#include "../JuceLibraryCode/JuceHeader.h"
#include "DspFilters/Dsp.h"
#include "constants.h"

//==============================================================================
/**
    As the name suggest, this class does the actual audio processing.
*/
class sBMP4AudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    sBMP4AudioProcessor();
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void updateSimpleFilter();

    void releaseResources() override;
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;
    void addSubOscMidiNotes(MidiBuffer& midiMessages);
    void reset() override;

    //==============================================================================
    bool hasEditor() const override                  { return true; }
    AudioProcessorEditor* createEditor() override;

    //==============================================================================
    const String getName() const override            { return JucePlugin_Name; }

    int getNumParameters() override;
    float getParameter (int index) override;
    float getParameterDefaultValue (int index) override;
    void setParameter (int index, float newValue) override;
    const String getParameterName (int index) override;
    const String getParameterText (int index) override;

    const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    bool isInputChannelStereoPair (int index) const override;
    bool isOutputChannelStereoPair (int index) const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override                                               { return 1; }
    int getCurrentProgram() override                                            { return 0; }
    void setCurrentProgram (int /*index*/) override                             {}
    const String getProgramName (int /*index*/) override                        { return "Default"; }
    void changeProgramName (int /*index*/, const String& /*newName*/) override  {}
    std::pair<int, int> getDimensions()                                         {return m_oLastDimensions;}
    void setDimensions(std::pair<int, int> p_oNewDimensions)                    {m_oLastDimensions = p_oNewDimensions;}
	void setLfoOn(bool p_bLfoIsOn){	m_bLfoIsOn = p_bLfoIsOn;}
	void setLfoOn(float p_fLfoIsOn){ (p_fLfoIsOn == 1.) ? m_bLfoIsOn = true : m_bLfoIsOn = false;}
	bool getLfoOn() { return m_bLfoIsOn;}
	void setSubOscOn(bool p_bSubOscIsOn){	m_bSubOscIsOn = p_bSubOscIsOn;}
	void setSubOscOn(float p_fSubOscIsOn){ (p_fSubOscIsOn == 1.) ? m_bSubOscIsOn = true : m_bSubOscIsOn = false;}
	bool getSubOscOn() { return m_bSubOscIsOn;}
    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // this is kept up to date with the midi messages that arrive, and the UI component
    // registers with it so it can represent the incoming messages
    MidiKeyboardState m_oKeyboardState;

private:

    void simplestLP(float* p_pfSamples, int p_iTotalSamples, std::vector<float> &p_fLookBackVec);

    float m_fGain, m_fDelay, m_fWave, m_fFilterFr, m_fLfoFrHr, m_fQHr, m_fLfoAngle, m_fLfoOmega;

	bool m_bLfoIsOn;
	bool m_bSubOscIsOn;

    void setWaveType(float p_fWave);

    void setFilterFr(float p_fFilterFr);

	void setFilterQ01(float p_fQ);
	float getFilterQ01();

	void setLfoFr01(float p_fLfoFr);
	float getLfoFr01();

    float m_fSampleRate;

    std::pair<int, int> m_oLastDimensions;

    //==============================================================================
    AudioSampleBuffer m_oDelayBuffer;
    int m_iDelayPosition;

    Synthesiser m_oSynth;

#if USE_SIMPLEST_LP
    int m_iBufferSize;
    std::vector<float> m_oLookBackVec[2];
#else
    Dsp::SimpleFilter <Dsp::RBJ::LowPass, 1>  m_simpleFilter;	//2 here is the number of channels, and is mandatory!
#endif

    static BusesProperties getBusesProperties();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(sBMP4AudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_526ED7A9__
