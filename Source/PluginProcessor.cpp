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
#include "BMP4SynthVoice.h"
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

AudioProcessor* JUCE_CALLTYPE createPluginFilter();

//==============================================================================
sBMP4AudioProcessor::sBMP4AudioProcessor()
: m_oLastDimensions()
, m_oDelayBuffer(2, 12000)
, m_fGain(k_fDefaultGain)
, m_fDelay(k_fDefaultDelay)
, m_fQHr(k_fDefaultQHr)
, m_iBufferSize(100)	//totally arbitrary value
, m_fLfoAngle(0.)
, m_fLfoOmega(0.)
, m_bLfoIsOn(true)
{
	//add our own audio input, because otherwise there is just a ghost input channel that is always on...
	busArrangement.inputBuses.clear();
	busArrangement.inputBuses.add(AudioProcessorBus("Mono input", AudioChannelSet::mono()));
	//and make sure we only have 1 audio output, otherwise really weird
	busArrangement.outputBuses.clear();
	busArrangement.outputBuses.add(AudioProcessorBus("Mono output", AudioChannelSet::mono()));

    for(int iCurVox = 0; iCurVox < k_iNumberOfVoices; ++iCurVox){
		Bmp4SynthVoice* voice = new Bmp4SynthVoice();
		JUCE_COMPILER_WARNING("this is terrible")
		voice->setProcessor(this);
        m_oSynth.addVoice(voice);
    }

    setWaveType(k_fDefaultWave);
	setFilterFr(k_fDefaultFilterFr);
	setLfoFr01(k_fDefaultLfoFr01);
    if(k_bUseSimplestLp){
        for(int iCurChannel = 0; iCurChannel < 2; ++iCurChannel){
            m_oLookBackVec[iCurChannel] = std::vector<float>(100, 0.f);
        }
    }

	//width of 265 is 20 (x buffer on left) + 3*75 (3 sliders) + 20 (buffer on right)
    /*m_oLastDimensions = std::make_pair(20+5*70+25, k_iKeyboardHeight + 80 + 60);*/
	m_oLastDimensions = std::make_pair(2*k_iXMargin + k_iNumberOfHorizontalSliders*k_iSliderWidth, 
									   k_iYMargin   + k_iNumberOfVerticaltalSliders * (k_iSliderHeight + k_iLabelHeight) + k_iKeyboardHeight);
    m_iDelayPosition = 0;
}

sBMP4AudioProcessor::~sBMP4AudioProcessor() {
}

void sBMP4AudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    m_oSynth.setCurrentPlaybackSampleRate(sampleRate);
    if(!k_bUseSimplestLp){
        updateSimpleFilter(sampleRate);
    }
	setLfoFr01(getLfoFr01());
	setFilterFr(m_fFilterFr);
    m_oKeyboardState.reset();
    m_oDelayBuffer.clear();
}

void sBMP4AudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) {
   const int numSamples = buffer.getNumSamples();
   if(k_bUseSimplestLp && m_iBufferSize != numSamples){
		m_iBufferSize = numSamples;
		setFilterFr(m_fFilterFr);	//just to update the lookback vector
	}
    //Pass any incoming midi messages to our keyboard, which will add messages to the buffer keys are pressed
    m_oKeyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);
	bool m_bUseSubOsc = true;
	if (m_bUseSubOsc) {
		MidiBuffer::Iterator it(midiMessages);
		MidiMessage msg;
		MidiBuffer allSubOscMessages;
		int iPosition;
		while(it.getNextEvent(msg, iPosition)){
			if (msg.isNoteOn() || msg.isNoteOff()){
				MidiMessage curSubOscMsg(msg);
				curSubOscMsg.setNoteNumber(curSubOscMsg.getNoteNumber()-8);
				allSubOscMessages.addEvent(curSubOscMsg, iPosition);
			}
		}
		midiMessages.addEvents(allSubOscMessages,0,-1,0);
	}
    //Process these midi events and generate the output audio
    m_oSynth.renderNextBlock (buffer, midiMessages, 0, numSamples);
	//this loop is useless since we only have 1 output, but leave it for future expansion
	int iDelayPosition = 0;
	for (int iCurChannel = 0; iCurChannel < getMainBusNumOutputChannels(); ++iCurChannel){

		//-----GAIN
		buffer.applyGain(iCurChannel, 0, buffer.getNumSamples(), m_fGain);

		float* channelData = buffer.getWritePointer (iCurChannel);
        
        //-----FILTER
        if(k_bUseSimplestLp){
			simplestLP(channelData, numSamples, m_oLookBackVec[iCurChannel]);
		} else {
			float* channelData[1];
			channelData[0] = buffer.getWritePointer(iCurChannel);
			m_simpleFilter.process(numSamples, channelData);
		}

		//-----DELAY AND LFO
		float* delayData = m_oDelayBuffer.getWritePointer(jmin(iCurChannel, m_oDelayBuffer.getNumChannels() - 1));
		iDelayPosition = m_iDelayPosition;
		for (int i = 0; i < numSamples; ++i) {
			const float in = channelData[i];
			//----LFO
			if(m_bLfoIsOn){
				channelData[i] *= (sin(m_fLfoAngle) + 1) / 2;
				JUCE_COMPILER_WARNING("on the first note on after all notes are off, this angle should be reset to 0")
					m_fLfoAngle += m_fLfoOmega;
				if(m_fLfoAngle > 2 * M_PI){
					m_fLfoAngle -= 2 * M_PI;
				}
			}

			//----DELAY
			channelData[i] += delayData[iDelayPosition];
			delayData[iDelayPosition] = (delayData[iDelayPosition] + in) * m_fDelay;
			if (++iDelayPosition >= m_oDelayBuffer.getNumSamples()) {
				iDelayPosition = 0;
			}
		}
    }
    m_iDelayPosition = iDelayPosition;
}

