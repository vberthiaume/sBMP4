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
    , m_oWaveSlider("wave")
	, m_oWaveLabel("", "wave")
	, m_oFilterSlider("Filter")
    , m_oFilterLabel("", "LP filter")
	, m_oQSlider("Resonance")
    , m_oQLabel("", "Resonance")
	, m_oLfoSlider("LFO")
	, m_oLfoLabel("", "LFO")
	, m_oDelaySlider("delay")
    , m_oDelayLabel("", "delay")
    , m_oGainSlider("gain")
    , m_oGainLabel("", "gain")
    , m_oSineImage("sine")
    , m_oSawImage("saw")
    , m_oTriangleImage("triangle")
    , m_oLogoImage("sBMP4")
{
    addSlider(&m_oWaveSlider,	1.f/3);
	addSlider(&m_oLfoSlider,	.01);
	addSlider(&m_oFilterSlider, .01);
	JUCE_COMPILER_WARNING("we need to use [0,1] inside editor")
	addSlider(&m_oQSlider,		.01, minQ);
 	addSlider(&m_oDelaySlider,	.01);
	addSlider(&m_oGainSlider,	.01);

	addToggleButton(&m_oLfoTogBut); 

JUCE_COMPILER_WARNING("path needs to make sense on mac, not be hard-coded")
		String strPrefix;
#ifdef JUCE_LINUX
        strPrefix = "/home/vberthiaume/Documents/git/sBMP4/Source/DspFilters";
#elif JUCE_MAC

#elif JUCE_WINDOWS
	strPrefix = "C:\\Users\\barth\\Documents\\git\\sBMP4\\icons\\";
#endif
	m_oSineImage.setImage(		ImageFileFormat::loadFrom(File(strPrefix + "sine.png")));
	m_oSawImage.setImage(		ImageFileFormat::loadFrom(File(strPrefix + "saw.png")));
	m_oSquareImage.setImage(	ImageFileFormat::loadFrom(File(strPrefix + "square.png")));
	m_oTriangleImage.setImage(	ImageFileFormat::loadFrom(File(strPrefix + "triangle.png")));
	m_oLogoImage.setImage(		ImageFileFormat::loadFrom(File(strPrefix + "main.png")));
	
	addAndMakeVisible(m_oSineImage);
	addAndMakeVisible(m_oSawImage);
	addAndMakeVisible(m_oSquareImage);
	addAndMakeVisible(m_oTriangleImage);
	addAndMakeVisible(m_oLogoImage);

    // add some labels for the sliders
	addLabel(&m_oWaveLabel);
	addLabel(&m_oLfoLabel);
	addLabel(&m_oFilterLabel);
	addLabel(&m_oQLabel);
	addLabel(&m_oGainLabel);
	addLabel(&m_oDelayLabel);

    addAndMakeVisible (m_oMidiKeyboard);

    // add the triangular m_pResizer component for the bottom-right of the UI
    addAndMakeVisible (m_pResizer = new ResizableCornerComponent (this, &m_oResizeLimits));
    m_oResizeLimits.setSizeLimits (processor.getDimensions().first, processor.getDimensions().second, 
								 2*processor.getDimensions().first, 2*processor.getDimensions().second);

    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize (processor.getDimensions().first, processor.getDimensions().second);

    startTimer (50);
}

sBMP4AudioProcessorEditor::~sBMP4AudioProcessorEditor()
{
}

JUCE_COMPILER_WARNING("will need to remove last 2 args here when no longer needed")
void sBMP4AudioProcessorEditor::addSlider(Slider* p_pSlider, const float &p_fIncrement, int p_iLowerBound, int p_iHigherBound){
	addAndMakeVisible(*p_pSlider);
	p_pSlider->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
	p_pSlider->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
	p_pSlider->setColour(Slider::ColourIds::rotarySliderFillColourId, Colours::white);
	p_pSlider->setColour(Slider::ColourIds::rotarySliderOutlineColourId, Colours::yellow);
	p_pSlider->addListener(this);
	p_pSlider->setRange(p_iLowerBound, p_iHigherBound, p_fIncrement);
}

void sBMP4AudioProcessorEditor::addLabel(Label * p_pLabel){
	p_pLabel->setFont(Font(11.0f));
	p_pLabel->setColour(Label::textColourId, Colours::white);
	p_pLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(p_pLabel);
}

void sBMP4AudioProcessorEditor::addToggleButton(ToggleButton* p_pTogButton){
	p_pTogButton->setColour(Label::textColourId, Colours::yellow);
	p_pTogButton->addListener(this);
	addAndMakeVisible(p_pTogButton);
}

//==============================================================================
void sBMP4AudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colour::greyLevel(0.20f));
}

