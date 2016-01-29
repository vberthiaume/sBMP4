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

#ifndef __PLUGINPROCESSOR_H_526ED7A9__
#define __PLUGINPROCESSOR_H_526ED7A9__

#include "../JuceLibraryCode/JuceHeader.h"
#include "DspFilters/Dsp.h"

//==============================================================================
/**
    As the name suggest, this class does the actual audio processing.
*/
class sBMP4AudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    sBMP4AudioProcessor();
    ~sBMP4AudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void updateSimpleFilter(double sampleRate);

    void releaseResources() override;
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;
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

	float getLfoFr() { return m_fLfoFr*20; }

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

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // this is kept up to date with the midi messages that arrive, and the UI component
    // registers with it so it can represent the incoming messages
    MidiKeyboardState m_oKeyboardState;

private:

    void simplestLP(float* p_pfSamples, int p_iTotalSamples, std::vector<float> &p_fLookBackVec);

    float m_fGain, m_fDelay, m_fWave, m_fFilterFr, m_fLfoFr, m_fQ, m_fLfoAngle, m_fLfoOmega;

    void setWaveType(float p_fWave);

    void setFilterFr(float p_fFilterFr);

	void setFilterQ(float p_fQ);

	void setLfoFr(float p_fLfoFr);

    std::pair<int, int> m_oLastDimensions;

    //==============================================================================
    AudioSampleBuffer m_oDelayBuffer;
    int m_iDelayPosition;

    Synthesiser m_oSynth;
    double m_dLfoCurAngle;

    int m_iBufferSize;
    std::vector<float> m_oLookBackVec[2];

    Dsp::SimpleFilter <Dsp::RBJ::LowPass, 1>  m_simpleFilter;	//2 here is the number of channels, and is mandatory!

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(sBMP4AudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_526ED7A9__
