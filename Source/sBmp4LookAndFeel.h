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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/** Custom Look And Feel subclasss.
 
 Simply override the methods you need to, anything else will be inherited from the base class.
 It's a good idea not to hard code your colours, use the findColour method along with appropriate
 ColourIds so you can set these on a per-component basis.
 */
class sBmp4LookAndFeel    : public LookAndFeel_V3 {
private:
    Font m_Font;
    float m_fFontSize;
    Colour m_BackGroundAndFieldColor;
public:
    sBmp4LookAndFeel(){
        m_BackGroundAndFieldColor = Colours::darkgrey;
        m_Font = Font(juce::CustomTypeface::createSystemTypefaceFor(BinaryData::After_Shok_ttf, (size_t) BinaryData::After_Shok_ttfSize));
        m_fFontSize = 10.f;
        m_Font.setHeight(m_fFontSize);
    }
    
    Font getFont(){
        return m_Font;
    }
    
    Font getLabelFont (Label & label) override{
        return m_Font;
    }
    Font getComboBoxFont (ComboBox & comboBox) override{
        return m_Font;
    }
    Font getTextButtonFont (TextButton &, int buttonHeight) override{
        return m_Font;
    }
    Font getMenuBarFont	(MenuBarComponent &, int itemIndex, const String & itemText) override{
        return m_Font;
    }
    Colour getBackgroundColor(){
        //        return Colours::darkblue;
        //        return Colours::dodgerblue;
        return m_BackGroundAndFieldColor;
    }
    
    Colour getFieldColor(){
        //        return Colours::darkblue;
        //        return Colours::dodgerblue;
        return m_BackGroundAndFieldColor;
    }
    
    Colour getFontColour(){
        //        return Colours::azure;
        return Colours::yellow;
    }
    
    Colour getSliderColour(){
        //        return Colours::azure;
        return Colours::black;
    }
    
