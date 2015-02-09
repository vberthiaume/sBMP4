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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "constants.h"
#define _USE_MATH_DEFINES
#include <math.h>

AudioProcessor* JUCE_CALLTYPE createPluginFilter();


//==============================================================================
//Synth sounds

class SineWaveSound : public SynthesiserSound
{
public:
    SineWaveSound() {}

    bool appliesToNote (int /*midiNoteNumber*/) override  { return true; }
    bool appliesToChannel (int /*midiChannel*/) override  { return true; }
};

class SquareWaveSound : public SynthesiserSound
{
public:
    SquareWaveSound() {}
    
    bool appliesToNote (int /*midiNoteNumber*/) override  { return true; }
    bool appliesToChannel (int /*midiChannel*/) override  { return true; }
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

//==============================================================================
//Synth voices

class Bmp4SynthVoice : public SynthesiserVoice
{
public:
	Bmp4SynthVoice()
        : m_dOmega (0.0),
          m_dTailOff (0.0)
    {
    }

	// this is where we determine which unique sound this voice can play
	bool canPlaySound(SynthesiserSound* sound) = 0;

	virtual float getSample(double p_dAngle, double p_dLevel, double dTail) = 0;

	void renderNextBlock(AudioSampleBuffer& p_oOutputBuffer, int p_iStartSample, int p_iTotalSamples) override {
		//VB not sure why this is?
		if (m_dOmega == 0.0) {
			return;
		}

		double dTailOffCopy = m_dTailOff > 0 ? m_dTailOff : 1;

		//while (--numSamples >= 0) {
		for (int iCurSample = 0; iCurSample < p_iTotalSamples; ++iCurSample) {

			float fCurrentSample = 0.0;

			fCurrentSample = getSample(m_dCurrentAngle, m_dLevel, dTailOffCopy);

			for (int i = p_oOutputBuffer.getNumChannels(); --i >= 0;){
				p_oOutputBuffer.addSample(i, p_iStartSample, fCurrentSample);
			}

			m_dCurrentAngle += m_dOmega;
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

    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int /*currentPitchWheelPosition*/) override {
        
        m_dCurrentAngle = 0.0;
        m_dLevel = velocity * 0.15;
        m_dTailOff = 0.0;
        
        double dFrequency = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double dNormalizedFreq = dFrequency / getSampleRate();
        m_dOmega = dNormalizedFreq * 2.0 * double_Pi;
        
		m_oCurrentSynthSound = dynamic_cast<SynthesiserSound*>(sound);
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override {
        if (allowTailOff) {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            if (m_dTailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
                                // stopNote method could be called more than once.
                m_dTailOff = 1.0;
        } else {
            // we're being told to stop playing immediately, so reset everything..

            clearCurrentNote();
            m_dOmega = 0.0;
        }
    }

    void pitchWheelMoved (int /*newValue*/) override {
        // can't be bothered implementing this for the demo!
    }

    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override {
        // not interested in controllers in this case.
    }

protected:
    double m_dCurrentAngle, m_dOmega, m_dLevel, m_dTailOff;
	SynthesiserSound* m_oCurrentSynthSound;
};

class SineWaveVoice : public Bmp4SynthVoice
{
	virtual bool canPlaySound(SynthesiserSound* sound) override {

		if (dynamic_cast <SineWaveSound*> (sound)){
			return true;
		} else {
			return false;
		}
	}

	virtual float getSample(double p_dAngle, double p_dLevel, double dTail) {
		return (float)(sin(p_dAngle) * p_dLevel * dTail);
	}

};

JUCE_COMPILER_WARNING(new std::string("Would probably be way more efficient to " + 
	"use wave tables for all these additive synthesis getSamples in square, triangle and sawtooth"))

class SquareWaveVoice : public Bmp4SynthVoice
{
public:
	SquareWaveVoice()
		:m_iK(50)
	{
	}

protected:
	bool canPlaySound(SynthesiserSound* sound) override {

		if (dynamic_cast <SquareWaveSound*> (sound)){
			return true;
		} else {
			return false;
		}
	}

	virtual float getSample(double p_dAngle, double p_dLevel, double dTail) {

		float fCurrentSample = 0.0;
		for (int iCurK = 0; iCurK < m_iK; ++iCurK){
			fCurrentSample += static_cast<float> (sin(m_dCurrentAngle * (2 * iCurK + 1)) / (2 * iCurK + 1));
		}
		return fCurrentSample * p_dLevel * dTail;
	}

	int m_iK;
};

class TriangleWaveVoice : public SquareWaveVoice
{
	virtual bool canPlaySound(SynthesiserSound* sound) override {

		if (dynamic_cast <TriangleWaveSound*> (sound)){
			return true;
		} else {
			return false;
		}
	}

	virtual float getSample(double p_dAngle, double p_dLevel, double dTail) {

		float fCurrentSample = 0.0;

		for (int iCurK = 0; iCurK < m_iK; ++iCurK){
			fCurrentSample += static_cast<float> (  sin(M_PI*(2*iCurK+1)/2) * (sin(m_dCurrentAngle * (2*iCurK+1)) / pow((2 * iCurK + 1),2))   );
		}

		JUCE_COMPILER_WARNING(new std::string("p_dLevel doesn<t seem to work for any wave"))
		return (8 / pow(M_PI,2)) * fCurrentSample * p_dLevel * dTail;
	}

};

class SawtoothWaveVoice : public SquareWaveVoice
{
	virtual bool canPlaySound(SynthesiserSound* sound) override {

		if (dynamic_cast <SawtoothWaveSound*> (sound)){
			return true;
		} else {
			return false;
		}
	}

	virtual float getSample(double p_dAngle, double p_dLevel, double dTail) {

		float fCurrentSample = 0.0;

		for (int iCurK = 0; iCurK < m_iK; ++iCurK){
			fCurrentSample += static_cast<float> (sin( (M_PI * iCurK) / 2) * (sin(m_dCurrentAngle * iCurK) / iCurK)) ;
		}

		return (2 / M_PI) * fCurrentSample * p_dLevel * dTail;
	}

};

//==============================================================================
sBMP4AudioProcessor::sBMP4AudioProcessor()
    : m_oLastDimensions(),
    m_oDelayBuffer (2, 12000)
{
    // Set up some default values..
    m_fGain = defaultGain;
    m_fDelay = defaultDelay;
    setWaveType(defaultWave);

    m_oLastDimensions = std::make_pair(400,200);

    lastPosInfo.resetToDefault();
    m_iDelayPosition = 0;
   

}

sBMP4AudioProcessor::~sBMP4AudioProcessor()
{
}

//==============================================================================
int sBMP4AudioProcessor::getNumParameters()
{
    return paramTotalNum;
}

float sBMP4AudioProcessor::getParameter (int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case paramGain:     return m_fGain;
        case paramDelay:    return m_fDelay;
        case paramWave:     return m_fWave;
        default:            return 0.0f;
    }
}

void sBMP4AudioProcessor::setParameter (int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case paramGain:     m_fGain = newValue;  break;
        case paramDelay:    m_fDelay = newValue;  break;
        case paramWave:     setWaveType(newValue);  break;
        default:            break;
    }
}

void sBMP4AudioProcessor::setWaveType(float p_fWave){
    m_fWave = p_fWave;
	JUCE_COMPILER_WARNING(new string("probably the sounds should be loaded by the voices..."))
	m_oSynth.clearSounds();
    m_oSynth.clearVoices();

    if (m_fWave == 0){
        m_oSynth.addSound (new SineWaveSound());
		m_oSynth.addVoice(new SineWaveVoice());
    }
    else if(areSame(m_fWave, 1.f/3)){
        m_oSynth.addSound (new SquareWaveSound());
		m_oSynth.addVoice(new SquareWaveVoice());
    }
	else if (areSame(m_fWave, 2.f / 3)){
		m_oSynth.addSound(new TriangleWaveSound());
		m_oSynth.addVoice(new TriangleWaveVoice());
	}
	else if (m_fWave == 1){
		m_oSynth.addSound(new SawtoothWaveSound());
		m_oSynth.addVoice(new SawtoothWaveVoice());
	}

	//to have a polyphonic synth, need to load several voices, like this
	//for (int i = 4; --i >= 0;){
	//	m_oSynth.addVoice(new SineWaveVoice());   // These voices will play our custom sine-wave sounds..
	//}
}

float sBMP4AudioProcessor::getParameterDefaultValue (int index)
{
    switch (index)
    {
        case paramGain:     return defaultGain;
        case paramDelay:    return defaultDelay;
        case paramWave:     return defaultWave;
        default:            break;
    }

    return 0.0f;
}

const String sBMP4AudioProcessor::getParameterName (int index)
{
    switch (index)
    {
        case paramGain:     return "gain";
        case paramDelay:    return "delay";
        case paramWave:     return "wave";
        default:            break;
    }

    return String::empty;
}

const String sBMP4AudioProcessor::getParameterText (int index)
{
    return String (getParameter (index), 2);
}

//==============================================================================
void sBMP4AudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    m_oSynth.setCurrentPlaybackSampleRate (sampleRate);
    m_oKeyboardState.reset();
    m_oDelayBuffer.clear();
}

void sBMP4AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    m_oKeyboardState.reset();
}

