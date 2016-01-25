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
#include "BMP4SynthVoice.h"
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

AudioProcessor* JUCE_CALLTYPE createPluginFilter();

//==============================================================================
sBMP4AudioProcessor::sBMP4AudioProcessor()
: m_oLastDimensions()
, m_oDelayBuffer(2, 12000)
, m_fGain(defaultGain)
, m_fDelay(defaultDelay)
, m_iBufferSize(100)	//totally arbitrary value
, m_dLfoCurAngle(0.)
{
	//add our own audio input, because otherwise there is just a ghost input channel that is always on...
	busArrangement.inputBuses.clear();
	busArrangement.inputBuses.add(AudioProcessorBus("Mono input", AudioChannelSet::mono()));
	//and make sure we only have 1 audio output, otherwise really weird
	busArrangement.outputBuses.clear();
	busArrangement.outputBuses.add(AudioProcessorBus("Mono output", AudioChannelSet::mono()));

    for(int iCurVox = 0; iCurVox < s_iNumberOfVoices; ++iCurVox){
		Bmp4SynthVoice* voice = new Bmp4SynthVoice();
		JUCE_COMPILER_WARNING("this is terrible")
		voice->setProcessor(this);
        m_oSynth.addVoice(voice);
    }

    setWaveType(defaultWave);
	setFilterFr(defaultFilterFr);
    if(s_bUseSimplestLp){
        for(int iCurChannel = 0; iCurChannel < 2; ++iCurChannel){
            m_oLookBackVec[iCurChannel] = std::vector<float>(100, 0.f);
        }
    }

	//width of 265 is 20 (x buffer on left) + 3*75 (3 sliders) + 20 (buffer on right)
    m_oLastDimensions = std::make_pair(20+5*70+25, 150);
    m_iDelayPosition = 0;
}

sBMP4AudioProcessor::~sBMP4AudioProcessor() {
}

void sBMP4AudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) {
   const int numSamples = buffer.getNumSamples();
   if(s_bUseSimplestLp && m_iBufferSize != numSamples){
		m_iBufferSize = numSamples;
		setFilterFr(m_fFilterFr);	//just to update the lookback vector
	}

    //Pass any incoming midi messages to our keyboard, which will add messages to the buffer keys are pressed
    m_oKeyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    //Process these midi events and generate the output audio
    m_oSynth.renderNextBlock (buffer, midiMessages, 0, numSamples);

	int iDelayPosition = 0;
	//this loop is useless since we only have 1 output, but leave it for future expansion
	for (int iCurChannel = 0; iCurChannel < getMainBusNumOutputChannels(); ++iCurChannel){

		//-----GAIN/LFO
#if USE_LFO
		double dCurLfoValue = 1.;
		double dLfoFr = 5;
		dCurLfoValue = sin(m_dLfoCurAngle);
		buffer.applyGain(iCurChannel, 0, buffer.getNumSamples(), dCurLfoValue*m_fGain);
		m_dLfoCurAngle += dLfoFr * 2.0 * double_Pi;
#else
		buffer.applyGain(iCurChannel, 0, buffer.getNumSamples(), m_fGain);
#endif

        float* channelData = buffer.getWritePointer (iCurChannel);
        
        //-----DELAY
        float* delayData = m_oDelayBuffer.getWritePointer (jmin (iCurChannel, m_oDelayBuffer.getNumChannels() - 1));
        iDelayPosition = m_iDelayPosition;
        for (int i = 0; i < numSamples; ++i) {
            const float in = channelData[i];
            channelData[i] += delayData[iDelayPosition];
            delayData[iDelayPosition] = (delayData[iDelayPosition] + in) * m_fDelay;
            if(++iDelayPosition >= m_oDelayBuffer.getNumSamples()){
                iDelayPosition = 0;
            }
        }

        //-----FILTER
        if(s_bUseSimplestLp){
			simplestLP(channelData, numSamples, m_oLookBackVec[iCurChannel]);
		} else {
			float* channelData[1];
			channelData[0] = buffer.getWritePointer(iCurChannel);
			m_simpleFilter.process(numSamples, channelData);
		}
    }
    m_iDelayPosition = iDelayPosition;
}

