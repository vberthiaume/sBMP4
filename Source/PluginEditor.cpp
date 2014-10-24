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
#include "constants.h"

//==============================================================================
sBMP4AudioProcessorEditor::sBMP4AudioProcessorEditor (sBMP4AudioProcessor& owner)
    : AudioProcessorEditor (owner),
        m_oMidiKeyboard (owner.m_oKeyboardState, MidiKeyboardComponent::horizontalKeyboard),
        m_oWaveLabel ("", "Wave:"),
        m_oInfoLabel (String::empty),
        m_oGainLabel ("", "Throughput level:"),
        m_oDelayLabel ("", "Delay:"),
        m_oWaveSlider("wave"),
        m_oGainSlider ("gain"),
        m_oDelaySlider ("delay")
{
    
    // add some sliders..
    // add some sliders..
    addAndMakeVisible (m_oWaveSlider);
    m_oWaveSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    m_oWaveSlider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    m_oWaveSlider.addListener (this);
    m_oWaveSlider.setRange (0.0, 1.0, 1.f/3);
    
    addAndMakeVisible (m_oGainSlider);
    m_oGainSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    m_oGainSlider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    m_oGainSlider.addListener (this);
    m_oGainSlider.setRange (0.0, 1.0, 0.01);

    addAndMakeVisible (m_oDelaySlider);
    m_oDelaySlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    m_oDelaySlider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    m_oDelaySlider.addListener (this);
    m_oDelaySlider.setRange (0.0, 1.0, 0.01);

    // add some labels for the sliders..
    m_oWaveLabel.attachToComponent (&m_oWaveSlider, false);
    m_oWaveLabel.setFont (Font (11.0f));
    
    m_oGainLabel.attachToComponent (&m_oGainSlider, false);
    m_oGainLabel.setFont (Font (11.0f));

    m_oDelayLabel.attachToComponent (&m_oDelaySlider, false);
    m_oDelayLabel.setFont (Font (11.0f));

    // add the midi keyboard component..
    addAndMakeVisible (m_oMidiKeyboard);

    // add a label that will display the current timecode and status..
//    addAndMakeVisible (m_oInfoLabel);
//    m_oInfoLabel.setColour (Label::textColourId, Colours::blue);

    // add the triangular m_pResizer component for the bottom-right of the UI
    addAndMakeVisible (m_pResizer = new ResizableCornerComponent (this, &m_oResizeLimits));
    m_oResizeLimits.setSizeLimits (150, 150, 800, 300);

    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize (owner.getDimensions().first, owner.getDimensions().second);

    startTimer (50);
}

sBMP4AudioProcessorEditor::~sBMP4AudioProcessorEditor()
{
}

//==============================================================================
void sBMP4AudioProcessorEditor::paint (Graphics& g)
{
    g.setGradientFill (ColourGradient (Colours::white, 0, 0,
                                       Colours::grey, 0, (float) getHeight(), false));
    g.fillAll();
}

void sBMP4AudioProcessorEditor::resized() {
    //m_oInfoLabel.setBounds (10, 4, 400, 25);
    int x = 20, y = 50, w = 100, h = 40;
    m_oWaveSlider.setBounds  (x,        y, w, h);
    m_oGainSlider.setBounds  (x + w,    y, w, h);
    m_oDelaySlider.setBounds (x + 2*w,  y, w, h);

    const int keyboardHeight = 70;
    m_oMidiKeyboard.setBounds (4, getHeight() - keyboardHeight - 4, getWidth() - 8, keyboardHeight);

    m_pResizer->setBounds (getWidth() - 16, getHeight() - 16, 16, 16);

    getProcessor().setDimensions(std::make_pair(getWidth(), getHeight()));
//    getProcessor().m_iLastUIWidth = getWidth();
//    getProcessor().m_iLastUIHeight = getHeight();
}

//==============================================================================
// This timer periodically checks whether any of the filter's parameters have changed...
void sBMP4AudioProcessorEditor::timerCallback() {
    sBMP4AudioProcessor& ourProcessor = getProcessor();

    AudioPlayHead::CurrentPositionInfo newPos (ourProcessor.getLastPosInfo());

    if (m_oLastDisplayedPosition != newPos)
        displayPositionInfo (newPos);

    m_oGainSlider.setValue (ourProcessor.getParameter(paramGain), dontSendNotification);
    m_oDelaySlider.setValue (ourProcessor.getParameter(paramDelay), dontSendNotification);
}

// This is our Slider::Listener callback, when the user drags a slider.
void sBMP4AudioProcessorEditor::sliderValueChanged (Slider* slider) {
    if (slider == &m_oGainSlider)    {
        getProcessor().setParameterNotifyingHost (paramGain, (float) m_oGainSlider.getValue());
    } else if (slider == &m_oDelaySlider) {
        getProcessor().setParameterNotifyingHost (paramDelay, (float) m_oDelaySlider.getValue());
    } else if (slider == &m_oWaveSlider) {
        getProcessor().setParameterNotifyingHost (paramWave, (float) m_oWaveSlider.getValue());
    }
}

//==============================================================================
// quick-and-dirty function to format a timecode string
static const String timeToTimecodeString (const double seconds)
{
    const double absSecs = fabs (seconds);

    const int hours =  (int) (absSecs / (60.0 * 60.0));
    const int mins  = ((int) (absSecs / 60.0)) % 60;
    const int secs  = ((int) absSecs) % 60;

    String s (seconds < 0 ? "-" : "");

    s << String (hours).paddedLeft ('0', 2) << ":"
      << String (mins) .paddedLeft ('0', 2) << ":"
      << String (secs) .paddedLeft ('0', 2) << ":"
      << String (roundToInt (absSecs * 1000) % 1000).paddedLeft ('0', 3);

    return s;
}

// quick-and-dirty function to format a bars/beats string
static const String ppqToBarsBeatsString (double ppq, double /*lastBarPPQ*/, int numerator, int denominator)
{
    if (numerator == 0 || denominator == 0)
        return "1|1|0";

    const int ppqPerBar = (numerator * 4 / denominator);
    const double beats  = (fmod (ppq, ppqPerBar) / ppqPerBar) * numerator;

    const int bar    = ((int) ppq) / ppqPerBar + 1;
    const int beat   = ((int) beats) + 1;
    const int ticks  = ((int) (fmod (beats, 1.0) * 960.0 + 0.5));

    String s;
    s << bar << '|' << beat << '|' << ticks;
    return s;
}

// Updates the text in our position label.
void sBMP4AudioProcessorEditor::displayPositionInfo (const AudioPlayHead::CurrentPositionInfo& pos)
{
    m_oLastDisplayedPosition = pos;
    String displayText;
    displayText.preallocateBytes (128);

    displayText << String (pos.bpm, 2) << " bpm, "
                << pos.timeSigNumerator << '/' << pos.timeSigDenominator
                << "  -  " << timeToTimecodeString (pos.timeInSeconds)
                << "  -  " << ppqToBarsBeatsString (pos.ppqPosition, pos.ppqPositionOfLastBarStart,
                                                    pos.timeSigNumerator, pos.timeSigDenominator);

    if (pos.isRecording)
        displayText << "  (recording)";
    else if (pos.isPlaying)
        displayText << "  (playing)";

    m_oInfoLabel.setText ("[" + SystemStats::getJUCEVersion() + "]   " + displayText, dontSendNotification);
}