void sBMP4AudioProcessor::reset()
{
    // Use this method as the place to clear any delay lines, buffers, etc, as it
    // means there's been a break in the audio's continuity.
    m_oDelayBuffer.clear();
}

void sBMP4AudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();
    int channel, dp = 0;

    // Go through the incoming data, and apply our gain to it...
    for (channel = 0; channel < getNumInputChannels(); ++channel)
        buffer.applyGain (channel, 0, buffer.getNumSamples(), m_fGain);

    // Now pass any incoming midi messages to our keyboard state object, and let it
    // add messages to the buffer if the user is clicking on the on-screen keys
    m_oKeyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    // and now get the synth to process these midi events and generate its output.
    m_oSynth.renderNextBlock (buffer, midiMessages, 0, numSamples);

    // Apply our delay effect to the new output..
    for (channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);
        float* delayData = m_oDelayBuffer.getWritePointer (jmin (channel, m_oDelayBuffer.getNumChannels() - 1));
        dp = m_iDelayPosition;

        for (int i = 0; i < numSamples; ++i)
        {
            const float in = channelData[i];
            channelData[i] += delayData[dp];
            delayData[dp] = (delayData[dp] + in) * m_fDelay;
            if (++dp >= m_oDelayBuffer.getNumSamples())
                dp = 0;
        }
    }

    m_iDelayPosition = dp;

    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // ask the host for the current time so we can display it...
    AudioPlayHead::CurrentPositionInfo newTime;

    if (getPlayHead() != nullptr && getPlayHead()->getCurrentPosition (newTime))
    {
        // Successfully got the current time from the host..
        lastPosInfo = newTime;
    }
    else
    {
        // If the host fails to fill-in the current time, we'll just clear it to a default..
        lastPosInfo.resetToDefault();
    }
}