void sBMP4AudioProcessor::setFilterFr(float p_fFilterFr){
    m_fFilterFr = p_fFilterFr;
    if(s_bUseSimplestLp){
        suspendProcessing(true);
        int i = static_cast<int>(m_fFilterFr*m_iBufferSize/10);
        m_oLookBackVec[0].resize(i);
        m_oLookBackVec[1].resize(i);
        suspendProcessing(false);
    } else if(m_oSynth.getSampleRate() > 0){
        updateSimpleFilter(m_oSynth.getSampleRate());
    }
}

void sBMP4AudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    // Use this method as the place to do any pre-playback initialisation that you need
    m_oSynth.setCurrentPlaybackSampleRate(sampleRate);
    if(!s_bUseSimplestLp){
        updateSimpleFilter(sampleRate);
    }
    m_oKeyboardState.reset();
    m_oDelayBuffer.clear();
}


void sBMP4AudioProcessor::updateSimpleFilter(double sampleRate) {
    float fMultiple = 1;   //the higher this is, the more linear and less curvy the exponential is
    JUCE_COMPILER_WARNING("s_iSimpleFilterLF should be the currently played note... but this is hard" + 
             "to get because as far as I know, we can only access that from the voice, which is buried in m_oSynth")
    float fExpCutoffFr = fMultiple * exp(log(s_iSimpleFilterHF * m_fFilterFr/fMultiple)) + s_iSimpleFilterLF;
    
    //this is called setup, but really it's just setting some values. 
    float q = 5.f;
#if	WIN32
    m_simpleFilter.setup(sampleRate, fExpCutoffFr, q);
#endif
}

JUCE_COMPILER_WARNING("need to put this in my audio library")
//from here: https://ccrma.stanford.edu/~jos/filters/Definition_Simplest_Low_Pass.html
void sBMP4AudioProcessor::simplestLP(float* p_pfSamples, int p_iTotalSamples, std::vector<float> &p_fLookBackVec){
	int iTotalLookBack = p_fLookBackVec.size();
	int iTotalAverage = iTotalLookBack+1;

	float* output = new float[p_iTotalSamples]{};

	int iCurSpl;
	for(iCurSpl = 0; iCurSpl < iTotalLookBack; ++iCurSpl){
		output[iCurSpl] = p_pfSamples[iCurSpl]/iTotalAverage;
		for(int iCurLookBack = (iTotalLookBack -1); iCurLookBack >= iCurSpl; --iCurLookBack){
			output[iCurSpl] += p_fLookBackVec[iCurLookBack]/iTotalAverage;
		}
		for(int iCurSubSpl = 0; iCurSubSpl < iCurSpl; ++iCurSubSpl){
			output[iCurSpl] += p_pfSamples[iCurSubSpl]/iTotalAverage;
		}
	}

	for(; iCurSpl < p_iTotalSamples; ++iCurSpl){
		output[iCurSpl] = p_pfSamples[iCurSpl]/iTotalAverage;
		for(int iCurLookBack = 0; iCurLookBack < iTotalLookBack; ++iCurLookBack){
			output[iCurSpl] += p_pfSamples[iCurSpl-iCurLookBack-1]/iTotalAverage;
		}
	}

	int iCurLookback = 0;
	iCurSpl = p_iTotalSamples-iTotalLookBack;
	while(iCurLookback < iTotalLookBack){
		p_fLookBackVec[iCurLookback++] = p_pfSamples[iCurSpl++];
	}

	std::copy(output, output + p_iTotalSamples, p_pfSamples);
	delete[] output;
}

//==============================================================================
int sBMP4AudioProcessor::getNumParameters(){
	return paramTotalNum;
}

float sBMP4AudioProcessor::getParameter(int index)
{
	// This method will be called by the host, probably on the audio thread, so
	// it's absolutely time-critical. Don't use critical sections or anything
	// UI-related, or anything at all that may block in any way!
	switch(index) {
	case paramGain:     return m_fGain;
	case paramDelay:    return m_fDelay;
	case paramWave:     return m_fWave;
	case paramFilterFr: return m_fFilterFr;
	default:            return 0.0f;
	}
}

void sBMP4AudioProcessor::setParameter(int index, float newValue)
{
	// This method will be called by the host, probably on the audio thread, so
	// it's absolutely time-critical. Don't use critical sections or anything
	// UI-related, or anything at all that may block in any way!
    switch(index) {
    case paramGain:		m_fGain = newValue;		break;
    case paramDelay:    m_fDelay = newValue;	break;
    case paramWave:     setWaveType(newValue);  break;
    case paramFilterFr: setFilterFr(newValue);  break;
	case paramLfoFr:	m_fLfoFr = newValue;  break;
    default:            break;
    }
}

