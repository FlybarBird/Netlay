// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#include <JuceHeader.h>
#include "SlimUi.h"

class SoundboardPage : public Component
{
public:
    explicit SoundboardPage (Component& soundboardView);
    ~SoundboardPage() override = default;

    void paint (Graphics& g) override;
    void resized() override;

private:
    Component& soundboardView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundboardPage)
};
