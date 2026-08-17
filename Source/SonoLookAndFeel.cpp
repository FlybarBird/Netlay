// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception
// Copyright (C) 2020 Jesse Chappell


#include "SonoLookAndFeel.h"
#include "SonoDrawableButton.h"
#include "SonoTextButton.h"
#include "SlimUi.h"

#include <JuceHeader.h>

#define OLDFONTSTUFF 1

#if OLDFONTSTUFF

float SonoLookAndFeel::fontScale = 1.0f;

#else

 #if JUCE_ANDROID
 float SonoLookAndFeel::fontScale = 1.0f;
 #elif JUCE_WINDOWS
 float SonoLookAndFeel::fontScale = 1.35f;
 #else
 float SonoLookAndFeel::fontScale = 1.25f;
 #endif

#endif

//==============================================================================

static TextLayout sonoLayoutTooltipText (const String& text, Colour colour) noexcept
{
    const float tooltipFontSize = 13.0f;
    const int maxToolTipWidth = 400;

    AttributedString s;
    s.setJustification (Justification::centred);
    s.append (text, Font (tooltipFontSize, Font::bold), colour);

    TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, (float) maxToolTipWidth);
    return tl;
}

SonoLookAndFeel::SonoLookAndFeel(bool useUniversalFont) : mUseUniversalFont(useUniversalFont)
{
    // setColour (mainBackgroundColourId, Colour::greyLevel (0.8f));
    //DBG("Sonolook and feel");
    
    setUsingNativeAlertWindows(true);

    //fontScale = 1.2; // 1.125;

    setColourScheme(getDarkColourScheme());

    getCurrentColourScheme().setUIColour(ColourScheme::UIColour::windowBackground, SlimUi::background());
    getCurrentColourScheme().setUIColour(ColourScheme::UIColour::widgetBackground, SlimUi::card());
    getCurrentColourScheme().setUIColour(ColourScheme::UIColour::outline, SlimUi::outline());

    setColour (Label::textColourId, SlimUi::text());
    setColour (Label::textWhenEditingColourId, SlimUi::text());
    
    setColour(ResizableWindow::backgroundColourId, SlimUi::background());
    
    setColour (TextButton::buttonColourId, SlimUi::cardRaised());
    setColour (TextButton::buttonOnColourId, SlimUi::accentPurple());
    setColour (TextButton::textColourOnId, SlimUi::text());
    setColour (TextButton::textColourOffId, SlimUi::text());

    setColour (ToggleButton::textColourId, SlimUi::textDim());

    
    setColour (SonoTextButton::outlineColourId, SlimUi::outline());

    setColour (ScrollBar::ColourIds::thumbColourId, SlimUi::accentPurple().withAlpha (0.7f));
    
    setColour (ComboBox::backgroundColourId, SlimUi::cardRaised());
    setColour (ComboBox::textColourId, SlimUi::text());
    setColour (ComboBox::outlineColourId, SlimUi::outline());

    setColour (TextEditor::backgroundColourId, SlimUi::sidebar());
    setColour (TextEditor::textColourId, SlimUi::text());
    setColour (TextEditor::highlightColourId, SlimUi::accentBlue().withAlpha (0.45f));
    setColour (TextEditor::outlineColourId, SlimUi::outline());
    setColour (TextEditor::focusedOutlineColourId, SlimUi::accentBlue().withAlpha (0.8f));

    setColour (Slider::backgroundColourId, SlimUi::cardRaised());
    setColour (Slider::rotarySliderOutlineColourId, SlimUi::cardRaised());
    setColour (Slider::textBoxTextColourId, SlimUi::textDim());
    setColour (Slider::textBoxBackgroundColourId, SlimUi::sidebar());
    setColour (Slider::textBoxHighlightColourId, SlimUi::accentBlue().withAlpha (0.35f));
    setColour (Slider::textBoxOutlineColourId, SlimUi::outline());
    
    setColour (Slider::trackColourId, SlimUi::accentBlue());
    setColour (Slider::thumbColourId, SlimUi::text());
    setColour (Slider::rotarySliderFillColourId, SlimUi::accentBlue());
    
    setColour (TabbedButtonBar::tabOutlineColourId, SlimUi::outline());
    setColour (TabbedButtonBar::frontTextColourId, SlimUi::accentLavender());
    setColour (TabbedButtonBar::tabTextColourId, SlimUi::textDim());


    
    setColour (ListBox::backgroundColourId, SlimUi::card());
    setColour (ListBox::outlineColourId, SlimUi::outline());

    setColour (BubbleComponent::backgroundColourId, SlimUi::cardRaised());
    setColour (BubbleComponent::outlineColourId, SlimUi::outline());
    setColour (TooltipWindow::textColourId, SlimUi::text());
    setColour (TooltipWindow::backgroundColourId, SlimUi::card());

    setColour (PopupMenu::backgroundColourId, SlimUi::cardRaised());
    setColour (PopupMenu::highlightedBackgroundColourId, SlimUi::accentPurple());

    setColour (SidePanel::backgroundColour, SlimUi::sidebar());


    
    //setColour (SonoDrawableButton::overOverlayColourId, Colour::fromFloatRGBA(0.8, 0.8, 0.8, 0.08));
    //setColour (SonoDrawableButton::downOverlayColourId, Colour::fromFloatRGBA(0.8, 0.8, 0.8, 0.3));
    setColour (SonoDrawableButton::overOverlayColourId, Colour::fromFloatRGBA(0.8, 0.8, 0.8, 0.08));
    setColour (SonoDrawableButton::downOverlayColourId, Colour::fromFloatRGBA(0.8, 0.8, 0.8, 0.3));
    
    setColour (DrawableButton::textColourId, Colour (0xffb9b9b9));
    setColour (DrawableButton::textColourOnId, Colour (0xffe9e9e9));

    //setColour (DrawableButton::backgroundColourId, Colour (0xffb9b9b9));
    setColour (DrawableButton::backgroundOnColourId, SlimUi::accentPurple());

    setColour(ToggleButton::tickColourId, SlimUi::accentBlue());
    
    setColour (DirectoryContentsDisplayComponent::highlightColourId, SlimUi::accentBlue().withAlpha (0.85f));
    setColour (DirectoryContentsDisplayComponent::textColourId, Colour (0xffe9e9e9));
    // setColour (Label::textColourId, Colour (0xffe9e9e9));

    //myFont = Typeface::createSystemTypefaceFor (BinaryData::DejaVuSans_ttf, BinaryData::DejaVuSans_ttfSize);
    //setDefaultSansSerifTypefaceName("Gill Sans");
    //setDefaultSansSerifTypefaceName("Arial Unicode MS");
    //setDefaultSansSerifTypefaceName(myFont.getTypefaceName());

    //myFont = Typeface::createSystemTypefaceFor (BinaryData::GillSans_ttc, BinaryData::GillSans_ttcSize);
    myFont = SlimUi::displayRegular (16 * fontScale);
    
    setupDefaultMeterColours();
    setColour (foleys::LevelMeter::lmMeterGradientLowColour, SlimUi::meterLow());
    setColour (foleys::LevelMeter::lmMeterGradientMidColour, SlimUi::meterMid());
    setColour (foleys::LevelMeter::lmMeterGradientMaxColour, SlimUi::meterHigh());
    setColour (foleys::LevelMeter::lmMeterBackgroundColour, SlimUi::sidebar());

    if (auto * deflnf = dynamic_cast<SonoLookAndFeel*>(&LookAndFeel::getDefaultLookAndFeel())) {
        setLanguageCode(deflnf->languageCode, mUseUniversalFont);
    }

    //DBG("Myfont name " << myFont.getTypefaceName());
}

void SonoLookAndFeel::setLanguageCode(const String & lang, bool useUniversalFont)
{
    languageCode = lang;
    mUseUniversalFont = useUniversalFont;

    /*
    if (lang.startsWith("zh")) {
        fontScale = 1.0f;
    }
    else if (lang.startsWith("ko")) {
        fontScale = 1.15f;
    }
     */
}