void sBMP4AudioProcessorEditor::resized() {
    int x = s_iXMargin, y = s_iYMargin, iCurCol = 0, iCurRow = 0;
	int iTogButSize = 25;

    m_oWaveSlider.setBounds	    (x, y,		s_iSliderWidth, s_iSliderHeight);
	m_oWaveLabel.setBounds      (x, y + 1.5*s_iLabelHeight, s_iSliderWidth, s_iLabelHeight);

	m_oSineImage.setBounds      (x - 5, y + 25, 20, 20);
	m_oSquareImage.setBounds    (x + 0, y - 15, 20, 20);
	m_oTriangleImage.setBounds  (x + 7 * s_iSliderWidth / 10, y - 15, 20, 20);
	m_oSawImage.setBounds       (x + 8 * s_iSliderWidth / 10, y + 25, 20, 20);

	++iCurRow;

	m_oLfoSlider.setBounds		(x + iCurCol * s_iSliderWidth, y + iCurRow * (s_iSliderHeight + s_iLabelHeight), s_iSliderWidth, s_iSliderHeight);
	m_oLfoLabel.setBounds		(x + iCurCol * s_iSliderWidth+iTogButSize, y + iCurRow * (s_iSliderHeight + 2.5*s_iLabelHeight), s_iSliderWidth-(2*iTogButSize), s_iLabelHeight);
	m_oLfoTogBut.setBounds		(x, y + iCurRow * (s_iSliderHeight + s_iLabelHeight) + iTogButSize, iTogButSize, iTogButSize);
	--iCurRow;
	++iCurCol;

    m_oFilterSlider.setBounds   (x + iCurCol * s_iSliderWidth, y,		s_iSliderWidth, s_iSliderHeight);
	m_oFilterLabel.setBounds	(x + iCurCol *  s_iSliderWidth, y + 1.5*s_iLabelHeight,	s_iSliderWidth, s_iLabelHeight);

	++iCurRow;

	m_oQSlider.setBounds		(x + iCurCol * s_iSliderWidth, y + iCurRow * (s_iSliderHeight + s_iLabelHeight), s_iSliderWidth, s_iSliderHeight);
	m_oQLabel.setBounds			(x + iCurCol * s_iSliderWidth, y + iCurRow * (s_iSliderHeight + 2.5*s_iLabelHeight), s_iSliderWidth, s_iLabelHeight);

	--iCurRow;
	++iCurCol;

	m_oDelaySlider.setBounds	(x + iCurCol *  s_iSliderWidth, y, s_iSliderWidth, s_iSliderHeight);
	m_oDelayLabel.setBounds		(x + iCurCol *  s_iSliderWidth, y + 1.5*s_iLabelHeight, s_iSliderWidth, s_iLabelHeight);

	++iCurCol;

	m_oGainSlider.setBounds		(x + iCurCol *  s_iSliderWidth, y, s_iSliderWidth, s_iSliderHeight);
	m_oGainLabel.setBounds		(x + iCurCol *  s_iSliderWidth, y + 1.5*s_iLabelHeight, s_iSliderWidth, s_iLabelHeight);



	
	m_oLogoImage.setBounds(getWidth() - s_iLogoW, 5, s_iLogoW, s_iLogoH);
    m_oMidiKeyboard.setBounds (4, getHeight() - s_iKeyboardHeight - 4, getWidth() - 8, s_iKeyboardHeight);
    m_pResizer->setBounds (getWidth() - 16, getHeight() - 16, 16, 16);
    getProcessor().setDimensions(std::make_pair(getWidth(), getHeight()));
}

//==============================================================================
// This timer periodically checks whether any of the filter's parameters have changed...
void sBMP4AudioProcessorEditor::timerCallback() {
    sBMP4AudioProcessor& ourProcessor = getProcessor();
    m_oGainSlider	.setValue(ourProcessor.getParameter(paramGain),     dontSendNotification);
    m_oDelaySlider	.setValue(ourProcessor.getParameter(paramDelay),    dontSendNotification);
    m_oWaveSlider	.setValue(ourProcessor.getParameter(paramWave),     dontSendNotification); 
    m_oFilterSlider	.setValue(ourProcessor.getParameter(paramFilterFr), dontSendNotification);
	m_oQSlider		.setValue(ourProcessor.getParameter(paramQ),		dontSendNotification);
	m_oLfoSlider	.setValue(ourProcessor.getParameter(paramLfoFr),	dontSendNotification);
	//m_oLfoTogBut	.setToggleState(ourProcessor.getParameter(paramLfoOn), dontSendNotification);
	m_oLfoTogBut	.setToggleState(ourProcessor.getLfoOn(), dontSendNotification);
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
	} else if (slider == &m_oLfoSlider) {
		getProcessor().setParameterNotifyingHost(paramLfoFr, (float)m_oLfoSlider.getValue());
	} else if (slider == &m_oQSlider) {
		getProcessor().setParameterNotifyingHost(paramQ, (float)m_oQSlider.getValue());
	}

}
void sBMP4AudioProcessorEditor::buttonClicked(Button* p_pButtonClicked){
	if (p_pButtonClicked == &m_oLfoTogBut){
		//getProcessor().setParameterNotifyingHost(paramLfoFr, m_oLfoTogBut.getToggleState());
		getProcessor().setLfoOn(m_oLfoTogBut.getToggleState());
	}
}