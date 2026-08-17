// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#include "SettingsPage.h"

SettingsPage::SettingsPage()
{
    credit.setFont (SlimUi::displayRegular (12.0f));
    credit.setColour (Label::textColourId, SlimUi::textDim());
    credit.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (credit);
}

void SettingsPage::setOptionsView (Component* options)
{
    if (optionsView == options)
        return;

    if (optionsView != nullptr)
        removeChildComponent (optionsView);

    optionsView = options;

    if (optionsView != nullptr)
    {
        addChildComponent (optionsView);
        optionsView->setVisible (isVisible());
    }

    resized();
}

void SettingsPage::setInputMixer (Component* mixer)
{
    if (mixerView == mixer)
        return;

    if (mixerView != nullptr)
        removeChildComponent (mixerView);

    mixerView = mixer;

    if (mixerView != nullptr)
    {
        addChildComponent (mixerView);
        mixerView->setVisible (isVisible());
    }

    resized();
}

void SettingsPage::visibilityChanged()
{
    if (optionsView != nullptr)
        optionsView->setVisible (isVisible());
    if (mixerView != nullptr)
        mixerView->setVisible (isVisible());
}

void SettingsPage::paint (Graphics& g)
{
    g.fillAll (SlimUi::background());
}

void SettingsPage::resized()
{
    auto bounds = getLocalBounds().reduced (12);
    credit.setBounds (bounds.removeFromBottom (20));
    bounds.removeFromBottom (8);

    if (mixerView != nullptr)
    {
        // Give the mixer close to half the page so the strips have room to grow, but
        // always leave enough for the options list, which can scroll on its own.
        const int wanted = jlimit (400, 640, bounds.getHeight() * 46 / 100);
        const int mixerH = jmax (240, jmin (wanted, bounds.getHeight() - 150));
        mixerView->setBounds (bounds.removeFromBottom (mixerH));
    }

    if (mixerView != nullptr && optionsView != nullptr && bounds.getHeight() > 10)
        bounds.removeFromBottom (10);

    if (optionsView != nullptr)
        optionsView->setBounds (bounds);
}