Typeface::Ptr SonoLookAndFeel::getTypefaceForFont (const Font& font)
{
#if JUCE_MAC
    const auto name = font.getTypefaceName();
    if (name == Font::getDefaultSansSerifFontName()
        || name == ".AppleSystemUIFont"
        || name == "SF Pro Display")
    {
        Font sys (font);
        sys.setTypefaceName (".AppleSystemUIFont");
        const bool wantBold = font.isBold()
                              || font.getTypefaceStyle().containsIgnoreCase ("Bold")
                              || font.getTypefaceStyle().containsIgnoreCase ("Heavy");
        sys.setTypefaceStyle (wantBold ? "Bold" : "Regular");
        return Typeface::createSystemTypefaceFor (sys);
    }
#endif

    DBG("get typeface for font " << font.getTypefaceName() << " with defaultsansser: " << Font::getDefaultSansSerifFontName());
    if (font.getTypefaceName() == Font::getDefaultSansSerifFontName())
    {
        // if on android and language is japanese/chinese/korean, use DroidSansFallback
        String lang = SystemStats::getUserLanguage();
        if (languageCode.isNotEmpty()) {
            lang = languageCode;
        }
        
        String slang = lang.initialSectionNotContaining("_").toLowerCase();

        if (!mUseUniversalFont)
        {
            if (slang.startsWith("ja")) {
                DBG("Using japanese");
                Font jfont(font);
#if (JUCE_MAC || JUCE_IOS)
                jfont.setTypefaceName("Hiragino Sans W3");
#elif JUCE_ANDROID
                //jfont.setTypefaceName("Droid Sans Fallback");
                return Typeface::createSystemTypefaceFor (BinaryData::DejaVuSans_ttf, BinaryData::DejaVuSans_ttfSize);
#elif JUCE_WINDOWS
                jfont.setTypefaceName("Arial Unicode MS");
#endif
                return Typeface::createSystemTypefaceFor (jfont);
            }
            else if (slang.startsWith("ko")) {
                DBG("Using korean");
                Font jfont(font);
#if (JUCE_MAC || JUCE_IOS)
                jfont.setTypefaceName("Apple SD Gothic Neo");
#elif JUCE_ANDROID
                //jfont.setTypefaceName("Droid Sans Fallback");
                return Typeface::createSystemTypefaceFor (BinaryData::DejaVuSans_ttf, BinaryData::DejaVuSans_ttfSize);
#elif JUCE_WINDOWS
                jfont.setTypefaceName("Malgun Gothic");
                //jfont.setTypefaceName("Arial Unicode MS");
#endif
                return Typeface::createSystemTypefaceFor (jfont);
            }
            else if (slang.startsWith("zh")) {
                DBG("Using chinese");
                Font jfont(font);
#if (JUCE_MAC || JUCE_IOS)
                jfont.setTypefaceName("PingFang SC");
#elif JUCE_WINDOWS
                jfont.setTypefaceName("Arial Unicode MS");
#elif JUCE_ANDROID
                jfont.setTypefaceName("DroidSansFallback");
                return Typeface::createSystemTypefaceFor (BinaryData::DejaVuSans_ttf, BinaryData::DejaVuSans_ttfSize);
#endif
                return Typeface::createSystemTypefaceFor (jfont);
            }
            else
            {
                DBG("Creating custom typeface!!");

                return Typeface::createSystemTypefaceFor (BinaryData::DejaVuSans_ttf, BinaryData::DejaVuSans_ttfSize);
                //return Typeface::createSystemTypefaceFor (BinaryData::InterUnicode_ttf, BinaryData::InterUnicode_ttfSize);
                //return Typeface::createSystemTypefaceFor (BinaryData::GoNotoKurrentRegular_ttf, BinaryData::GoNotoKurrentRegular_ttfSize);
            }
        }
        else {
            // universal font stuff
            {
                DBG("Creating custom typeface!!");

                //return Typeface::createSystemTypefaceFor (BinaryData::DejaVuSans_ttf, BinaryData::DejaVuSans_ttfSize);
                //return Typeface::createSystemTypefaceFor (BinaryData::InterUnicode_ttf, BinaryData::InterUnicode_ttfSize);
                return Typeface::createSystemTypefaceFor (BinaryData::DejaVuSans_ttf, BinaryData::DejaVuSans_ttfSize);
            }
        }
    }
    return LookAndFeel_V4::getTypefaceForFont(font);
}

#if 1


//==============================================================================
void SonoLookAndFeel::drawTooltip (Graphics& g, const String& text, int width, int height)
{
    Rectangle<int> bounds (width, height);
    auto cornerSize = 5.0f;

    g.setColour (findColour (TooltipWindow::backgroundColourId));
    g.fillRoundedRectangle (bounds.toFloat(), cornerSize);

    g.setColour (findColour (TooltipWindow::outlineColourId));
    g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

    sonoLayoutTooltipText (text, findColour (TooltipWindow::textColourId))
        .draw (g, { static_cast<float> (width), static_cast<float> (height) });
}

Font SonoLookAndFeel::getMenuBarFont (MenuBarComponent& menuBar, int /*itemIndex*/, const String& /*menuName*/)
{
    return SlimUi::displayRegular (menuBar.getHeight() * 0.7f * fontScale);
}

//==============================================================================
void SonoLookAndFeel::drawCallOutBoxBackground (CallOutBox& box, Graphics& g,
                                               const Path& path, Image& cachedImage)
{
    if (cachedImage.isNull())
    {
        cachedImage = Image (Image::ARGB, box.getWidth(), box.getHeight(), true);
        Graphics g2 (cachedImage);
        
        DropShadow (Colours::black.withAlpha (0.7f), 8, Point<int> (0, 2)).drawForPath (g2, path);
    }
    
    g.setColour (Colours::black);
    g.drawImageAt (cachedImage, 0, 0);
    
    //g.setColour (getCurrentColourScheme().getUIColour (ColourScheme::UIColour::widgetBackground).withAlpha (0.8f));
    g.setColour (getCurrentColourScheme().getUIColour (ColourScheme::UIColour::widgetBackground));
    g.fillPath (path);
    
    g.setColour (getCurrentColourScheme().getUIColour (ColourScheme::UIColour::outline).withAlpha (0.8f));
    g.strokePath (path, PathStrokeType (1.0f));
}



int SonoLookAndFeel::getTabButtonBestWidth (TabBarButton& button, int depth)
{
    return 250; // 120;
}

int SonoLookAndFeel::getTabButtonSpaceAroundImage() {
    return 0;
}

static Colour getTabBackgroundColour (TabBarButton& button)
{
    
    const Colour bkg (button.findColour (TabbedComponent::backgroundColourId).contrasting (0.15f));

    if (button.isFrontTab())
        return bkg.overlaidWith (Colours::yellow.withAlpha (0.8f));

    return bkg;
}

Rectangle<int> SonoLookAndFeel::getTabButtonExtraComponentBounds (const TabBarButton& button, Rectangle<int>& textArea, Component& comp)
{
    Rectangle<int> extraComp;
    
    auto orientation = button.getTabbedButtonBar().getOrientation();
    
    if (button.getExtraComponentPlacement() == TabBarButton::beforeText)
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtBottom:
            case TabbedButtonBar::TabsAtTop:     extraComp = textArea.removeFromLeft   (comp.getWidth()); break;
            case TabbedButtonBar::TabsAtLeft:    extraComp = textArea.removeFromBottom (comp.getHeight()); break;
            case TabbedButtonBar::TabsAtRight:   extraComp = textArea.removeFromTop    (comp.getHeight()); break;
            default:                             jassertfalse; break;
        }
    }
    else if (button.getExtraComponentPlacement() == TabBarButton::afterText)
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtBottom:
            case TabbedButtonBar::TabsAtTop:     extraComp = textArea.removeFromRight  (comp.getWidth()); break;
            case TabbedButtonBar::TabsAtLeft:    extraComp = textArea.removeFromTop    (comp.getHeight()); break;
            case TabbedButtonBar::TabsAtRight:   extraComp = textArea.removeFromBottom (comp.getHeight()); break;
            default:                             jassertfalse; break;
        }
    }
    else if (button.getExtraComponentPlacement() == TabBarButton::aboveText)
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtBottom:
            case TabbedButtonBar::TabsAtTop:     extraComp = textArea.removeFromTop  (comp.getHeight()); break;
            case TabbedButtonBar::TabsAtLeft:    extraComp = textArea.removeFromTop    (comp.getHeight()); break;
            case TabbedButtonBar::TabsAtRight:   extraComp = textArea.removeFromTop (comp.getHeight()); break;
            default:                             jassertfalse; break;
        }

        // DBG("Extra comp bounds: " << extraComp.toString())
        extraComp.translate(0, 3);
        //DBG("After Extra comp bounds: " << extraComp.toString())

    }
    else if (button.getExtraComponentPlacement() == TabBarButton::belowText)
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtBottom:
            case TabbedButtonBar::TabsAtTop:     extraComp = textArea.removeFromBottom  (comp.getHeight()); break;
            case TabbedButtonBar::TabsAtLeft:    extraComp = textArea.removeFromBottom    (comp.getHeight()); break;
            case TabbedButtonBar::TabsAtRight:   extraComp = textArea.removeFromBottom (comp.getHeight()); break;
            default:                             jassertfalse; break;
        }
    }
    
    return extraComp;
}

void SonoLookAndFeel::createTabTextLayout (const TabBarButton& button, float length, float depth,
                                          Colour colour, TextLayout& textLayout)
{
    float fontsize = button.getExtraComponent() != nullptr ? jmin(depth, 32.0f) * 0.5f : jmin(depth, 32.0f) * 0.5f;
    Font font = SlimUi::displayRegular (fontsize * fontScale);
    font.setUnderline (button.hasKeyboardFocus (false));

    AttributedString s;
    s.setWordWrap(AttributedString::byWord);
    s.setJustification (Justification::centred);
    s.append (button.getButtonText().trim(), font, colour);
    
    textLayout.createLayout (s, length);
}

void SonoLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const Rectangle<int> activeArea (button.getActiveArea());
    
    const TabbedButtonBar::Orientation o = button.getTabbedButtonBar().getOrientation();
    
    const Colour bkg (button.getTabBackgroundColour());
    const Colour selcol = SlimUi::accentPurple();

    // DBG("Sono draw tab button");
    
    
    if (button.getToggleState() && bkg != Colours::black)
    {
        //g.setColour (bkg);
        g.setColour (selcol);
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

        g.setColour(isMouseDown ? bkg.brighter(0.1) : bkg);

        //g.setGradientFill (ColourGradient (bkg.darker (0.1f), (float) p1.x, (float) p1.y,
        //                                   bkg.darker (0.5f),   (float) p2.x, (float) p2.y, false));
    }

    Rectangle<int> p (activeArea.reduced(1));

    g.fillRect (p);

    //g.fillRect (activeArea);
   
