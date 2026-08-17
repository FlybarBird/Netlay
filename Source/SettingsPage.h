// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#include <JuceHeader.h>
#include "SlimUi.h"

class SettingsPage : public Component
{
public:
    SettingsPage();
    ~SettingsPage() override = default;

    void paint (Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;

    void setOptionsView (Component* options);
    void setInputMixer (Component* mixer);

private:
    Component* optionsView = nullptr;
    Component* mixerView = nullptr;
    Label credit { {}, "Based on SonoBus" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsPage)
};
