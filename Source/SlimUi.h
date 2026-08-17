// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#include <JuceHeader.h>

namespace SlimUi
{
    // Sampled from the Connect / Devices / Peers mockups
    inline Colour background()      { return Colour (0xff1B1D23); }
    inline Colour sidebar()         { return Colour (0xff16181E); }
    inline Colour header()          { return Colour (0xff1A1B1E); }
    inline Colour card()            { return Colour (0xff202124); }
    inline Colour cardRaised()      { return Colour (0xff252729); }
    inline Colour outline()         { return Colour (0x33ffffff); }
    inline Colour accentBlue()      { return Colour (0xff3581D6); }
    inline Colour accentPurple()    { return accentBlue(); }
    inline Colour accentLavender()  { return Colour (0xffF2F3F5); }
    inline Colour meterLow()        { return Colour (0xff1A5FBF); }
    inline Colour meterMid()        { return Colour (0xff2FB0EE); }
    inline Colour meterHigh()       { return Colour (0xff47C0E9); }
    inline Colour mute()            { return Colour (0xff844B43); }
    inline Colour text()            { return Colour (0xffF2F3F5); }
    inline Colour textDim()         { return Colour (0xff9CA3AF); }
    inline Colour success()         { return Colour (0xff26BA5A); }
    inline Colour mixerWell()       { return Colour (0xff14161B); }
    inline Colour mixerStrip()      { return Colour (0xff1C1E24); }
    inline Colour mixerFaderCap()   { return Colour (0xff3A3D46); }

    inline Colour mixerStripColour (int index)
    {
        static const uint32 cols[] = {
            0xff3581D6, 0xff2FB0EE, 0xff3DDC97, 0xffE0B43A,
            0xffE07A3A, 0xffE05B7A, 0xff9B7AE0, 0xff5BB8A4
        };
        return Colour (cols[(unsigned) index % (unsigned) (sizeof (cols) / sizeof (cols[0]))]);
    }

    constexpr float cardRadius = 10.0f;
    constexpr float controlRadius = 8.0f;
    constexpr int sidebarWidth = 216;
    constexpr int headerHeight = 56;
    constexpr float regularTracking = 0.08f;

#if JUCE_MAC
    inline const char* displayFamily() { return ".AppleSystemUIFont"; } // SF Pro Display
#else
    inline const char* displayFamily() { return nullptr; }
#endif

    inline Font displayRegular (float height)
    {
#if JUCE_MAC
        return Font (displayFamily(), "Regular", height).withExtraKerningFactor (regularTracking);
#else
        return Font (height, Font::plain).withExtraKerningFactor (regularTracking);
#endif
    }

    inline Font displayBold (float height)
    {
#if JUCE_MAC
        return Font (displayFamily(), "Bold", height);
#else
        return Font (height, Font::bold);
#endif
    }

    inline Font applyDisplayStyle (Font f)
    {
#if JUCE_MAC
        f.setTypefaceName (displayFamily());
#endif
        if (f.isBold())
        {
            f.setTypefaceStyle ("Bold");
            return f;
        }

        f.setTypefaceStyle ("Regular");
        f.setExtraKerningFactor (jmax (f.getExtraKerningFactor(), regularTracking));
        return f;
    }

    inline void paintCard (Graphics& g, juce::Rectangle<int> bounds, float radius = cardRadius)
    {
        auto r = bounds.toFloat();
        g.setColour (card());
        g.fillRoundedRectangle (r, radius);
        g.setColour (outline());
        g.drawRoundedRectangle (r.reduced (0.5f), radius, 1.0f);
    }

    inline Image withDarkBackgroundRemoved (const Image& source)
    {
        if (source.isNull())
            return {};

        Image img (source.convertedToFormat (Image::ARGB));
        Image::BitmapData pixels (img, Image::BitmapData::readWrite);

        for (int y = 0; y < pixels.height; ++y)
        {
            for (int x = 0; x < pixels.width; ++x)
            {
                const auto c = pixels.getPixelColour (x, y);
                if (c.getBrightness() < 0.14f && c.getSaturation() < 0.28f)
                    pixels.setPixelColour (x, y, c.withAlpha (0.0f));
            }
        }

        return img;
    }
}

enum class AppPage
{
    Network,
    Peers,
    GroupControl,
    Mixer,
    Soundboard,
    Settings
};
