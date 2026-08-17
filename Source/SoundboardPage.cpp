// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#include "SoundboardPage.h"

SoundboardPage::SoundboardPage (Component& view)
    : soundboardView (view)
{
    addAndMakeVisible (soundboardView);
}

void SoundboardPage::paint (Graphics& g)
{
    g.fillAll (SlimUi::background());
}

void SoundboardPage::resized()
{
    soundboardView.setBounds (getLocalBounds().reduced (8));
}
