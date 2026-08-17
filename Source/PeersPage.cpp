// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#include "PeersPage.h"

PeersPage::PeersPage (Component& mixer)
    : mixerChrome (mixer)
{
    setWantsKeyboardFocus (true);
    addAndMakeVisible (mixerChrome);
}

void PeersPage::paint (Graphics& g)
{
    g.fillAll (SlimUi::background());
}

void PeersPage::resized()
{
    mixerChrome.setBounds (getLocalBounds());
}