void sBMP4AudioProcessor::setWaveType(float p_fWave){
	m_fWave = p_fWave;
	JUCE_COMPILER_WARNING("probably the sounds should be loaded by the voices...")
	m_oSynth.clearSounds();
	//m_oSynth.clearVoices();

    if(m_fWave == 0){
		m_oSynth.addSound(new SineWaveSound());
	} 
    else if(areSame(m_fWave, 1.f/3)){
		m_oSynth.addSound(new SquareWaveSound());
	} 
    else if(areSame(m_fWave, 2.f / 3)){
		m_oSynth.addSound(new TriangleWaveSound());
	} 
    else if(m_fWave == 1){
		m_oSynth.addSound(new SawtoothWaveSound());
	}

	//HAVING A MONOPHONIC SYNTH MAKES CLICKS BETWEEN NOTES BECAUSE NO TAILING OFF BETWEEN NOTES
	//to have a polyphonic synth, need to load several voices, like this
	//for (int i = 4; --i >= 0;){
	//	m_oSynth.addVoice(new SineWaveVoice());   // These voices will play our custom sine-wave sounds..
	//}
}

float sBMP4AudioProcessor::getParameterDefaultValue(int index){
    switch(index){
    case paramGain:     return defaultGain;
    case paramDelay:    return defaultDelay;
    case paramWave:     return defaultWave;
    case paramFilterFr: return defaultFilterFr;
    default:            break;
    }

    return 0.0f;
}

const String sBMP4AudioProcessor::getParameterName(int index){
	switch(index){
	case paramGain:     return "gain";
	case paramDelay:    return "delay";
	case paramWave:     return "wave";
    case paramFilterFr: return "filter";
	default:            break;
	}
	return String::empty;
}

const String sBMP4AudioProcessor::getParameterText(int index){
	return String(getParameter(index), 2);
}

//==============================================================================


void sBMP4AudioProcessor::releaseResources(){
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
	m_oKeyboardState.reset();
}

void sBMP4AudioProcessor::reset(){
	// Use this method as the place to clear any delay lines, buffers, etc, as it
	// means there's been a break in the audio's continuity.
	m_oDelayBuffer.clear();
}


//==============================================================================
void sBMP4AudioProcessor::getStateInformation (MemoryBlock& destData){
    XmlElement xml ("SBMP4SETTINGS");

    xml.setAttribute ("uiWidth", m_oLastDimensions.first);
    xml.setAttribute ("uiHeight", m_oLastDimensions.second);
    xml.setAttribute ("gain", m_fGain);
    xml.setAttribute ("delay", m_fDelay);
    xml.setAttribute ("wave", m_fWave);
    xml.setAttribute ("filter", m_fFilterFr);

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

            m_fGain     = (float)xmlState->getDoubleAttribute("gain",   m_fGain);
            m_fDelay    = (float)xmlState->getDoubleAttribute("delay",  m_fDelay);
            setWaveType((float)xmlState->getDoubleAttribute("wave",   m_fWave));
            setFilterFr((float)xmlState->getDoubleAttribute("filter", m_fFilterFr));
        }
    }
}

//==============================================================================
AudioProcessorEditor* sBMP4AudioProcessor::createEditor(){
    return new sBMP4AudioProcessorEditor (*this);
}

const String sBMP4AudioProcessor::getInputChannelName (const int channelIndex) const{
    return String (channelIndex + 1);
}

const String sBMP4AudioProcessor::getOutputChannelName (const int channelIndex) const{
    return String (channelIndex + 1);
}

bool sBMP4AudioProcessor::isInputChannelStereoPair (int /*index*/) const{
    return true;
}

bool sBMP4AudioProcessor::isOutputChannelStereoPair (int /*index*/) const{
    return true;
}

bool sBMP4AudioProcessor::acceptsMidi() const{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool sBMP4AudioProcessor::producesMidi() const{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool sBMP4AudioProcessor::silenceInProducesSilenceOut() const{
    return false;
}

double sBMP4AudioProcessor::getTailLengthSeconds() const{
    return 0.0;
}

// This creates new instances of the plugin.
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){
    return new sBMP4AudioProcessor();
}