#if 0
    g.setColour (button.findColour (TabbedButtonBar::tabOutlineColourId));
    
    Rectangle<int> r (activeArea);
    
    if (o != TabbedButtonBar::TabsAtBottom)   g.fillRect (r.removeFromTop (1));
    if (o != TabbedButtonBar::TabsAtTop)      g.fillRect (r.removeFromBottom (1));
    if (o != TabbedButtonBar::TabsAtRight)    g.fillRect (r.removeFromLeft (1));
    if (o != TabbedButtonBar::TabsAtLeft)     g.fillRect (r.removeFromRight (1));
#endif


    const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
    
    Colour col (bkg.contrasting().withMultipliedAlpha (alpha));
    
    if (TabbedButtonBar* bar = button.findParentComponentOfClass<TabbedButtonBar>())
    {
        TabbedButtonBar::ColourIds colID = button.isFrontTab() ? TabbedButtonBar::frontTextColourId
        : TabbedButtonBar::tabTextColourId;
        
        if (button.isColourSpecified (colID))
            col = button.findColour (colID);
        else if (bar->isColourSpecified (colID))
            col = bar->findColour (colID);
        else if (isColourSpecified (colID))
            col = findColour (colID);
    }
    
    const Rectangle<float> area (button.getTextArea().reduced(1).toFloat());
    
    float length = area.getWidth();
    float depth  = area.getHeight();
    
    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);
    
    TextLayout textLayout;
    createTabTextLayout (button, length, depth, col, textLayout);
    
    AffineTransform t;
    
    switch (o)
    {
        case TabbedButtonBar::TabsAtLeft:   t = t.rotated (MathConstants<float>::pi * -0.5f).translated (area.getX(), area.getBottom()); break;
        case TabbedButtonBar::TabsAtRight:  t = t.rotated (MathConstants<float>::pi *  0.5f).translated (area.getRight(), area.getY()); break;
        case TabbedButtonBar::TabsAtTop:
        case TabbedButtonBar::TabsAtBottom: t = t.translated (area.getX(), area.getY()); break;
        default:                            jassertfalse; break;
    }
    
    g.addTransform (t);
    textLayout.draw (g, Rectangle<float> (length, depth));

}

/*
void SonoLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const Rectangle<int> activeArea (button.getActiveArea());

    const Colour bkg (getTabBackgroundColour (button));

    g.setGradientFill (ColourGradient (bkg.brighter (0.1f), 0, (float) activeArea.getY(),
                                       bkg.darker (0.1f), 0, (float) activeArea.getBottom(), false));
    g.fillRect (activeArea);

    g.setColour (button.findColour (TabbedComponent::backgroundColourId).darker (0.3f));
    g.drawRect (activeArea);

    const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
    const Colour col (bkg.contrasting().withMultipliedAlpha (alpha));

    TextLayout textLayout;
    LookAndFeel_V3::createTabTextLayout (button, (float) activeArea.getWidth(), (float) activeArea.getHeight(), col, textLayout);

    textLayout.draw (g, button.getTextArea().toFloat());
}
*/

void SonoLookAndFeel::drawTabbedButtonBarBackground (TabbedButtonBar&, Graphics&) {}

void SonoLookAndFeel::drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, const int w, const int h)
{
    const float shadowSize = 0.15f;
    
    Rectangle<int> shadowRect, line;
    ColourGradient gradient (Colours::black.withAlpha (bar.isEnabled() ? 0.08f : 0.04f), 0, 0,
                             Colours::transparentBlack, 0, 0, false);
    
    switch (bar.getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:
            gradient.point1.x = (float) w;
            gradient.point2.x = w * (1.0f - shadowSize);
            shadowRect.setBounds ((int) gradient.point2.x, 0, w - (int) gradient.point2.x, h);
            line.setBounds (w - 1, 0, 1, h);
            break;
            
        case TabbedButtonBar::TabsAtRight:
            gradient.point2.x = w * shadowSize;
            shadowRect.setBounds (0, 0, (int) gradient.point2.x, h);
            line.setBounds (0, 0, 1, h);
            break;
            
        case TabbedButtonBar::TabsAtTop:
            gradient.point1.y = (float) h;
            gradient.point2.y = h * (1.0f - shadowSize);
            shadowRect.setBounds (0, (int) gradient.point2.y, w, h - (int) gradient.point2.y);
            line.setBounds (0, h - 1, w, 1);
            break;
            
        case TabbedButtonBar::TabsAtBottom:
            gradient.point2.y = h * shadowSize;
            shadowRect.setBounds (0, 0, w, (int) gradient.point2.y);
            line.setBounds (0, 0, w, 1);
            break;
            
        default: break;
    }
    
    g.setGradientFill (gradient);
    g.fillRect (shadowRect.expanded (2, 2));
    
    g.setColour (bar.findColour (TabbedButtonBar::tabOutlineColourId));
    g.fillRect (line);
}

void SonoLookAndFeel::drawTabButtonText (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const Rectangle<float> area (button.getTextArea().toFloat());
    
    //DBG("Sono look and feel drawtabbutton text: " << button.getButtonText());
    
    float length = area.getWidth();
    float depth  = area.getHeight();
    
    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);
    
    Font font  = SlimUi::displayRegular (jmin(depth,30.0f) * 0.6f * fontScale);
    font.setUnderline (button.hasKeyboardFocus (false));
    
    AffineTransform t;
    
    switch (button.getTabbedButtonBar().getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:   t = t.rotated (MathConstants<float>::pi * -0.5f).translated (area.getX(), area.getBottom()); break;
        case TabbedButtonBar::TabsAtRight:  t = t.rotated (MathConstants<float>::pi *  0.5f).translated (area.getRight(), area.getY()); break;
        case TabbedButtonBar::TabsAtTop:
        case TabbedButtonBar::TabsAtBottom: t = t.translated (area.getX(), area.getY()); break;
        default:                            jassertfalse; break;
    }
    
    Colour col;
    
    if (button.isFrontTab() && (button.isColourSpecified (TabbedButtonBar::frontTextColourId)
                                || isColourSpecified (TabbedButtonBar::frontTextColourId)))
        col = findColour (TabbedButtonBar::frontTextColourId);
    else if (button.isColourSpecified (TabbedButtonBar::tabTextColourId)
             || isColourSpecified (TabbedButtonBar::tabTextColourId))
        col = findColour (TabbedButtonBar::tabTextColourId);
    else
        col = button.getTabBackgroundColour().contrasting();
    
    const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
    
    g.setColour (col.withMultipliedAlpha (alpha));
    g.setFont (font);
    g.addTransform (t);
    
    g.drawFittedText (button.getButtonText().trim(),
                      0, 0, (int) length, (int) depth,
                      Justification::centred,
                      //jmax (1, ((int) depth) / 12), 0.5f);
                      1, 0.5f);
}

static Range<float> getBrightnessRange (const Image& im)
{
    float minB = 1.0f, maxB = 0;
    const int w = im.getWidth();
    const int h = im.getHeight();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const float b = im.getPixelAt (x, y).getBrightness();
            minB = jmin (minB, b);
            maxB = jmax (maxB, b);
        }
    }

    return Range<float> (minB, maxB);
}

Font SonoLookAndFeel::getLabelFont (Label& label)
{
    auto font = (fontScale == 1.0f) ? label.getFont()
                                    : label.getFont().withHeight (label.getFont().getHeight() * fontScale);
    return SlimUi::applyDisplayStyle (font);
}

void SonoLookAndFeel::drawLabel (Graphics& g, Label& label)
{
    Colour olcolor = label.findColour (Label::backgroundColourId);

    g.setColour(olcolor);
    if (!olcolor.isTransparent()) {
        if (labelCornerRadius > 0.0f) {
            g.fillRoundedRectangle(label.getLocalBounds().reduced(1).toFloat(), labelCornerRadius);
        } else {
            g.fillAll (olcolor);
        }
    }
    
    
    if (! label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font (getLabelFont (label));
        
        g.setColour (label.findColour (Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);
        
        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());
        
        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          jmax (1, (int) (textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());
        
        olcolor = label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha);
    }
    else if (label.isEnabled())
    {
        olcolor = label.findColour (Label::outlineColourId);
    }
    
    if (!olcolor.isTransparent()) {
        g.setColour (olcolor);
        if (labelCornerRadius > 0.0f) {
            g.drawRoundedRectangle(label.getLocalBounds().reduced(1).toFloat(), labelCornerRadius, 1.0f);
        } else {
            g.drawRect (label.getLocalBounds());        
        }
    }
}

Font SonoLookAndFeel::getTextButtonFont (TextButton& button, int buttonHeight)
{
    // DBG("GetTextButton font with height: " << buttonHeight);
    float textRatio = 0.5f;
    if (SonoTextButton* const textbutt = dynamic_cast<SonoTextButton*> (&button)) {
        textRatio = textbutt->getTextHeightRatio();
    }
    
    return SlimUi::displayRegular (jmin (16.0f, buttonHeight * textRatio) * fontScale);
}

Button* SonoLookAndFeel::createSliderButton (Slider&, const bool isIncrement)
{
    TextButton * butt = new TextButton (isIncrement ? "+" : "-", {});
    return butt;
}

Label* SonoLookAndFeel::createSliderTextBox (Slider& slider)
{
    Label * lab = LookAndFeel_V4::createSliderTextBox(slider);
    lab->setKeyboardType(TextInputTarget::decimalKeyboard);
    lab->setFont(SlimUi::displayRegular (14.0f));
    lab->setMinimumHorizontalScale(0.5);
    lab->setJustificationType(Justification::centredRight);
    return lab;
}

Font SonoLookAndFeel::getSliderPopupFont (Slider&)
{
    return SlimUi::displayBold (18.0f);
}