    void drawRoundThumb (Graphics& g, const float x, const float y, const float diameter, const Colour& colour, float outlineThickness) {
        const juce::Rectangle<float> a (x, y, diameter, diameter);
        const float halfThickness = outlineThickness * 0.5f;
        
        Path p;
        p.addEllipse (x + halfThickness, y + halfThickness, diameter - outlineThickness, diameter - outlineThickness);
        
        const DropShadow ds (Colours::black, 1, Point<int> (0, 0));
        ds.drawForPath (g, p);
        
        g.setColour (colour);
        g.fillPath (p);
        
        g.setColour (colour.brighter());
        g.strokePath (p, PathStrokeType (outlineThickness));
    }
    
    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown) override {
        Colour baseColour (backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f).withMultipliedAlpha (button.isEnabled() ? 0.9f : 0.5f));
        
        if (isButtonDown || isMouseOverButton){
            baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.1f);
        }
        const bool flatOnLeft   = button.isConnectedOnLeft();
        const bool flatOnRight  = button.isConnectedOnRight();
        const bool flatOnTop    = button.isConnectedOnTop();
        const bool flatOnBottom = button.isConnectedOnBottom();
        
        const float width  = button.getWidth() - 1.0f;
        const float height = button.getHeight() - 1.0f;
        
        if (width > 0 && height > 0) {
            const float cornerSize = jmin (15.0f, jmin (width, height) * 0.45f);
            const float lineThickness = cornerSize * 0.1f;
            const float halfThickness = lineThickness * 0.5f;
            
            Path outline;
            outline.addRoundedRectangle (0.5f + halfThickness, 0.5f + halfThickness, width - lineThickness, height - lineThickness,
                                         cornerSize, cornerSize,
                                         ! (flatOnLeft  || flatOnTop),
                                         ! (flatOnRight || flatOnTop),
                                         ! (flatOnLeft  || flatOnBottom),
                                         ! (flatOnRight || flatOnBottom));
            
            const Colour outlineColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId : TextButton::textColourOffId));
            g.setColour (baseColour);
            g.fillPath (outline);
            
            if (! button.getToggleState()) {
                g.setColour (outlineColour);
                g.strokePath (outline, PathStrokeType (lineThickness));
            }
        }
    }
    
    void drawTickBox (Graphics& g, Component& component, float x, float y, float w, float h, bool ticked, bool isEnabled, bool isMouseOverButton, bool isButtonDown) override {
        const float boxSize = w * 0.7f;
        bool isDownOrDragging = component.isEnabled() && (component.isMouseOverOrDragging() || component.isMouseButtonDown());
        const Colour colour (component.findColour (TextButton::buttonColourId).withMultipliedSaturation ((component.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f).withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.7f));
        
        drawRoundThumb (g, x, y + (h - boxSize) * 0.5f, boxSize, colour, isEnabled ? ((isButtonDown || isMouseOverButton) ? 1.1f : 0.5f) : 0.3f);
        
        if (ticked) {
            const Path tick (LookAndFeel_V2::getTickShape (6.0f));
            g.setColour (isEnabled ? findColour (TextButton::buttonOnColourId) : Colours::grey);
            const float scale = 9.0f;
            const AffineTransform trans (AffineTransform::scale (w / scale, h / scale).translated (x - 2.5f, y + 1.0f));
            g.fillPath (tick, trans);
        }
    }
    
    void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider) override {
        const float sliderRadius = (float) (getSliderThumbRadius (slider) - 2);
        bool isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());
        Colour knobColour (slider.findColour (Slider::thumbColourId).withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f).withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f));
        
        if (style == Slider::LinearHorizontal || style == Slider::LinearVertical) {
            float kx, ky;
            
            if (style == Slider::LinearVertical) {
                kx = x + width * 0.5f;
                ky = sliderPos;
            } else {
                kx = sliderPos;
                ky = y + height * 0.5f;
            }
            
            const float outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;
            
            drawRoundThumb (g,
                            kx - sliderRadius,
                            ky - sliderRadius,
                            sliderRadius * 2.0f,
                            knobColour, outlineThickness);
        } else {
            // Just call the base class for the demo
            LookAndFeel_V2::drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }
    
    void drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider) override {
        g.fillAll (slider.findColour (Slider::backgroundColourId));
        
        if (style == Slider::LinearBar || style == Slider::LinearBarVertical) {
            const float fx = (float) x, fy = (float) y, fw = (float) width, fh = (float) height;
            
            Path p;
            
            if (style == Slider::LinearBarVertical)
                p.addRectangle (fx, sliderPos, fw, 1.0f + fh - sliderPos);
                else
                    p.addRectangle (fx, fy, sliderPos - fx, fh);
                    
                    
                    Colour baseColour (slider.findColour (Slider::rotarySliderFillColourId)
                                       .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                                       .withMultipliedAlpha (0.8f));
                    
                    g.setColour (baseColour);
                    g.fillPath (p);
                    
                    const float lineThickness = jmin (15.0f, jmin (width, height) * 0.45f) * 0.1f;
            g.drawRect (slider.getLocalBounds().toFloat(), lineThickness);
        } else {
            drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }
    
    void drawLinearSliderBackground (Graphics& g, int x, int y, int width, int height, float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/, const Slider::SliderStyle /*style*/, Slider& slider) override {
        const float sliderRadius = getSliderThumbRadius (slider) - 5.0f;
        Path on, off;
        if (slider.isHorizontal()) {
            const float iy = y + height * 0.5f - sliderRadius * 0.5f;
            juce::Rectangle<float> r (x - sliderRadius * 0.5f, iy, width + sliderRadius, sliderRadius);
            const float onW = r.getWidth() * ((float) slider.valueToProportionOfLength (slider.getValue()));
            on.addRectangle (r.removeFromLeft (onW));
            off.addRectangle (r);
        } else {
            const float ix = x + width * 0.5f - sliderRadius * 0.5f;
            juce::Rectangle<float> r (ix, y - sliderRadius * 0.5f, sliderRadius, height + sliderRadius);
            const float onH = r.getHeight() * ((float) slider.valueToProportionOfLength (slider.getValue()));
            on.addRectangle (r.removeFromBottom (onH));
            off.addRectangle (r);
        }
        g.setColour (slider.findColour (Slider::rotarySliderFillColourId));
        g.fillPath (on);
        g.setColour (slider.findColour (Slider::trackColourId));
        g.fillPath (off);
    }
    
    void drawToggleButton (Graphics& g, ToggleButton& button, bool isMouseOverButton, bool isButtonDown) override {
        if (button.hasKeyboardFocus (true))
        {
            g.setColour (button.findColour (TextEditor::focusedOutlineColourId));
            g.drawRect (0, 0, button.getWidth(), button.getHeight());
        }
        
        float fontSize = jmin (15.0f, button.getHeight() * 0.75f);
        const float tickWidth = fontSize * 1.1f;
        
        drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
                     tickWidth, tickWidth,
                     button.getToggleState(),
                     button.isEnabled(),
                     isMouseOverButton,
                     isButtonDown);
        
        g.setColour (button.findColour (ToggleButton::textColourId));
        g.setFont(m_Font);
        
        if (! button.isEnabled())
            g.setOpacity (0.5f);
        
        const int textX = (int) tickWidth + 5;
        
        g.drawFittedText (button.getButtonText(),
                          textX, 0,
                          button.getWidth() - textX - 2, button.getHeight(),
                          Justification::centredLeft, 10);
    }
    
    void drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown) override{
        const Rectangle<int> activeArea (button.getActiveArea());
        
        
        const TabbedButtonBar::Orientation o = button.getTabbedButtonBar().getOrientation();
        
        const Colour bkg (button.getTabBackgroundColour());
        
        if (button.getToggleState())
        {
            g.setColour (bkg);
        }
        else
        {
            Point<int> p1, p2;
            
            switch (o)
            {
                case TabbedButtonBar::TabsAtBottom:   p1 = activeArea.getBottomLeft(); p2 = activeArea.getTopLeft();    break;
                case TabbedButtonBar::TabsAtTop:      p1 = activeArea.getTopLeft();    p2 = activeArea.getBottomLeft(); break;
                case TabbedButtonBar::TabsAtRight:    p1 = activeArea.getTopRight();   p2 = activeArea.getTopLeft();    break;
                case TabbedButtonBar::TabsAtLeft:     p1 = activeArea.getTopLeft();    p2 = activeArea.getTopRight();   break;
                default:                              jassertfalse; break;
            }
            
            g.setGradientFill (ColourGradient (bkg.brighter (0.2f), (float) p1.x, (float) p1.y,
                                               bkg.darker (0.1f),   (float) p2.x, (float) p2.y, false));
        }
        
        g.fillRect (activeArea);
        
        g.setColour (button.findColour (TabbedButtonBar::tabOutlineColourId));
        
        Rectangle<int> r (activeArea);
        
        if (o != TabbedButtonBar::TabsAtBottom)   g.fillRect (r.removeFromTop (1));
        if (o != TabbedButtonBar::TabsAtTop)      g.fillRect (r.removeFromBottom (1));
        if (o != TabbedButtonBar::TabsAtRight)    g.fillRect (r.removeFromLeft (1));
        if (o != TabbedButtonBar::TabsAtLeft)     g.fillRect (r.removeFromRight (1));
        
        const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
        
        Colour col (bkg.contrasting().withMultipliedAlpha (alpha));
        
        if (TabbedButtonBar* bar = button.findParentComponentOfClass<TabbedButtonBar>())
        {
            TabbedButtonBar::ColourIds colID = button.isFrontTab() ? TabbedButtonBar::frontTextColourId
            : TabbedButtonBar::tabTextColourId;
            
            if (bar->isColourSpecified (colID))
                col = bar->findColour (colID);
            else if (isColourSpecified (colID))
                col = findColour (colID);
        }
        
        const Rectangle<float> area (button.getTextArea().toFloat());
        
        float length = area.getWidth();
        float depth  = area.getHeight();
        
        if (button.getTabbedButtonBar().isVertical())
            std::swap (length, depth);
        
        TextLayout textLayout;
        createTabTextLayout (button, length, depth, col, textLayout);
        
        AffineTransform t;
        
        switch (o)
        {
            case TabbedButtonBar::TabsAtLeft:   t = t.rotated (float_Pi * -0.5f).translated (area.getX(), area.getBottom()); break;
            case TabbedButtonBar::TabsAtRight:  t = t.rotated (float_Pi *  0.5f).translated (area.getRight(), area.getY()); break;
            case TabbedButtonBar::TabsAtTop:
            case TabbedButtonBar::TabsAtBottom: t = t.translated (area.getX(), area.getY()); break;
            default:                            jassertfalse; break;
        }
        
        g.addTransform (t);
        textLayout.draw (g, Rectangle<float> (length, depth));
    }
    
    void createTabTextLayout (const TabBarButton& button, float length, float depth, Colour colour, TextLayout& textLayout)
    {
        Font font (m_Font);
        font.setHeight(depth * 0.35f);
        font.setUnderline (button.hasKeyboardFocus (false));
        
        AttributedString s;
        s.setJustification (Justification::centred);
        s.append (button.getButtonText().trim(), font, colour);
        
        textLayout.createLayout (s, length);
    }
    
