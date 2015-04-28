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
sBMP4AudioProcessorEditor::sBMP4AudioProcessorEditor (sBMP4AudioProcessor& processor)
    : AudioProcessorEditor (processor),
        m_oMidiKeyboard (processor.m_oKeyboardState, MidiKeyboardComponent::horizontalKeyboard),
		m_oWaveLabel("", "wave"),//"Wave:"),
        m_oInfoLabel (String::empty),
        m_oGainLabel ("", "gain"),
        m_oDelayLabel ("", "delay"),
        m_oWaveSlider("wave"),
        m_oGainSlider ("gain"),
        m_oDelaySlider ("delay"),
		m_oSineImage("sine"), 
		m_oSawImage("saw"),
		m_oTriangleImage("triangle"), 
		m_oLogoImage("sBMP4")
{
    
    // add some sliders..
    addAndMakeVisible (m_oWaveSlider);
    m_oWaveSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    m_oWaveSlider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
	m_oWaveSlider.setColour(Slider::ColourIds::rotarySliderFillColourId, Colours::white);
	m_oWaveSlider.setColour(Slider::ColourIds::rotarySliderOutlineColourId, Colours::yellow);
    m_oWaveSlider.addListener (this);
    m_oWaveSlider.setRange (0.0, 1.0, 1.f/3);
    
    addAndMakeVisible (m_oGainSlider);
    m_oGainSlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    m_oGainSlider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
	m_oGainSlider.setColour(Slider::ColourIds::rotarySliderFillColourId, Colours::white);
	m_oGainSlider.setColour(Slider::ColourIds::rotarySliderOutlineColourId, Colours::yellow);
    m_oGainSlider.addListener (this);
    m_oGainSlider.setRange (0.0, 1.0, 0.01);

    addAndMakeVisible (m_oDelaySlider);
    m_oDelaySlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    m_oDelaySlider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
	m_oDelaySlider.setColour(Slider::ColourIds::rotarySliderFillColourId, Colours::white);
	m_oDelaySlider.setColour(Slider::ColourIds::rotarySliderOutlineColourId, Colours::yellow);
    m_oDelaySlider.addListener (this);
    m_oDelaySlider.setRange (0.0, 1.0, 0.01);


	JUCE_COMPILER_WARNING("path needs to make sense on mac")
	m_oSineImage.setImage(ImageFileFormat::loadFrom(File::createFileWithoutCheckingPath("C:\\Users\\Vincent\\Documents\\git\\sBMP4\\icons\\sine.png")));
	m_oSawImage.setImage(ImageFileFormat::loadFrom(File::createFileWithoutCheckingPath("C:\\Users\\Vincent\\Documents\\git\\sBMP4\\icons\\saw.png")));
	m_oSquareImage.setImage(ImageFileFormat::loadFrom(File::createFileWithoutCheckingPath("C:\\Users\\Vincent\\Documents\\git\\sBMP4\\icons\\square.png")));
	m_oTriangleImage.setImage(ImageFileFormat::loadFrom(File::createFileWithoutCheckingPath("C:\\Users\\Vincent\\Documents\\git\\sBMP4\\icons\\noise.png")));
	m_oLogoImage.setImage(ImageFileFormat::loadFrom(File::createFileWithoutCheckingPath("C:\\Users\\Vincent\\Documents\\git\\sBMP4\\icons\\main.png")));

	addAndMakeVisible(m_oSineImage);
	addAndMakeVisible(m_oSawImage);
	addAndMakeVisible(m_oSquareImage);
	addAndMakeVisible(m_oTriangleImage);
	addAndMakeVisible(m_oLogoImage);

    // add some labels for the sliders
    m_oWaveLabel.setFont (Font (11.0f));
	m_oWaveLabel.setColour(Label::textColourId, Colours::white);
	m_oWaveLabel.setJustificationType(Justification::centred);
	addAndMakeVisible(m_oWaveLabel);

    m_oGainLabel.setFont (Font (11.0f));
	m_oGainLabel.setColour(Label::textColourId, Colours::white);
	m_oGainLabel.setJustificationType(Justification::centred);
	addAndMakeVisible(m_oGainLabel);

    m_oDelayLabel.setFont (Font (11.0f));
	m_oDelayLabel.setColour(Label::textColourId, Colours::white);
	m_oDelayLabel.setJustificationType(Justification::centred);
	addAndMakeVisible(m_oDelayLabel);
    // add the midi keyboard component..
    addAndMakeVisible (m_oMidiKeyboard);

    // add a label that will display the current timecode and status..
//    addAndMakeVisible (m_oInfoLabel);
//    m_oInfoLabel.setColour (Label::textColourId, Colours::blue);

    // add the triangular m_pResizer component for the bottom-right of the UI
    addAndMakeVisible (m_pResizer = new ResizableCornerComponent (this, &m_oResizeLimits));
    m_oResizeLimits.setSizeLimits (20+4*65+20, 150, 800, 300);

    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize (processor.getDimensions().first, processor.getDimensions().second);

    startTimer (50);
}

sBMP4AudioProcessorEditor::~sBMP4AudioProcessorEditor()
{
}

//==============================================================================
void sBMP4AudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colour::greyLevel(0.20f));
}

void sBMP4AudioProcessorEditor::resized() {
    //m_oInfoLabel.setBounds (10, 4, 400, 25);

    int x = 20, y = 25, w = 75, sh = 40, wh = 20;
    m_oWaveSlider.setBounds	(x, y,		w, sh);
	m_oWaveLabel.setBounds(x, y + 1.5*wh, w, wh);

	m_oSineImage.setBounds(x - 5, y + 25, 20, 20);
	m_oSquareImage.setBounds(x + 0, y - 15, 20, 20);
	m_oTriangleImage.setBounds(x + 7 * w / 10, y - 15, 20, 20);
	m_oSawImage.setBounds(x + 8 * w / 10, y + 25, 20, 20);

    m_oGainSlider.setBounds (x + w, y,		w, sh);
	m_oGainLabel.setBounds	(x + w, y+1.5*wh,	w, wh);
    
	m_oDelaySlider.setBounds(x + 2*w, y,	    w, sh);
	m_oDelayLabel.setBounds (x + 2*w, y+1.5*wh, w, wh);

	const int keyboardHeight = 70;

	//m_oLogoImage.setBounds(x + 3 * w, 5, 48, 48);
	m_oLogoImage.setBounds(getWidth() - (4+48), 5, 48, 48);

    
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