int SonoLookAndFeel::getSliderPopupPlacement (Slider&)
{
    return BubbleComponent::above
    //| BubbleComponent::below
    | BubbleComponent::left
    | BubbleComponent::right
    ;
}

void SonoLookAndFeel::drawButtonTextWithAlignment (Graphics& g, TextButton& button, bool /*isMouseOverButton*/, bool /*isButtonDown*/, Justification textjust)
{
    Font font (getTextButtonFont (button, button.getHeight()));
    g.setFont (font);
    g.setColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
                                    : TextButton::textColourOffId)
                 .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));
    
    float textRatio = 0.7f;
    if (SonoTextButton* const textbutt = dynamic_cast<SonoTextButton*> (&button)) {
        textRatio = textbutt->getTextHeightRatio();
    }
    
    const int yIndent = jmin (2, button.proportionOfHeight ((1.0 - textRatio) * 0.5));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;
    
    const int fontHeight = roundToInt (font.getHeight() * 0.3);
    const int leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    
    g.drawFittedText (button.getButtonText(),
                      leftIndent,
                      yIndent,
                      button.getWidth() - leftIndent - rightIndent,
                      button.getHeight() - yIndent * 2,
                      textjust, 2, 0.7f);
}


void SonoLookAndFeel::drawButtonText (Graphics& g, TextButton& button, bool isMouseOverButton, bool isButtonDown)
{
    drawButtonTextWithAlignment(g, button, isMouseOverButton, isButtonDown);
}


void SonoLookAndFeel::drawFileBrowserRow (Graphics& g, int width, int height,
                                         const File& file, const String& filename, Image* icon,
                                         const String& fileSizeDescription,
                                         const String& fileTimeDescription,
                                         bool isDirectory, bool isItemSelected,
                                         int itemIndex, DirectoryContentsDisplayComponent& dcc)
{
    Component* const fileListComp = dynamic_cast<Component*> (&dcc);

    if (isItemSelected)
        g.fillAll (fileListComp != nullptr ? fileListComp->findColour (DirectoryContentsDisplayComponent::highlightColourId)
                   : findColour (DirectoryContentsDisplayComponent::highlightColourId));

    int x = 32;
    g.setColour (Colours::black);

    if (isDirectory) {
        if (icon != nullptr && icon->isValid())
        {
            g.drawImageWithin (*icon, 2, 2, x - 4, height - 4,
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                               false);
        }
        else
        {
            if (const Drawable* d = isDirectory ? getDefaultFolderImage()
                : getDefaultDocumentFileImage())
                d->drawWithin (g, Rectangle<float> (2.0f, 2.0f, x - 4.0f, height - 4.0f),
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
        }
    }
    else {
        x = 4;
    }

    g.setColour (fileListComp != nullptr ? fileListComp->findColour (DirectoryContentsDisplayComponent::textColourId)
                 : findColour (DirectoryContentsDisplayComponent::textColourId));
    g.setFont (myFont.withHeight(height * 0.5f));

    if (width > 450 && ! isDirectory)
    {
        const int sizeX = roundToInt (width * 0.7f);
        const int dateX = roundToInt (width * 0.8f);

        g.drawFittedText (filename,
                          x, 0, sizeX - x, height,
                          Justification::centredLeft, 1);

        g.setFont (myFont.withHeight(height * 0.5f));
        g.setColour (Colours::darkgrey);

        if (! isDirectory)
        {
            g.drawFittedText (fileSizeDescription,
                              sizeX, 0, dateX - sizeX - 8, height,
                              Justification::centredRight, 1);

            g.drawFittedText (fileTimeDescription,
                              dateX, 0, width - 8 - dateX, height,
                              Justification::centredRight, 1);
        }
    }
    else
    {
        g.drawFittedText (filename,
                          x, 0, width - x, height,
                          Justification::centredLeft, 1);

    }
}

Button* SonoLookAndFeel::createFileBrowserGoUpButton()
{
    DrawableButton* goUpButton = new DrawableButton ("up", DrawableButton::ImageOnButtonBackground);

    Path arrowPath;
    arrowPath.addArrow (Line<float> (50.0f, 100.0f, 50.0f, 0.0f), 40.0f, 100.0f, 50.0f);

    DrawablePath arrowImage;
    arrowImage.setFill (Colours::white.withAlpha (0.4f));
    arrowImage.setPath (arrowPath);

    goUpButton->setImages (&arrowImage);

    return goUpButton;
}

void SonoLookAndFeel::layoutFileBrowserComponent (FileBrowserComponent& browserComp,
                                                 DirectoryContentsDisplayComponent* fileListComponent,
                                                 FilePreviewComponent* previewComp,
                                                 ComboBox* currentPathBox,
                                                 TextEditor* filenameBox,
                                                 Button* goUpButton)
{
    const int x = 8;
    int w = browserComp.getWidth() - x - x;

    if (previewComp != nullptr)
    {
        const int previewWidth = w / 3;
        previewComp->setBounds (x + w - previewWidth, 0, previewWidth, browserComp.getHeight());

        w -= previewWidth + 4;
    }

    int y = 4;

    const int controlsHeight = 22;
    const int bottomSectionHeight = controlsHeight + 8;
    const int upButtonWidth = 50;

    currentPathBox->setBounds (x, y, w - upButtonWidth - 6, controlsHeight);
    goUpButton->setBounds (x + w - upButtonWidth, y, upButtonWidth, controlsHeight);

    y += controlsHeight + 4;

    if (Component* const listAsComp = dynamic_cast <Component*> (fileListComponent))
    {
        listAsComp->setBounds (x, y, w, browserComp.getHeight() - y - bottomSectionHeight);
        y = listAsComp->getBottom() + 4;
    }

    filenameBox->setBounds (x + 50, y, w - 50, controlsHeight);
}

Font SonoLookAndFeel::getComboBoxFont (ComboBox& box)
{
    return SlimUi::displayRegular (jmin (16.0f, (float) box.getHeight() * 0.9f));
}

Font SonoLookAndFeel::getPopupMenuFont()
{
    return SlimUi::displayRegular (17.0f * fontScale);
}

void SonoLookAndFeel::drawPopupMenuBackground (Graphics& g, [[maybe_unused]] int width, [[maybe_unused]] int height)
{
    g.fillAll (findColour (PopupMenu::backgroundColourId));

   #if ! JUCE_MAC
    g.setColour (findColour (PopupMenu::textColourId).brighter(0.1f).withAlpha(0.1f));
    g.drawRect (0, 0, width, height);
   #endif
}

PopupMenu::Options SonoLookAndFeel::getOptionsForComboBoxPopupMenu (ComboBox& box, Label& label)
{
    auto options = PopupMenu::Options().withTargetComponent (&box)
                               .withItemThatMustBeVisible (box.getSelectedId())
                               .withMinimumWidth (box.getWidth())
                               .withMaximumNumColumns (1)
                               .withStandardItemHeight (label.getHeight());

#if JUCE_IOS || JUCE_ANDROID
    auto * dw = box.findParentComponentOfClass<AudioProcessorEditor>();
    if (dw) {
        options = options.withParentComponent(dw);
    }
#endif

    return options;
}

void SonoLookAndFeel::drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<float>& area,
                                               Colour backgroundColour, bool isOpen, bool isMouseOver)
{
    Path p;
    p.addTriangle (0.0f, 0.0f, 1.0f, isOpen ? 0.0f : 0.5f, isOpen ? 0.5f : 0.0f, 1.0f);

    DBG("draw plus minus ours");

    //g.setColour (backgroundColour.contrasting().withAlpha (isMouseOver ? 0.5f : 0.3f));
    g.setColour (Colours::white.withAlpha (isMouseOver ? 0.5f : 0.3f));
    g.fillPath (p, p.getTransformToScaleToFit (area.reduced (2, area.getHeight() / 4), true));
}


void SonoLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& button,
                                       bool isMouseOverButton, bool isButtonDown)
{
    /*
    if (button.hasKeyboardFocus (true))
    {
        g.setColour (button.findColour (TextEditor::focusedOutlineColourId));
        g.drawRect (0, 0, button.getWidth(), button.getHeight());
    }
     */

    float fontSize = jmin (15.0f, button.getHeight() * 0.75f) * fontScale;
    const float tickWidth = fontSize * 1.1f;

    drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
                 tickWidth, tickWidth,
                 button.getToggleState(),
                 button.isEnabled(),
                 isMouseOverButton,
                 isButtonDown);

    g.setColour (button.findColour (ToggleButton::textColourId));
    g.setFont (myFont.withHeight(fontSize));

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    const int textX = (int) tickWidth + 10;

    g.drawFittedText (button.getButtonText(),
                      textX, 0,
                      button.getWidth() - textX - 2, button.getHeight(),
                      Justification::centredLeft, 10);
}

void SonoLookAndFeel::drawTickBox (Graphics& g, Component& component,
                                  float x, float y, float w, float h,
                                  const bool ticked,
                                  const bool isEnabled,
                                  const bool isMouseOverButton,
                                  const bool isButtonDown)
{
    const float boxSize = w * 1.0f;


    g.setColour (component.findColour (TextEditor::focusedOutlineColourId));
    g.drawRect (x, y + (h - boxSize) * 0.5f, boxSize, boxSize);

    if (ticked)
    {
        Path tick;
        tick.startNewSubPath (1.5f, 3.0f);
        tick.lineTo (3.0f, 6.0f);
        tick.lineTo (6.0f, 0.0f);

        
        g.setColour (isEnabled ? component.findColour(ToggleButton::tickColourId) : Colours::grey);

        const AffineTransform trans (AffineTransform::scale (w / 9.0f, h / 9.0f)
                                     .translated (x+2, y+1));

        g.strokePath (tick, PathStrokeType (2.5f), trans);
    }
}

void SonoLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                       const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
    const auto outline = findColour (Slider::rotarySliderOutlineColourId);
    const auto fill    = findColour (Slider::rotarySliderFillColourId);
    
    const auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced (3);
    
    
    auto radius = jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin (8.0f, radius * 0.3f);
    auto arcRadius = radius - lineW * 0.5f;
    
    Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(),
                                 bounds.getCentreY(),
                                 arcRadius,
                                 arcRadius,
                                 0.0f,
                                 rotaryStartAngle,
                                 rotaryEndAngle,
                                 true);
    
    g.setColour (outline);
    g.strokePath (backgroundArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    
    auto rotStartAngle = rotaryStartAngle;
    
    if (slider.getProperties().contains ("fromCentre"))
    {
        rotStartAngle = (rotStartAngle + rotaryEndAngle) / 2;
    }

    
    if (slider.isEnabled())
    {
        Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(),
                                bounds.getCentreY(),
                                arcRadius,
                                arcRadius,
                                0.0f,
                                rotStartAngle,
                                toAngle,
                                true);
        
        g.setColour (fill);
        g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }
    
    const auto thumbWidth = lineW ; // * 1.5f;
    const Point<float> thumbPoint (bounds.getCentreX() + arcRadius * std::cos (toAngle - MathConstants<float>::pi * 0.5f),
                                   bounds.getCentreY() + arcRadius * std::sin (toAngle - MathConstants<float>::pi * 0.5f));
    
    g.setColour (findColour (Slider::thumbColourId));
    g.fillEllipse (Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
}

//==============================================================================
int SonoLookAndFeel::getSliderThumbRadius (Slider& slider)
{
    if (slider.isTwoValue() || slider.isThreeValue()) {
        return jmin (14, slider.isHorizontal() ? static_cast<int> (slider.getHeight() * 0.25f)
                                               : static_cast<int> (slider.getWidth()  * 0.5f));        
    }
    return jmin (16, slider.isHorizontal() ? static_cast<int> (slider.getHeight() * 0.5f)
                                           : static_cast<int> (slider.getWidth()  * 0.5f));
}


Slider::SliderLayout SonoLookAndFeel::getSliderLayout (Slider& slider)
{
    // 1. compute the actually visible textBox size from the slider textBox size and some additional constraints

    int minXSpace = 0;
    int minYSpace = 0;

    auto textBoxPos = slider.getTextBoxPosition();

    if (textBoxPos == Slider::TextBoxLeft || textBoxPos == Slider::TextBoxRight)
        minXSpace = 30;
    else
        minYSpace = 15;

    auto localBounds = slider.getLocalBounds();

    auto textBoxWidth  = jmax (0, jmin (slider.getTextBoxWidth(),  localBounds.getWidth() - minXSpace));
    auto textBoxHeight = jmax (0, jmin (slider.getTextBoxHeight(), localBounds.getHeight() - minYSpace));

    Slider::SliderLayout layout;

    // 2. set the textBox bounds

    if (textBoxPos != Slider::NoTextBox)
    {
        if (slider.isBar())
        {
            layout.textBoxBounds = localBounds;
        }
        else
        {
            layout.textBoxBounds.setWidth (textBoxWidth);
            layout.textBoxBounds.setHeight (textBoxHeight);

            const int thumbIndent = getSliderThumbRadius (slider);

            if (textBoxPos == Slider::TextBoxLeft)           layout.textBoxBounds.setX (0);
            else if (textBoxPos == Slider::TextBoxRight)     layout.textBoxBounds.setX (localBounds.getWidth() - textBoxWidth);
            else if (sliderTextJustification.testFlags(Justification::right))/* above or below -> right */ layout.textBoxBounds.setX ((localBounds.getWidth() - textBoxWidth - 1));
            else if (sliderTextJustification.testFlags(Justification::left))/* above or below -> left */ layout.textBoxBounds.setX (1);
            else /* above or below -> centre horizontally */ layout.textBoxBounds.setX ((localBounds.getWidth() - textBoxWidth) / 2);

            if (textBoxPos == Slider::TextBoxAbove)          layout.textBoxBounds.setY (0);
            else if (textBoxPos == Slider::TextBoxBelow)     layout.textBoxBounds.setY (localBounds.getHeight() - textBoxHeight);
            else if (sliderTextJustification.testFlags(Justification::top))/* left or right -> top */ layout.textBoxBounds.setY (0);
            else if (sliderTextJustification.testFlags(Justification::bottom))/* left or right -> bottom */ layout.textBoxBounds.setY (localBounds.getHeight() - textBoxHeight);
            else /* left or right -> centre vertically */    layout.textBoxBounds.setY ((localBounds.getHeight() - textBoxHeight) / 2);
        }
    }

    // 3. set the slider bounds

    layout.sliderBounds = localBounds;

    if (slider.isBar())
    {
        layout.sliderBounds.reduce (1, 1);   // bar border
    }
    else
    {
        if (textBoxPos == Slider::TextBoxLeft)       layout.sliderBounds.removeFromLeft (textBoxWidth);
        else if (textBoxPos == Slider::TextBoxRight) layout.sliderBounds.removeFromRight (textBoxWidth);
        else if (textBoxPos == Slider::TextBoxAbove) layout.sliderBounds.removeFromTop (textBoxHeight);
        else if (textBoxPos == Slider::TextBoxBelow) layout.sliderBounds.removeFromBottom (textBoxHeight);

        const int thumbIndent = getSliderThumbRadius (slider);

        if (slider.isHorizontal())    layout.sliderBounds.reduce (thumbIndent, 0);
        else if (slider.isVertical()) layout.sliderBounds.reduce (0, thumbIndent);
    }

    return layout;
}


void SonoLookAndFeel::drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                                       float sliderPos,
                                       float minSliderPos,
                                       float maxSliderPos,
                                       const Slider::SliderStyle style, Slider& slider)
{
    if (slider.isBar())
    {
        if (slider.getProperties().contains ("fromCentre")) {
            auto centrex = x + width*0.5f;
            auto centrey = y + height*0.5f;
            
            if (!slider.getProperties().contains ("noFill")) {
                g.setColour (slider.findColour (Slider::trackColourId));

                g.fillRect (slider.isHorizontal() ? Rectangle<float> (sliderPos > centrex ? centrex : sliderPos, y + 0.5f, sliderPos > centrex ? sliderPos - centrex : centrex - sliderPos, height - 1.0f)
                            : Rectangle<float> (x + 0.5f, sliderPos < centrey ? sliderPos : centrey, width - 1.0f, sliderPos < centrey ?  centrey - sliderPos : sliderPos - centrey));
            }
            
            // draw line
            g.setColour (slider.findColour (Slider::thumbColourId));

            g.fillRect (slider.isHorizontal() ? Rectangle<float> (sliderPos - 1, y + 0.5f, 2, height - 1.0f)
                        : Rectangle<float> (x + 0.5f, sliderPos - 1, width - 1.0f, 2));
        }
        else {
            
            if (!slider.getProperties().contains ("noFill")) {

                g.setColour (slider.findColour (Slider::trackColourId));

                g.fillRect (slider.isHorizontal() ? Rectangle<float> (static_cast<float> (x), y + 0.5f, sliderPos - x, height - 1.0f)
                            : Rectangle<float> (x + 0.5f, sliderPos, width - 1.0f, y + (height - sliderPos)));
            }
            //else

            g.setColour (slider.findColour (Slider::thumbColourId));

            {
                // draw line
                g.fillRect (slider.isHorizontal() ? Rectangle<float> (sliderPos - 1, y + 0.5f, 3, height - 1.0f)
                            : Rectangle<float> (x + 0.5f, sliderPos - 1, width - 1.0f, 3));
            }
        }
    }
    else
    {
        auto isTwoVal   = (style == Slider::SliderStyle::TwoValueVertical   || style == Slider::SliderStyle::TwoValueHorizontal);
        auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);
        
        auto trackWidth = jmin (10.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);
        
        Point<float> startPoint (slider.isHorizontal() ? x : x + width * 0.5f,
                                 slider.isHorizontal() ? y + height * 0.5f : height + y);
        
        Point<float> endPoint (slider.isHorizontal() ? width + x : startPoint.x,
                               slider.isHorizontal() ? startPoint.y : y);
        
        Path backgroundTrack;
        backgroundTrack.startNewSubPath (startPoint);
        backgroundTrack.lineTo (endPoint);
        g.setColour (slider.findColour (Slider::backgroundColourId));
        g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
        
        Path valueTrack;
        Point<float> minPoint, maxPoint, thumbPoint;
        
        if (isTwoVal || isThreeVal)
        {
            minPoint = { slider.isHorizontal() ? minSliderPos : width * 0.5f,
                slider.isHorizontal() ? height * 0.5f : minSliderPos };
            
            if (isThreeVal)
                thumbPoint = { slider.isHorizontal() ? sliderPos : width * 0.5f,
                    slider.isHorizontal() ? height * 0.5f : sliderPos };
            
            maxPoint = { slider.isHorizontal() ? maxSliderPos : width * 0.5f,
                slider.isHorizontal() ? height * 0.5f : maxSliderPos };
        }
        else
        {
            auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
            auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;
            
            minPoint = startPoint;
            maxPoint = { kx, ky };
        }
        
        auto thumbWidth = getSliderThumbRadius (slider);
        
        valueTrack.startNewSubPath (minPoint);
        valueTrack.lineTo (isThreeVal ? thumbPoint : maxPoint);
        g.setColour (slider.findColour (Slider::trackColourId));
        g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
        
        if (! isTwoVal)
        {
            g.setColour (slider.findColour (Slider::thumbColourId));
            g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (isThreeVal ? thumbPoint : maxPoint));
        }
        
        if (isTwoVal || isThreeVal)
        {
            auto sr = jmin (trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
            auto pointerColour = slider.findColour (Slider::thumbColourId);
            
            auto wscale = 1.5f;
            
            if (slider.isHorizontal())
            {
                /*
                drawPointer (g, minSliderPos - sr,
                             height * 0.5f - trackWidth*wscale*0.5, //jmax (0.0f, y + height * 0.5f - trackWidth * 0.5f),
                             trackWidth * wscale, pointerColour, 1); // 2
                
                drawPointer (g, maxSliderPos - trackWidth,
                             height * 0.5f - trackWidth*wscale*0.5, //jmin (y + height - trackWidth * 2.0f, (float)y + height * 0.5f),
                             trackWidth * wscale, pointerColour, 3); // 4
                */
                
                drawPointer (g, minSliderPos - sr,
                             jmax (0.0f, y + height * 0.5f - trackWidth * wscale),
                             trackWidth * wscale, pointerColour, 2); // 2
                
                drawPointer (g, maxSliderPos - trackWidth*0.5*wscale,
                             jmin (y + height - trackWidth * wscale, (float)y + height * 0.5f),
                             trackWidth * wscale, pointerColour, 4); // 4
            }
            else
            {
                drawPointer (g, jmax (0.0f, x + width * 0.5f - trackWidth * 2.0f),
                             minSliderPos - trackWidth,
                             trackWidth * wscale, pointerColour, 1);
                
                drawPointer (g, jmin (x + width - trackWidth * 2.0f, x + width * 0.5f), maxSliderPos - sr,
                             trackWidth * wscale, pointerColour, 3);
            }
        }
    }
}