//==============================================================================
void sBMP4AudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // Here's an example of how you can use XML to make it easy and more robust:

    // Create an outer XML element..
    XmlElement xml ("SBMP4SETTINGS");

    // add some attributes to it..
    xml.setAttribute ("uiWidth", m_oLastDimensions.first);
    xml.setAttribute ("uiHeight", m_oLastDimensions.second);
    xml.setAttribute ("gain", m_fGain);
    xml.setAttribute ("delay", m_fDelay);
    xml.setAttribute ("wave", m_fWave);

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void sBMP4AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters..
            m_oLastDimensions.first  = xmlState->getIntAttribute ("uiWidth", m_oLastDimensions.first);
            m_oLastDimensions.second = xmlState->getIntAttribute ("uiHeight", m_oLastDimensions.second);

            m_fGain  = (float) xmlState->getDoubleAttribute ("gain", m_fGain);
            m_fDelay = (float) xmlState->getDoubleAttribute ("delay", m_fDelay);
            m_fWave  = (float) xmlState->getDoubleAttribute ("wave", m_fWave);

        }
    }
}


//==============================================================================
AudioProcessorEditor* sBMP4AudioProcessor::createEditor()
{
    return new sBMP4AudioProcessorEditor (*this);
}

const String sBMP4AudioProcessor::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String sBMP4AudioProcessor::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool sBMP4AudioProcessor::isInputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool sBMP4AudioProcessor::isOutputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool sBMP4AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool sBMP4AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool sBMP4AudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double sBMP4AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new sBMP4AudioProcessor();
}
