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
#include "sBMP4SoundsAndVoices.h"

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef __WIN32 
    #include <windows.h>
#endif

AudioProcessor* JUCE_CALLTYPE createPluginFilter();

//==============================================================================
sBMP4AudioProcessor::sBMP4AudioProcessor()
    :m_oLastDimensions()
    ,m_oDelayBuffer (2, 12000)
	,m_bUseSimplestLP(false)
	,m_fGain(defaultGain)
	,m_fDelay(defaultDelay)
{
    setWaveType(defaultWave);
	setFilterFr(defaultFilterFr);

	//width of 265 is 20 (x buffer on left) + 3*75 (3 sliders) + 20 (buffer on right)
    m_oLastDimensions = std::make_pair(20+4*65+20, 150);

    m_iDelayPosition = 0;
}

sBMP4AudioProcessor::~sBMP4AudioProcessor() {
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
    switch (index) {
        case paramGain:     return m_fGain;
        case paramDelay:    return m_fDelay;
        case paramWave:     return m_fWave;
		case paramFilterFr: return m_fFilterFr;
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
        case paramGain:		m_fGain = newValue;		break;
        case paramDelay:    m_fDelay = newValue;	break;
        case paramWave:     setWaveType(newValue);  break;
		case paramFilterFr: setFilterFr(newValue); break;
        default:            break;
    }
}

void sBMP4AudioProcessor::setWaveType(float p_fWave){
    m_fWave = p_fWave;
	JUCE_COMPILER_WARNING("probably the sounds should be loaded by the voices...")
	m_oSynth.clearSounds();
    m_oSynth.clearVoices();

    if (m_fWave == 0){
        m_oSynth.addSound (new SineWaveSound());
		for (int i = 4; --i >= 0;)
			m_oSynth.addVoice(new SineWaveVoice());
    }
    else if(areSame(m_fWave, 1.f/3)){
        m_oSynth.addSound (new SquareWaveSound());
		for (int i = 4; --i >= 0;)
			m_oSynth.addVoice(new SquareWaveVoice());
    }
	else if (areSame(m_fWave, 2.f / 3)){
		m_oSynth.addSound(new TriangleWaveSound());
		for (int i = 4; --i >= 0;)
			m_oSynth.addVoice(new TriangleWaveVoice());
	}
	else if (m_fWave == 1){
		m_oSynth.addSound(new SawtoothWaveSound());
		for (int i = 4; --i >= 0;)
			m_oSynth.addVoice(new SawtoothWaveVoice());
	}

	//HAVING A MONOPHONIC SYNTH MAKES CLICKS BETWEEN NOTES BECAUSE NO TAILING OFF BETWEEN NOTES
	//to have a polyphonic synth, need to load several voices, like this
	//for (int i = 4; --i >= 0;){
	//	m_oSynth.addVoice(new SineWaveVoice());   // These voices will play our custom sine-wave sounds..
	//}
}

void sBMP4AudioProcessor::setFilterFr(float p_fFilterFr){
	m_fFilterFr = p_fFilterFr;

	if(m_fFilterFr == 0){
		m_iFilterState = 0;
	} else if(areSame(m_fFilterFr, 1.f / 3)){
		m_iFilterState = 1;
	} else if(areSame(m_fFilterFr, 2.f / 3)){
		m_iFilterState = 2;
	} else if(m_fFilterFr == 1){
		m_iFilterState = 3;
	}
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

    // Now pass any incoming midi messages to our keyboard state object, and let it
    // add messages to the buffer if the user is clicking on the on-screen keys
    m_oKeyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    // and now get the synth to process these midi events and generate its output.
    m_oSynth.renderNextBlock (buffer, midiMessages, 0, numSamples);

	//-----GAIN
	for (channel = 0; channel < getNumInputChannels(); ++channel){
		buffer.applyGain(channel, 0, buffer.getNumSamples(), m_fGain);
	}

	//-----DELAY
    for (channel = 0; channel < getNumInputChannels(); ++channel) {
        float* channelData = buffer.getWritePointer (channel);
        float* delayData = m_oDelayBuffer.getWritePointer (jmin (channel, m_oDelayBuffer.getNumChannels() - 1));
        dp = m_iDelayPosition;

        for (int i = 0; i < numSamples; ++i) {
            const float in = channelData[i];
            channelData[i] += delayData[dp];
            delayData[dp] = (delayData[dp] + in) * m_fDelay;
			if (++dp >= m_oDelayBuffer.getNumSamples()){
				dp = 0;
			}
        }
    }
    m_iDelayPosition = dp;

	//-----SIMPLESTLP
	if(m_iFilterState != 0){
		for(channel = 0; channel < getNumInputChannels(); ++channel) {
			float* channelData = buffer.getWritePointer(channel);
			simplestLP(channelData, numSamples);
		}
	}

    // clear unused output channels
	for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i){
		buffer.clear(i, 0, buffer.getNumSamples());
	}
}

JUCE_COMPILER_WARNING("need to put this in my audio library")
//from here: https://ccrma.stanford.edu/~jos/filters/Definition_Simplest_Low_Pass.html
void sBMP4AudioProcessor::simplestLP(float *p_fAllSamples, int p_iTotalSamples){
	if(m_iFilterState == 1){
		for (int iCurSpl = 1; iCurSpl < p_iTotalSamples; ++iCurSpl) {
			p_fAllSamples[iCurSpl] = p_fAllSamples[iCurSpl]/2 + p_fAllSamples[iCurSpl-1]/2;
		}
	} else if(m_iFilterState == 2){
		p_fAllSamples[1] = p_fAllSamples[0] / 2 + p_fAllSamples[1] / 2;
		for(int iCurSpl = 2; iCurSpl < p_iTotalSamples-1; ++iCurSpl) {
			p_fAllSamples[iCurSpl] = p_fAllSamples[iCurSpl-1]/3 + p_fAllSamples[iCurSpl]/3 + p_fAllSamples[iCurSpl+1]/3;
		}
	} else if(m_iFilterState == 3){
		p_fAllSamples[1] = p_fAllSamples[0] / 2 + p_fAllSamples[1] / 2;
		p_fAllSamples[2] = p_fAllSamples[0] / 3 + p_fAllSamples[1] / 3 + p_fAllSamples[2] / 3;
		for(int iCurSpl = 3; iCurSpl < p_iTotalSamples - 1; ++iCurSpl) {
			p_fAllSamples[iCurSpl] = p_fAllSamples[iCurSpl - 2] / 4 + p_fAllSamples[iCurSpl - 1] / 4 + p_fAllSamples[iCurSpl] / 4 + p_fAllSamples[iCurSpl + 1] / 4;
		}
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

    if (xmlState != nullptr) {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("SBMP4SETTINGS")) {
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