void SonoLookAndFeel::drawDrawableButton (Graphics& g, DrawableButton& button,
                                         bool isMouseOverButton, bool isButtonDown)
{
    const auto cornerSize = 8.0f;
    bool toggleState = button.getToggleState();
    
    ;

    //Rectangle<float> bounds = g.getClipBounds().toFloat();
    Rectangle<float> bounds = button.getLocalBounds().toFloat();

    g.setColour(button.findColour (toggleState ? DrawableButton::backgroundOnColourId
                                   : DrawableButton::backgroundColourId));
    //g.fillAll();
    g.fillRoundedRectangle(bounds, cornerSize);
    
    if (isButtonDown) {
        g.setColour(findColour(SonoDrawableButton::downOverlayColourId));
        //g.fillAll();
        g.fillRoundedRectangle(bounds, cornerSize);
    }
    else if (isMouseOverButton) {
        g.setColour(findColour(SonoDrawableButton::overOverlayColourId));
        //g.fillAll();
        g.fillRoundedRectangle(bounds, cornerSize);
    }
    
    //g.fillAll (button.findColour (toggleState ? DrawableButton::backgroundOnColourId
    //                              : DrawableButton::backgroundColourId));
    
    int textH = 0;
    int textW = 0;
    float imageratio = 0.75f;

    if (SonoDrawableButton* const sonobutt = dynamic_cast<SonoDrawableButton*> (&button)) {
        imageratio = sonobutt->getForegroundImageRatio();
    }

    if (button.getStyle() == DrawableButton::ImageAboveTextLabel || button.getStyle() == DrawableButton::ImageBelowTextLabel) {
        textH = jmin (14, button.proportionOfHeight (0.2f));
    } else if (button.getStyle() == DrawableButton::ImageLeftOfTextLabel || button.getStyle() == DrawableButton::ImageRightOfTextLabel) {
        textH = jmin (14, button.proportionOfHeight (0.8f));
        textW = jmax (20, button.proportionOfWidth (1.0f - imageratio));
    }


    if (textH > 0)
    {
        g.setFont (myFont.withHeight((float) textH * fontScale));
        
        g.setColour (button.findColour (toggleState ? DrawableButton::textColourOnId
                                        : DrawableButton::textColourId)
                     .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.4f));

        if (button.getStyle() == DrawableButton::ImageAboveTextLabel) {

            g.drawFittedText (button.getButtonText(),
                              2, button.getHeight() - textH - 1,
                              button.getWidth() - 4, textH,
                              Justification::centred, 1);
        }
        else if (button.getStyle() == DrawableButton::ImageBelowTextLabel) {
            g.drawFittedText (button.getButtonText(),
                              2, 1,
                              button.getWidth() - 4, textH,
                              Justification::centred, 1);
        }
        else if (button.getStyle() == DrawableButton::ImageRightOfTextLabel) {
            g.drawFittedText (button.getButtonText(),
                              2, 1,
                              textW , button.getHeight() - 2,
                              Justification::centred, 2, 0.6f);
        }
        else if (button.getStyle() == DrawableButton::ImageLeftOfTextLabel) {
            g.drawFittedText (button.getButtonText(),
                              button.getWidth() - textW - 4 , 1,
                              textW , button.getHeight() - 2,
                              Justification::centred, 2, 0.6f);
        }
    }
}


void SonoLookAndFeel::drawBubble (Graphics& g, BubbleComponent& comp,
                                 const Point<float>& tip, const Rectangle<float>& body)
{
    Path p;
    p.addBubble (body.reduced (0.5f), body.getUnion (Rectangle<float> (tip.x, tip.y, 1.0f, 1.0f)),
                 tip, 5.0f, jmin (10.0f, body.getWidth() * 0.2f, body.getHeight() * 0.2f));
    
    g.setColour (comp.findColour (BubbleComponent::backgroundColourId));
    g.fillPath (p);
    
    g.setColour (comp.findColour (BubbleComponent::outlineColourId));
    g.strokePath (p, PathStrokeType (1.0f));
}


SonoBigTextLookAndFeel::SonoBigTextLookAndFeel(float maxTextSize)
: maxSize(maxTextSize)
{
    
}

Font SonoBigTextLookAndFeel::getTextButtonFont (TextButton& button, int buttonHeight)
{
    // DBG("GetTextButton font with height: " << buttonHeight << " maxsize: " << maxSize);
    float textRatio = 0.8f;
    if (SonoTextButton* const textbutt = dynamic_cast<SonoTextButton*> (&button)) {
        textRatio = textbutt->getTextHeightRatio();
    }
    
    return myFont.withHeight(jmin (maxSize, buttonHeight * textRatio) * fontScale);
}

Label* SonoBigTextLookAndFeel::createSliderTextBox (Slider& slider)
{
    Label * lab = LookAndFeel_V4::createSliderTextBox(slider);
    lab->setKeyboardType(TextInputTarget::decimalKeyboard);
    lab->setFont(myFont.withHeight(maxSize));
    lab->setJustificationType(textJustification);
    lab->setMinimumHorizontalScale(0.5);

    return lab;
}

Button* SonoBigTextLookAndFeel::createSliderButton (Slider&, const bool isIncrement)
{
    TextButton * butt = new TextButton (isIncrement ? "+" : "-", {});
    butt->setLookAndFeel(this);
    return butt;
}


void SonoBigTextLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& button,
                                       bool isMouseOverButton, bool isButtonDown)
{
    /*
    if (button.hasKeyboardFocus (true))
    {
        g.setColour (button.findColour (TextEditor::focusedOutlineColourId));
        g.drawRect (0, 0, button.getWidth(), button.getHeight());
    }
     */

    float fontSize = jmin (maxSize, button.getHeight() * 0.75f) * fontScale;
    const float tickWidth = fontSize * 1.1f;

    drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
                 tickWidth, tickWidth,
                 button.getToggleState(),
                 button.isEnabled(),
                 isMouseOverButton,
                 isButtonDown);

    g.setColour (button.findColour (ToggleButton::textColourId));
    g.setFont (myFont.withHeight(fontSize));

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    const int textX = (int) tickWidth + 10;

    g.drawFittedText (button.getButtonText(),
                      textX, 0,
                      button.getWidth() - textX - 2, button.getHeight(),
                      Justification::centredLeft, 10);
}


// pan slider look and feel

SonoPanSliderLookAndFeel::SonoPanSliderLookAndFeel(float maxTextSize)
: maxSize(maxTextSize)
{
    setColour (TooltipWindow::textColourId, Colour(0xeecccccc));
   // setColour (TooltipWindow::backgroundColourId, Colour(0xeeffff99));
    //setColour (Slider::textBoxBackgroundColourId, Colour::fromFloatRGBA(0.05, 0.05, 0.05, 1.0));
    setColour (Slider::textBoxBackgroundColourId, Colours::transparentBlack);
    setColour (Slider::textBoxHighlightColourId, Colour (0xaa555555));
    setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);

}

Label* SonoPanSliderLookAndFeel::createSliderTextBox (Slider& slider)
{
    Label * lab = LookAndFeel_V4::createSliderTextBox(slider);
    lab->setKeyboardType(TextInputTarget::decimalKeyboard);
    lab->setFont(myFont.withHeight(maxSize));
    lab->setJustificationType(textJustification);
    lab->setMinimumHorizontalScale(0.5);

    return lab;
}

Button* SonoPanSliderLookAndFeel::createSliderButton (Slider&, const bool isIncrement)
{
    TextButton * butt = new TextButton (isIncrement ? "+" : "-", {});
    butt->setLookAndFeel(this);
    return butt;
}

