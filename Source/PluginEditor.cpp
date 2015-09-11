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
sBMP4AudioProcessorEditor::sBMP4AudioProcessorEditor(sBMP4AudioProcessor& processor)
    : AudioProcessorEditor(processor)
    , m_oMidiKeyboard(processor.m_oKeyboardState, MidiKeyboardComponent::horizontalKeyboard)
    , m_oWaveLabel("", "wave")
    , m_oFilterLabel("", "filter")
    , m_oInfoLabel(String::empty)
    , m_oGainLabel("", "gain")
    , m_oDelayLabel("", "delay")
    , m_oWaveSlider("wave")
    , m_oFilterSlider("Filter")
    , m_oGainSlider("gain")
    , m_oDelaySlider("delay")
    , m_oSineImage("sine")
    , m_oSawImage("saw")
    , m_oTriangleImage("triangle")
    , m_oLogoImage("sBMP4")
{
    
    addSlider(&m_oWaveSlider	, 1.f/3);
	addSlider(&m_oFilterSlider	, .01);    
	addSlider(&m_oGainSlider	, .01);
 	addSlider(&m_oDelaySlider	, .01);

	JUCE_COMPILER_WARNING("path needs to make sense on mac")
		String strPrefix;
#ifdef JUCE_LINUX
        strPrefix = "/home/vberthiaume/Documents/git/sBMP4/Source/DspFilters";
#elif JUCE_MAC
#elif JUCE_WINDOWS
	strPrefix = "C:\\Users\\Vincent\\Documents\\git\\sBMP4\\icons\\";
#endif
	m_oSineImage.	 setImage(ImageFileFormat::loadFrom(File(strPrefix + "sine.png")));
	m_oSawImage.	 setImage(ImageFileFormat::loadFrom(File(strPrefix + "saw.png")));
	m_oSquareImage.	 setImage(ImageFileFormat::loadFrom(File(strPrefix + "square.png")));
	m_oTriangleImage.setImage(ImageFileFormat::loadFrom(File(strPrefix + "triangle.png")));
	m_oLogoImage.	 setImage(ImageFileFormat::loadFrom(File(strPrefix + "main.png")));


	addAndMakeVisible(m_oSineImage);
	addAndMakeVisible(m_oSawImage);
	addAndMakeVisible(m_oSquareImage);
	addAndMakeVisible(m_oTriangleImage);
	addAndMakeVisible(m_oLogoImage);

    // add some labels for the sliders
	addLabel(&m_oWaveLabel);
	addLabel(&m_oFilterLabel);
	addLabel(&m_oGainLabel);
	addLabel(&m_oDelayLabel);

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

void sBMP4AudioProcessorEditor::addSlider(Slider* p_pSlider, const float &p_fIncrement){
	addAndMakeVisible(*p_pSlider);
	p_pSlider->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
	p_pSlider->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
	p_pSlider->setColour(Slider::ColourIds::rotarySliderFillColourId, Colours::white);
	p_pSlider->setColour(Slider::ColourIds::rotarySliderOutlineColourId, Colours::yellow);
	p_pSlider->addListener(this);
	p_pSlider->setRange(0.0, 1.0, p_fIncrement);
}

void sBMP4AudioProcessorEditor::addLabel(Label * p_pLabel){
	p_pLabel->setFont(Font(11.0f));
	p_pLabel->setColour(Label::textColourId, Colours::white);
	p_pLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(p_pLabel);
}

//==============================================================================
void sBMP4AudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colour::greyLevel(0.20f));
}

void sBMP4AudioProcessorEditor::resized() {
    //m_oInfoLabel.setBounds (10, 4, 400, 25);

    int x = 20, y = 25, w = 75, sh = 40, wh = 20;
    m_oWaveSlider.setBounds	    (x, y,		w, sh);
	m_oWaveLabel.setBounds      (x, y + 1.5*wh, w, wh);

	m_oSineImage.setBounds      (x - 5, y + 25, 20, 20);
	m_oSquareImage.setBounds    (x + 0, y - 15, 20, 20);
	m_oTriangleImage.setBounds  (x + 7 * w / 10, y - 15, 20, 20);
	m_oSawImage.setBounds       (x + 8 * w / 10, y + 25, 20, 20);

    m_oFilterSlider.setBounds   (x + w, y,		w, sh);
	m_oFilterLabel.setBounds	(x + w, y+1.5*wh,	w, wh);
    
	m_oGainSlider.setBounds(x + 2 * w, y, w, sh);
	m_oGainLabel.setBounds(x + 2 * w, y + 1.5*wh, w, wh);

	m_oDelaySlider.setBounds(x + 3 * w, y, w, sh);
	m_oDelayLabel.setBounds(x + 3 * w, y + 1.5*wh, w, wh);

	const int keyboardHeight = 70;

	//m_oLogoImage.setBounds(x + 3 * w, 5, 48, 48);
	int iLogoW = 75, iLogoH = 30;
	m_oLogoImage.setBounds(getWidth() - iLogoW, 5, iLogoW, iLogoH);

    
    m_oMidiKeyboard.setBounds (4, getHeight() - keyboardHeight - 4, getWidth() - 8, keyboardHeight);

    m_pResizer->setBounds (getWidth() - 16, getHeight() - 16, 16, 16);

    getProcessor().setDimensions(std::make_pair(getWidth(), getHeight()));
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
	} else if(slider == &m_oFilterSlider) {
		getProcessor().setParameterNotifyingHost(paramFilterFr, (float)m_oFilterSlider.getValue());
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
