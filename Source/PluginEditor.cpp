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

//==============================================================================
sBMP4AudioProcessorEditor::sBMP4AudioProcessorEditor (sBMP4AudioProcessor& owner)
    : AudioProcessorEditor (owner),
      midiKeyboard (owner.keyboardState, MidiKeyboardComponent::horizontalKeyboard),
      infoLabel (String::empty),
      gainLabel ("", "Throughput level:"),
      delayLabel ("", "Delay:"),
      gainSlider ("gain"),
      delaySlider ("delay")
{
    // add some sliders..
    addAndMakeVisible (gainSlider);
    gainSlider.setSliderStyle (Slider::Rotary);
    gainSlider.addListener (this);
    gainSlider.setRange (0.0, 1.0, 0.01);

    addAndMakeVisible (delaySlider);
    delaySlider.setSliderStyle (Slider::Rotary);
    delaySlider.addListener (this);
    delaySlider.setRange (0.0, 1.0, 0.01);

    // add some labels for the sliders..
    gainLabel.attachToComponent (&gainSlider, false);
    gainLabel.setFont (Font (11.0f));

    delayLabel.attachToComponent (&delaySlider, false);
    delayLabel.setFont (Font (11.0f));

    // add the midi keyboard component..
    addAndMakeVisible (midiKeyboard);

    // add a label that will display the current timecode and status..
    addAndMakeVisible (infoLabel);
    infoLabel.setColour (Label::textColourId, Colours::blue);

    // add the triangular resizer component for the bottom-right of the UI
    addAndMakeVisible (resizer = new ResizableCornerComponent (this, &resizeLimits));
    resizeLimits.setSizeLimits (150, 150, 800, 300);

    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize (owner.m_iLastUIWidth,
             owner.m_iLastUIHeight);

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
    infoLabel.setBounds (10, 4, 400, 25);
    gainSlider.setBounds (20, 60, 150, 40);
    delaySlider.setBounds (200, 60, 150, 40);

    const int keyboardHeight = 70;
    midiKeyboard.setBounds (4, getHeight() - keyboardHeight - 4, getWidth() - 8, keyboardHeight);

    resizer->setBounds (getWidth() - 16, getHeight() - 16, 16, 16);

    getProcessor().m_iLastUIWidth = getWidth();
    getProcessor().m_iLastUIHeight = getHeight();
}

//==============================================================================
// This timer periodically checks whether any of the filter's parameters have changed...
void sBMP4AudioProcessorEditor::timerCallback() {
    sBMP4AudioProcessor& ourProcessor = getProcessor();

    AudioPlayHead::CurrentPositionInfo newPos (ourProcessor.lastPosInfo);

    if (lastDisplayedPosition != newPos)
        displayPositionInfo (newPos);

    gainSlider.setValue (ourProcessor.m_fGain, dontSendNotification);
    delaySlider.setValue (ourProcessor.m_fDelay, dontSendNotification);
}

// This is our Slider::Listener callback, when the user drags a slider.
void sBMP4AudioProcessorEditor::sliderValueChanged (Slider* slider) {
    if (slider == &gainSlider)    {
        // It's vital to use setParameterNotifyingHost to change any parameters that are automatable
        // by the host, rather than just modifying them directly, otherwise the host won't know
        // that they've changed.
        getProcessor().setParameterNotifyingHost (sBMP4AudioProcessor::gainParam, (float) gainSlider.getValue());
    } else if (slider == &delaySlider) {
        getProcessor().setParameterNotifyingHost (sBMP4AudioProcessor::delayParam, (float) delaySlider.getValue());
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
    lastDisplayedPosition = pos;
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

    infoLabel.setText ("[" + SystemStats::getJUCEVersion() + "]   " + displayText, dontSendNotification);
}
