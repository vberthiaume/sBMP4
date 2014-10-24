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

#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter();


//==============================================================================
/** A demo synth sound that's just a basic sine wave.. */
class SineWaveSound : public SynthesiserSound
{
public:
    SineWaveSound() {}

    bool appliesToNote (const int /*midiNoteNumber*/) override  { return true; }
    bool appliesToChannel (const int /*midiChannel*/) override  { return true; }
};

//==============================================================================
/** A simple demo synth voice that just plays a sine wave.. */
class SineWaveVoice  : public SynthesiserVoice
{
public:
    SineWaveVoice()
        : angleDelta (0.0),
          tailOff (0.0)
    {
    }

    bool canPlaySound (SynthesiserSound* sound) override
    {
        return dynamic_cast <SineWaveSound*> (sound) != 0;
    }

    void startNote (int midiNoteNumber, float velocity,
                    SynthesiserSound* /*sound*/, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        double cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * double_Pi;
    }

    void stopNote (bool allowTailOff) override
    {
        if (allowTailOff)
        {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
                                // stopNote method could be called more than once.
                tailOff = 1.0;
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..

            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved (int /*newValue*/) override
    {
        // can't be bothered implementing this for the demo!
    }

    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override
    {
        // not interested in controllers in this case.
    }

    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta != 0.0)
        {
            if (tailOff > 0)
            {
                while (--numSamples >= 0)
                {
                    const float currentSample = (float) (sin (currentAngle) * level * tailOff);

                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote();

                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    const float currentSample = (float) (sin (currentAngle) * level);

                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle, angleDelta, level, tailOff;
};

const float defaultGain = 1.0f;
const float defaultDelay = 0.5f;

//==============================================================================
sBMP4AudioProcessor::sBMP4AudioProcessor()
    : m_oDelayBuffer (2, 12000)
{
    // Set up some default values..
    m_fGain = defaultGain;
    m_fDelay = defaultDelay;

    m_iLastUIWidth = 400;
    m_iLastUIHeight = 200;

    lastPosInfo.resetToDefault();
    m_iDelayPosition = 0;

    // Initialise the synth...
    for (int i = 4; --i >= 0;){
        m_oSynth.addVoice (new SineWaveVoice());   // These voices will play our custom sine-wave sounds..
    }
    
    m_oSynth.addSound (new SineWaveSound());
}

sBMP4AudioProcessor::~sBMP4AudioProcessor()
{
}

//==============================================================================
int sBMP4AudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float sBMP4AudioProcessor::getParameter (int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    switch (index)
    {
        case gainParam:     return m_fGain;
        case delayParam:    return m_fDelay;
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
        case gainParam:     m_fGain = newValue;  break;
        case delayParam:    m_fDelay = newValue;  break;
        default:            break;
    }
}

float sBMP4AudioProcessor::getParameterDefaultValue (int index)
{
    switch (index)
    {
        case gainParam:     return defaultGain;
        case delayParam:    return defaultDelay;
        default:            break;
    }

    return 0.0f;
}

const String sBMP4AudioProcessor::getParameterName (int index)
{
    switch (index)
    {
        case gainParam:     return "gain";
        case delayParam:    return "delay";
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
    keyboardState.reset();
    m_oDelayBuffer.clear();
}

void sBMP4AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    keyboardState.reset();
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
    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

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
    XmlElement xml ("MYPLUGINSETTINGS");

    // add some attributes to it..
    xml.setAttribute ("uiWidth", m_iLastUIWidth);
    xml.setAttribute ("uiHeight", m_iLastUIHeight);
    xml.setAttribute ("gain", m_fGain);
    xml.setAttribute ("delay", m_fDelay);

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
            m_iLastUIWidth  = xmlState->getIntAttribute ("uiWidth", m_iLastUIWidth);
            m_iLastUIHeight = xmlState->getIntAttribute ("uiHeight", m_iLastUIHeight);

            m_fGain  = (float) xmlState->getDoubleAttribute ("gain", m_fGain);
            m_fDelay = (float) xmlState->getDoubleAttribute ("delay", m_fDelay);
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
