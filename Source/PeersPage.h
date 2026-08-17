// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#include <JuceHeader.h>
#include "SlimUi.h"

class PeersPage : public Component
{
public:
    explicit PeersPage (Component& mixerChrome);
    ~PeersPage() override = default;

    void paint (Graphics& g) override;
    void resized() override;

    Component& getMixerChrome() { return mixerChrome; }

private:
    Component& mixerChrome;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PeersPage)
};
