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
    , m_oFilterLabel("", "LP filter")
	, m_oLfoLabel("", "LFO")
    , m_oGainLabel("", "gain")
    , m_oDelayLabel("", "delay")
    , m_oWaveSlider("wave")
    , m_oFilterSlider("Filter")
    , m_oGainSlider("gain")
    , m_oDelaySlider("delay")
	, m_oLfoSlider("LFO")
    , m_oSineImage("sine")
    , m_oSawImage("saw")
    , m_oTriangleImage("triangle")
    , m_oLogoImage("sBMP4")
{
    addSlider(&m_oWaveSlider	, 1.f/3);
	addSlider(&m_oFilterSlider	, .01);
	addSlider(&m_oGainSlider	, .01);
 	addSlider(&m_oDelaySlider	, .01);
	addSlider(&m_oLfoSlider		, .01);
	

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
	addLabel(&m_oFilterLabel);
	addLabel(&m_oLfoLabel);
	addLabel(&m_oGainLabel);
	addLabel(&m_oDelayLabel);

    // add the midi keyboard component..
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
    int x = 20, y = 25, w = 75, sh = 40, wh = 20;
    m_oWaveSlider.setBounds	    (x, y,		w, sh);
	m_oWaveLabel.setBounds      (x, y + 1.5*wh, w, wh);

	m_oSineImage.setBounds      (x - 5, y + 25, 20, 20);
	m_oSquareImage.setBounds    (x + 0, y - 15, 20, 20);
	m_oTriangleImage.setBounds  (x + 7 * w / 10, y - 15, 20, 20);
	m_oSawImage.setBounds       (x + 8 * w / 10, y + 25, 20, 20);

	m_oLfoSlider.setBounds		(x + w, y, w, sh);
	m_oLfoLabel.setBounds		(x + w, y + 1.5*wh, w, wh);

    m_oFilterSlider.setBounds   (x + 2 * w, y,		w, sh);
	m_oFilterLabel.setBounds	(x + 2 * w, y+1.5*wh,	w, wh);

	m_oDelaySlider.setBounds(x + 3 * w, y, w, sh);
	m_oDelayLabel.setBounds(x + 3 * w, y + 1.5*wh, w, wh);

	m_oGainSlider.setBounds		(x + 4 * w, y, w, sh);
	m_oGainLabel.setBounds		(x + 4 * w, y + 1.5*wh, w, wh);

	const int keyboardHeight = 70;

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
    m_oGainSlider  .setValue(ourProcessor.getParameter(paramGain),      dontSendNotification);
    m_oDelaySlider .setValue(ourProcessor.getParameter(paramDelay),     dontSendNotification);
    m_oWaveSlider  .setValue(ourProcessor.getParameter(paramWave),      dontSendNotification); 
    m_oFilterSlider.setValue(ourProcessor.getParameter(paramFilterFr),  dontSendNotification);
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
	}

}