//    void drawTabButtonText (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown) override
//    {
//        const Rectangle<float> area (button.getTextArea().toFloat());
//        
//        float length = area.getWidth();
//        float depth  = area.getHeight();
//        
//        if (button.getTabbedButtonBar().isVertical())
//            std::swap (length, depth);
//        
//        Font font (m_Font);
//        font.setHeight(depth * 0.35f);
//        font.setUnderline (button.hasKeyboardFocus (false));
//        
//        AffineTransform t;
//        
//        switch (button.getTabbedButtonBar().getOrientation())
//        {
//            case TabbedButtonBar::TabsAtLeft:   t = t.rotated (float_Pi * -0.5f).translated (area.getX(), area.getBottom()); break;
//            case TabbedButtonBar::TabsAtRight:  t = t.rotated (float_Pi *  0.5f).translated (area.getRight(), area.getY()); break;
//            case TabbedButtonBar::TabsAtTop:
//            case TabbedButtonBar::TabsAtBottom: t = t.translated (area.getX(), area.getY()); break;
//            default:                            jassertfalse; break;
//        }
//        
//        Colour col;
//        
//        if (button.isFrontTab() && (button.isColourSpecified (TabbedButtonBar::frontTextColourId)
//                                    || isColourSpecified (TabbedButtonBar::frontTextColourId)))
//            col = findColour (TabbedButtonBar::frontTextColourId);
//        else if (button.isColourSpecified (TabbedButtonBar::tabTextColourId)
//                 || isColourSpecified (TabbedButtonBar::tabTextColourId))
//            col = findColour (TabbedButtonBar::tabTextColourId);
//        else
//            col = button.getTabBackgroundColour().contrasting();
//        
//        const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
//        
//        g.setColour (col.withMultipliedAlpha (alpha));
//        g.setFont (font);
//        g.addTransform (t);
//        
//        g.drawFittedText (button.getButtonText().trim(),
//                          0, 0, (int) length, (int) depth,
//                          Justification::centred,
//                          jmax (1, ((int) depth) / 12));
//    }
    
//        int getTabButtonOverlap (int /*tabDepth*/)            override { return -1; }
//        int getTabButtonSpaceAroundImage()                    override { return 0; }

        
        //    //we don't use those, so far
        //    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override {
        //        const float radius = jmin (width / 2, height / 2) - 2.0f;
        //        const float centreX = x + width * 0.5f;
        //        const float centreY = y + height * 0.5f;
        //        const float rx = centreX - radius;
        //        const float ry = centreY - radius;
        //        const float rw = radius * 2.0f;
        //        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        //        const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();
        //
        //        if (slider.isEnabled())
        //            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        //        else
        //            g.setColour (Colour (0x80808080));
        //
        //        {
        //            Path filledArc;
        //            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, 0.0);
        //            g.fillPath (filledArc);
        //        }
        //
        //        {
        //            const float lineThickness = jmin (15.0f, jmin (width, height) * 0.45f) * 0.1f;
        //            Path outlineArc;
        //            outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, 0.0);
        //            g.strokePath (outlineArc, PathStrokeType (lineThickness));
        //        }
        //    }
};
        
