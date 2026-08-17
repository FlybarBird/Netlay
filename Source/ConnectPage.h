// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#include <JuceHeader.h>
#include "SlimUi.h"

class ConnectView;
class SonobusAudioProcessor;

class ConnectPage : public Component,
                    public Button::Listener
{
public:
    ConnectPage (ConnectView& connectView, SonobusAudioProcessor& processor);
    ~ConnectPage() override = default;

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* button) override;
    void visibilityChanged() override;

    void refreshServerCard();

private:
    ConnectView& connectView;
    SonobusAudioProcessor& processor;

    Component serverCard;
    Label serverTitle { {}, "Built-in Server" };
    Label serverDetail { {}, "Port 10999" };
    Label serverHealth { {}, "Healthy" };
    ToggleButton serverToggle { "Enabled" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectPage)
};
