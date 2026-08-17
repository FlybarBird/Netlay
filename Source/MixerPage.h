// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#include <JuceHeader.h>
#include "SlimUi.h"

class ChannelGroupsView;

class MixerPage : public Component
{
public:
    MixerPage (ChannelGroupsView& mixer,
               Component& sendChannels,
               Component& addGroup,
               Component* inputMute,
               Component* inputSolo);
    ~MixerPage() override = default;

    void paint (Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;

private:
    void layoutMixer();

    ChannelGroupsView& mixer;
    Component& sendChannels;
    Component& addGroup;
    Component* inputMute = nullptr;
    Component* inputSolo = nullptr;

    Label heading { {}, "Input Mixer" };
    Viewport mixerViewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerPage)
};