int SonoPanSliderLookAndFeel::getSliderThumbRadius (Slider& slider)
{
    //if (slider.isTwoValue() || slider.isThreeValue()) {
        return jmin (14, slider.isHorizontal() ? static_cast<int> (slider.getHeight() * 0.25f)
                                               : static_cast<int> (slider.getWidth()  * 0.5f));        
    //}
    //return jmin (16, slider.isHorizontal() ? static_cast<int> (slider.getHeight() * 0.5f)
    //                                       : static_cast<int> (slider.getWidth()  * 0.5f));
}

Font SonoPanSliderLookAndFeel::getSliderPopupFont (Slider&)
{
    return Font (maxSize * fontScale, Font::bold);
}

int SonoPanSliderLookAndFeel::getSliderPopupPlacement (Slider&)
{
    return BubbleComponent::above
    //| BubbleComponent::below
    | BubbleComponent::left
    | BubbleComponent::right
    ;
}


void SonoPanSliderLookAndFeel::drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                                       float sliderPos,
                                       float minSliderPos,
                                       float maxSliderPos,
                                       const Slider::SliderStyle style, Slider& slider)
{
    if (slider.isBar())
    {
        g.setColour (slider.findColour (Slider::trackColourId));
        if (slider.getProperties().contains ("fromCentre")) {
            auto centrex = x + width*0.5f;
            auto centrey = y + height*0.5f;
            
            if (!slider.getProperties().contains ("noFill")) {
                g.fillRect (slider.isHorizontal() ? Rectangle<float> (sliderPos > centrex ? centrex : sliderPos, y + 0.5f, sliderPos > centrex ? sliderPos - centrex : centrex - sliderPos, height - 1.0f)
                            : Rectangle<float> (x + 0.5f, sliderPos < centrey ? sliderPos : centrey, width - 1.0f, sliderPos < centrey ?  centrey - sliderPos : sliderPos - centrey));
            }
            
            // draw line
            g.fillRect (slider.isHorizontal() ? Rectangle<float> (sliderPos - 1, y + 0.5f, 2, height - 1.0f)
                        : Rectangle<float> (x + 0.5f, sliderPos - 1, width - 1.0f, 2));
        }
        else {
            
            if (!slider.getProperties().contains ("noFill")) {
                g.fillRect (slider.isHorizontal() ? Rectangle<float> (static_cast<float> (x), y + 0.5f, sliderPos - x, height - 1.0f)
                            : Rectangle<float> (x + 0.5f, sliderPos, width - 1.0f, y + (height - sliderPos)));
            }
            else {
                // draw line
                g.fillRect (slider.isHorizontal() ? Rectangle<float> (sliderPos - 1, y + 0.5f, 3, height - 1.0f)
                            : Rectangle<float> (x + 0.5f, sliderPos - 1, width - 1.0f, 3));
            }
        }
    }
    else
    {
        auto isTwoVal   = (style == Slider::SliderStyle::TwoValueVertical   || style == Slider::SliderStyle::TwoValueHorizontal);
        auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);
        
        auto trackWidth = jmin (7.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);
        auto voffset = (isTwoVal || isThreeVal) ? trackWidth*0.75f : trackWidth*0.5f;
        
        Point<float> startPoint (slider.isHorizontal() ? x : x + width * 0.5f,
                                 slider.isHorizontal() ? y + height * 0.5f + voffset: height + y);
        
        Point<float> endPoint (slider.isHorizontal() ? width + x : startPoint.x,
                               slider.isHorizontal() ? startPoint.y : y);
        
        Path backgroundTrack;
        backgroundTrack.startNewSubPath (startPoint);
        backgroundTrack.lineTo (endPoint);
        g.setColour (slider.findColour (Slider::backgroundColourId));
        g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
        
        Path valueTrack;
        Point<float> minPoint, maxPoint, thumbPoint;
        
        if (isTwoVal || isThreeVal)
        {
            minPoint = { slider.isHorizontal() ? minSliderPos : width * 0.5f,
                slider.isHorizontal() ? height * 0.5f + voffset : minSliderPos };
            
            if (isThreeVal)
                thumbPoint = { slider.isHorizontal() ? sliderPos : width * 0.5f,
                    slider.isHorizontal() ? height * 0.5f + voffset: sliderPos };
            
            maxPoint = { slider.isHorizontal() ? maxSliderPos : width * 0.5f,
                slider.isHorizontal() ? height * 0.5f + voffset : maxSliderPos };
        }
        else
        {
            auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
            auto ky = slider.isHorizontal() ? (y + height * 0.5f + voffset) : sliderPos;
         
            if (slider.getProperties().contains ("fromCentre")) {
                if (kx > startPoint.x + width/2) {
                    minPoint = startPoint.translated(width/2, 0.0);
                    maxPoint = { kx, ky };                    
                } else {
                    minPoint = { kx, ky };
                    maxPoint = startPoint.translated(width/2, 0.0);
                }
                
                if (abs(minPoint.x - maxPoint.x) < 0.5) {
                    // make sure they are tiny bit different
                    maxPoint.x += 0.1;
                }
            }
            else {
                minPoint = startPoint;
                maxPoint = { kx, ky };
            }
        }
        
        auto thumbWidth = getSliderThumbRadius (slider);
        
        valueTrack.startNewSubPath (minPoint);
        valueTrack.lineTo (isThreeVal ? thumbPoint : maxPoint);
        g.setColour (slider.findColour (Slider::trackColourId));
        g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
        
        if (! isTwoVal)
        {
           // g.setColour (slider.findColour (Slider::thumbColourId));
           // g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (isThreeVal ? thumbPoint : maxPoint));

            auto sr = jmin (trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
            auto pointerColour = slider.findColour (Slider::thumbColourId);
            
            auto wscale = 1.5f;

            if (slider.isHorizontal())
            {
                /*
                drawPointer (g, minSliderPos - sr,
                             height * 0.5f - trackWidth*wscale*0.5, //jmax (0.0f, y + height * 0.5f - trackWidth * 0.5f),
                             trackWidth * wscale, pointerColour, 1); // 2
                
                drawPointer (g, maxSliderPos - trackWidth,
                             height * 0.5f - trackWidth*wscale*0.5, //jmin (y + height - trackWidth * 2.0f, (float)y + height * 0.5f),
                             trackWidth * wscale, pointerColour, 3); // 4
                */
                
                //drawPointer (g, minSliderPos - sr,
                //             jmax (0.0f, y + height * 0.5f - trackWidth * wscale),
                //             trackWidth * wscale, pointerColour, 2); // 2
                
                drawPointer (g, sliderPos - trackWidth*0.5*wscale,
                             jmin (y + height - trackWidth * wscale, (float)y + height * 0.5f + voffset),
                             trackWidth * wscale, pointerColour, 4); // 4
            }
            else
            {
                //drawPointer (g, jmax (0.0f, x + width * 0.5f - trackWidth * 2.0f),
                //             minSliderPos - trackWidth,
                //             trackWidth * wscale, pointerColour, 1);
                
                drawPointer (g, jmin (x + width - trackWidth * 2.0f, x + width * 0.5f), sliderPos - sr,
                             trackWidth * wscale, pointerColour, 3);
            }
        }
        
        if (isTwoVal || isThreeVal)
        {
            auto sr = jmin (trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
            auto pointerColour = slider.findColour (Slider::thumbColourId);
            
            auto wscale = 1.5f;
            
            if (slider.isHorizontal())
            {
                /*
                drawPointer (g, minSliderPos - sr,
                             height * 0.5f - trackWidth*wscale*0.5, //jmax (0.0f, y + height * 0.5f - trackWidth * 0.5f),
                             trackWidth * wscale, pointerColour, 1); // 2
                
                drawPointer (g, maxSliderPos - trackWidth,
                             height * 0.5f - trackWidth*wscale*0.5, //jmin (y + height - trackWidth * 2.0f, (float)y + height * 0.5f),
                             trackWidth * wscale, pointerColour, 3); // 4
                */
                
                drawPointer (g, minSliderPos - sr,
                             jmax (0.0f, y + height * 0.5f  + voffset - trackWidth * wscale),
                             trackWidth * wscale, pointerColour, 2); // 2
                
                drawPointer (g, maxSliderPos - trackWidth*0.5*wscale,
                             jmin (y + height - trackWidth * wscale, (float)y + height * 0.5f + voffset),
                             trackWidth * wscale, pointerColour, 4); // 4
            }
            else
            {
                drawPointer (g, jmax (0.0f, x + width * 0.5f - trackWidth * 2.0f),
                             minSliderPos - trackWidth,
                             trackWidth * wscale, pointerColour, 1);
                
                drawPointer (g, jmin (x + width - trackWidth * 2.0f, x + width * 0.5f), maxSliderPos - sr,
                             trackWidth * wscale, pointerColour, 3);
            }
        }
    }
}


#endif

void SonoDashedBorderButtonLookAndFeel::drawButtonBackground(Graphics& g, Button& button, const Colour& backgroundColour,
                                                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto cornerSize = 6.0f;
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
        .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);

    g.setColour (baseColour);

    auto flatOnLeft   = button.isConnectedOnLeft();
    auto flatOnRight  = button.isConnectedOnRight();
    auto flatOnTop    = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    // Compute stroke
    float strokeThickness = 1.5;
    PathStrokeType stroke(strokeThickness, PathStrokeType::JointStyle::curved);
    float dash[2] = {10.0f, 10.0f};

    Path strokePath;
    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(),
            bounds.getWidth(), bounds.getHeight(),
            cornerSize, cornerSize,
            ! (flatOnLeft  || flatOnTop),
            ! (flatOnRight || flatOnTop),
            ! (flatOnLeft  || flatOnBottom),
            ! (flatOnRight || flatOnBottom));

        g.fillPath (path);

        auto reducedBounds = bounds.reduced(strokeThickness);
        strokePath.addRoundedRectangle (reducedBounds.getX(), reducedBounds.getY(),
            reducedBounds.getWidth(), reducedBounds.getHeight(),
            cornerSize, cornerSize,
            ! (flatOnLeft  || flatOnTop),
            ! (flatOnRight || flatOnTop),
            ! (flatOnLeft  || flatOnBottom),
            ! (flatOnRight || flatOnBottom));
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);

        strokePath.addRoundedRectangle(bounds.reduced(strokeThickness), cornerSize);
    }

    // Draw border
    stroke.createDashedStroke(strokePath, strokePath, dash, 2);

    g.setColour(button.findColour(ComboBox::outlineColourId));
    g.strokePath(strokePath, stroke);
}

