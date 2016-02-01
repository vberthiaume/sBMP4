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

#ifndef __PLUGINEDITOR_H_4ACCBAA__
#define __PLUGINEDITOR_H_4ACCBAA__

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/** This is the editor component that our filter will display.
*/
class sBMP4AudioProcessorEditor
	: public AudioProcessorEditor
	, public SliderListener
	, public ButtonListener
	, public Timer
{
public:
    sBMP4AudioProcessorEditor (sBMP4AudioProcessor&);
    ~sBMP4AudioProcessorEditor();

    //==============================================================================
    void timerCallback() override;
    void paint (Graphics&) override;
    void resized() override;
    void sliderValueChanged (Slider*) override;
	void buttonClicked(Button*) override;

private:
    MidiKeyboardComponent m_oMidiKeyboard;
	Label m_oWaveLabel, m_oFilterLabel, m_oGainLabel, m_oDelayLabel, m_oLfoLabel, m_oQLabel;
	Slider m_oWaveSlider, m_oFilterSlider, m_oGainSlider, m_oDelaySlider, m_oLfoSlider, m_oQSlider;
	ToggleButton m_oLfoTogBut;
	ImageComponent m_oSineImage, m_oSawImage, m_oSquareImage, m_oTriangleImage, m_oLogoImage;
    ScopedPointer<ResizableCornerComponent> m_pResizer;
    ComponentBoundsConstrainer m_oResizeLimits;

    AudioPlayHead::CurrentPositionInfo m_oLastDisplayedPosition;

    sBMP4AudioProcessor& getProcessor() const{
        return static_cast<sBMP4AudioProcessor&> (processor);
    }

	void addSlider(Slider* p_pSlider, const float &p_fIncrement);
	void addLabel(Label * p_pLabel);
	void addToggleButton(ToggleButton* p_pTogButton);
};


#endif  // __PLUGINEDITOR_H_4ACCBAA__