//the argument to this will be [0, 1], which we need to convert to [kmin, kmax]
void sBMP4AudioProcessor::setLfoFr01(float p_fLfoFr01){
	m_fLfoFrHr = convert01ToHr(p_fLfoFr01, k_fMinLfoFr, k_fMaxLfoFr);
	m_fLfoOmega = 2 * M_PI*m_fLfoFrHr / getSampleRate();		//dividing the frequency by the sample rate essentially gives us the frequency in samples
}


//m_fLfoFrHr is stored internally as [kmin, kmax], and we need here to return something [0, 1]
float sBMP4AudioProcessor::getLfoFr01() {
	return convertHrTo01(m_fLfoFrHr, k_fMinLfoFr, k_fMaxLfoFr);
}

void sBMP4AudioProcessor::setFilterFr(float p_fFilterFr){
    m_fFilterFr = p_fFilterFr;
    if(k_bUseSimplestLp){
        suspendProcessing(true);
        int i = static_cast<int>(m_fFilterFr*m_iBufferSize/10);
        m_oLookBackVec[0].resize(i);
        m_oLookBackVec[1].resize(i);
        suspendProcessing(false);
    } else if(m_oSynth.getSampleRate() > 0){
        updateSimpleFilter(m_oSynth.getSampleRate());
    }
}

float sBMP4AudioProcessor::getFilterQ01(){
	return convertHrTo01(m_fQHr, k_fMinQHr, k_fMaxQHr);
}

void sBMP4AudioProcessor::setFilterQ01(float p_fQ01){
	m_fQHr = convert01ToHr(p_fQ01, k_fMinQHr, k_fMaxQHr);
	updateSimpleFilter(m_oSynth.getSampleRate());
}

void sBMP4AudioProcessor::updateSimpleFilter(double sampleRate) {
	if (sampleRate == 0){
		return;
	}
    float fMultiple = 1;   //the higher this is, the more linear and less curvy the exponential is
    JUCE_COMPILER_WARNING("k_iSimpleFilterLF should be the currently played note... but this is hard" + 
             "to get because as far as I know, we can only access that from the voice, which is buried in m_oSynth")
    float fExpCutoffFr = fMultiple * exp(log(k_iSimpleFilterHF * m_fFilterFr/fMultiple)) + k_iSimpleFilterLF;
    
#if	WIN32
	//this is called setup, but really it's just setting some values. 
	m_simpleFilter.setup(sampleRate, fExpCutoffFr, m_fQHr);
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
	case paramQ:		return getFilterQ01();
	case paramLfoFr:	return getLfoFr01();
	case paramLfoOn:	return getLfoOn();
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
    case paramFilterFr: setFilterFr(newValue);	break;
	case paramQ:		setFilterQ01(newValue);	break;
	case paramLfoFr:	setLfoFr01(newValue);	break;
	case paramLfoOn:	setLfoOn(newValue);		break;
	
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
		case paramGain:     return k_fDefaultGain;
		case paramDelay:    return k_fDefaultDelay;
		case paramWave:     return k_fDefaultWave;
		case paramFilterFr: return k_fDefaultFilterFr;
		case paramQ:		return k_fDefaultQ01;
		case paramLfoFr:	return k_fDefaultLfoFr01;
		case paramLfoOn:	return k_fDefaultLfoOn;
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
		case paramQ:		return "resonance";
		case paramLfoFr:	return "lfo_Fr";
		case paramLfoOn:	return "lfo_On";
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
    xml.setAttribute ("uiWidth",	m_oLastDimensions.first);
    xml.setAttribute ("uiHeight",	m_oLastDimensions.second);
    xml.setAttribute ("gain",		m_fGain);
    xml.setAttribute ("delay",		m_fDelay);
    xml.setAttribute ("wave",		m_fWave);
    xml.setAttribute ("filter",		m_fFilterFr);
	xml.setAttribute ("m_fLfoFrHr", getLfoFr01());
	xml.setAttribute ("m_fQHr",		getFilterQ01());
	xml.setAttribute ("m_bLfoIsOn", m_bLfoIsOn);

    copyXmlToBinary (xml, destData);
}

void sBMP4AudioProcessor::setStateInformation (const void* data, int sizeInBytes){
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr) {
        if (xmlState->hasTagName ("SBMP4SETTINGS")) {
            m_oLastDimensions.first  =	xmlState->getIntAttribute(		"uiWidth",		m_oLastDimensions.first);
            m_oLastDimensions.second =	xmlState->getIntAttribute(		"uiHeight",		m_oLastDimensions.second);
            m_fGain = (float)			xmlState->getDoubleAttribute(	"gain",			m_fGain);
            m_fDelay = (float)			xmlState->getDoubleAttribute(	"delay",		m_fDelay);
            setWaveType((float)			xmlState->getDoubleAttribute(	"wave",			m_fWave));
            setFilterFr((float)			xmlState->getDoubleAttribute(	"filter",		m_fFilterFr));
			setLfoFr01((float)			xmlState->getDoubleAttribute(	"m_fLfoFrHr",	getLfoFr01()));
			setFilterQ01((float)		xmlState->getDoubleAttribute(	"m_fQHr",		getFilterQ01()));
			setLfoOn(					xmlState->getBoolAttribute(		"m_bLfoIsOn",	m_bLfoIsOn));
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