MixerConsoleLookAndFeel::MixerConsoleLookAndFeel()
{
    setColour (Slider::backgroundColourId, Colour (0xff0C0D10));
    setColour (Slider::trackColourId, SlimUi::accentBlue());
    setColour (Slider::thumbColourId, Colour (0xffE8E9ED));
    setColour (Slider::rotarySliderOutlineColourId, Colour (0xff2A2C32));
    setColour (Slider::rotarySliderFillColourId, SlimUi::accentBlue());
    setColour (Slider::textBoxTextColourId, SlimUi::text());
    setColour (Slider::textBoxBackgroundColourId, Colours::transparentBlack);
    setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
}

int MixerConsoleLookAndFeel::getSliderThumbRadius (Slider& slider)
{
    if (slider.isVertical())
        return 22;
    if (slider.isRotary())
        return 12;
    return SonoLookAndFeel::getSliderThumbRadius (slider);
}

Slider::SliderLayout MixerConsoleLookAndFeel::getSliderLayout (Slider& slider)
{
    auto layout = SonoLookAndFeel::getSliderLayout (slider);
    if (slider.isVertical() && slider.getTextBoxPosition() != Slider::NoTextBox)
    {
        layout.textBoxBounds = slider.getLocalBounds().removeFromTop (16).reduced (2, 0);
        layout.sliderBounds = slider.getLocalBounds().withTrimmedTop (14).reduced (2, 4);
    }
    return layout;
}

Label* MixerConsoleLookAndFeel::createSliderTextBox (Slider& slider)
{
    auto* lab = LookAndFeel_V4::createSliderTextBox (slider);
    lab->setKeyboardType (TextInputTarget::decimalKeyboard);
    lab->setFont (SlimUi::displayBold (11.0f));
    lab->setJustificationType (Justification::centred);
    lab->setMinimumHorizontalScale (0.4f);
    lab->setColour (Label::textColourId, SlimUi::text());
    lab->setColour (Label::backgroundColourId, Colours::transparentBlack);
    lab->setColour (Label::outlineColourId, Colours::transparentBlack);
    lab->setColour (TextEditor::backgroundColourId, SlimUi::sidebar());
    lab->setColour (TextEditor::outlineColourId, SlimUi::outline());
    return lab;
}

Font MixerConsoleLookAndFeel::getLabelFont (Label& label)
{
    return SlimUi::displayBold (11.0f);
}

void MixerConsoleLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                                const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
    auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const auto radius = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (Colours::black.withAlpha (0.4f));
    g.fillEllipse (centre.x - radius + 1.0f, centre.y - radius + 2.5f, radius * 2.0f, radius * 2.0f);

    ColourGradient ring (Colour (0xff555861), centre.x, centre.y - radius,
                         Colour (0xff1A1C21), centre.x, centre.y + radius, false);
    g.setGradientFill (ring);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    const auto inner = radius * 0.72f;
    ColourGradient face (Colour (0xff3E424C), centre.x, centre.y - inner,
                         Colour (0xff22242A), centre.x, centre.y + inner, false);
    g.setGradientFill (face);
    g.fillEllipse (centre.x - inner, centre.y - inner, inner * 2.0f, inner * 2.0f);

    g.setColour (Colour (0x33ffffff));
    g.drawEllipse (centre.x - inner, centre.y - inner, inner * 2.0f, inner * 2.0f, 1.0f);

    Path valueArc;
    const auto arcR = radius - 1.8f;
    auto startAngle = rotaryStartAngle;
    if (slider.getProperties().contains ("fromCentre"))
        startAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;

    valueArc.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, toAngle, true);
    g.setColour (slider.findColour (Slider::rotarySliderFillColourId));
    g.strokePath (valueArc, PathStrokeType (2.6f, PathStrokeType::curved, PathStrokeType::rounded));

    const auto pointerLen = radius * 0.52f;
    const auto px = centre.x + pointerLen * std::cos (toAngle - MathConstants<float>::halfPi);
    const auto py = centre.y + pointerLen * std::sin (toAngle - MathConstants<float>::halfPi);
    g.setColour (Colour (0xffF2F3F5));
    g.drawLine (centre.x, centre.y, px, py, 2.0f);
    g.fillEllipse (centre.x - 2.4f, centre.y - 2.4f, 4.8f, 4.8f);
}

void MixerConsoleLookAndFeel::drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                                                float sliderPos, float minSliderPos, float maxSliderPos,
                                                const Slider::SliderStyle style, Slider& slider)
{
    if (! slider.isVertical() || slider.isBar() || style == Slider::TwoValueVertical || style == Slider::ThreeValueVertical)
    {
        SonoLookAndFeel::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    auto bounds = Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
    const float slotW = 7.0f;
    auto slot = Rectangle<float> (bounds.getCentreX() - slotW * 0.5f, bounds.getY() + 8.0f,
                                  slotW, jmax (8.0f, bounds.getHeight() - 16.0f));

    Colour strip = slider.findColour (Slider::trackColourId);
    if (slider.getProperties().contains ("stripColour"))
        strip = Colour ((uint32) (int) slider.getProperties()["stripColour"]);

    const float tickX = slot.getX() - 14.0f;
    g.setFont (SlimUi::displayRegular (8.5f));
    g.setColour (SlimUi::textDim().withAlpha (0.8f));
    const float dbs[] = { 6.0f, 0.0f, -6.0f, -12.0f, -24.0f, -48.0f };
    for (auto db : dbs)
    {
        const auto gain = Decibels::decibelsToGain (db);
        const auto p = (float) slider.valueToProportionOfLength ((double) gain);
        const float ty = slot.getY() + slot.getHeight() * (1.0f - p);
        g.setColour (std::abs (db) < 0.01f ? Colour (0xffF2F3F5) : SlimUi::textDim().withAlpha (0.7f));
        g.fillRect (slot.getX() - 5.0f, ty, 4.0f, 1.0f);
        if (tickX > bounds.getX())
            g.drawText (db > 0 ? "+" + String ((int) db) : String ((int) db),
                        Rectangle<float> (bounds.getX(), ty - 6.0f, 16.0f, 12.0f),
                        Justification::centredRight, false);
    }

    g.setColour (Colour (0xff07080A));
    g.fillRoundedRectangle (slot, 3.5f);
    g.setColour (Colour (0x66000000));
    g.drawRoundedRectangle (slot, 3.5f, 1.0f);

    const float fillTop = jlimit (slot.getY(), slot.getBottom(), sliderPos);
    auto fill = Rectangle<float> (slot.getX(), fillTop, slot.getWidth(), slot.getBottom() - fillTop);
    ColourGradient fillGrad (strip.brighter (0.15f), fill.getCentreX(), fill.getBottom(),
                             strip.darker (0.25f), fill.getCentreX(), fill.getY(), false);
    g.setGradientFill (fillGrad);
    g.fillRoundedRectangle (fill, 3.0f);

    const float capW = jmin (bounds.getWidth() * 0.72f, 32.0f);
    const float capH = 42.0f;
    auto cap = Rectangle<float> (bounds.getCentreX() - capW * 0.5f, sliderPos - capH * 0.5f, capW, capH);
    cap = cap.constrainedWithin (bounds.reduced (1.0f, 1.0f));

    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (cap.translated (0.0f, 2.0f), 5.0f);

    ColourGradient capGrad (Colour (0xff5A5E68), cap.getX(), cap.getY(),
                            Colour (0xff2A2C32), cap.getX(), cap.getBottom(), false);
    g.setGradientFill (capGrad);
    g.fillRoundedRectangle (cap, 5.0f);

    auto highlight = cap.removeFromTop (capH * 0.38f).reduced (1.2f, 0.0f);
    g.setColour (Colour (0x33ffffff));
    g.fillRoundedRectangle (highlight, 4.0f);

    g.setColour (Colour (0xff1A1C21));
    g.drawRoundedRectangle (Rectangle<float> (bounds.getCentreX() - capW * 0.5f, sliderPos - capH * 0.5f, capW, capH)
                                .constrainedWithin (bounds.reduced (1.0f, 1.0f)), 5.0f, 1.0f);

    g.setColour (strip);
    g.fillRoundedRectangle (Rectangle<float> (bounds.getCentreX() - capW * 0.32f, sliderPos - 1.3f, capW * 0.64f, 2.6f), 1.2f);
    g.setColour (Colour (0xffF2F3F5));
    g.fillRoundedRectangle (Rectangle<float> (bounds.getCentreX() - capW * 0.18f, sliderPos - 0.7f, capW * 0.36f, 1.4f), 0.7f);
}
