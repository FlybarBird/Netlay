// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#include "ConnectPage.h"
#include "ConnectView.h"
#include "SonobusPluginProcessor.h"

ConnectPage::ConnectPage (ConnectView& view, SonobusAudioProcessor& proc)
    : connectView (view), processor (proc)
{
    addAndMakeVisible (connectView);
    connectView.setEmbeddedInShell (true);

    serverTitle.setFont (SlimUi::displayBold (15.0f));
    serverTitle.setColour (Label::textColourId, SlimUi::text());
    serverDetail.setFont (SlimUi::displayRegular (13.0f));
    serverDetail.setColour (Label::textColourId, SlimUi::textDim());
    serverHealth.setColour (Label::textColourId, SlimUi::success());
    serverHealth.setJustificationType (Justification::centredRight);

    serverToggle.setClickingTogglesState (true);
    serverToggle.setColour (ToggleButton::textColourId, SlimUi::text());
    serverToggle.addListener (this);

    serverCard.addAndMakeVisible (serverTitle);
    serverCard.addAndMakeVisible (serverDetail);
    serverCard.addAndMakeVisible (serverHealth);
    serverCard.addAndMakeVisible (serverToggle);
    addAndMakeVisible (serverCard);

    refreshServerCard();
}

void ConnectPage::refreshServerCard()
{
    const bool running = processor.isAooServerRunning();
    serverToggle.setToggleState (running, dontSendNotification);
    serverHealth.setText (running ? "Healthy" : "Stopped", dontSendNotification);
    serverHealth.setColour (Label::textColourId, running ? SlimUi::success() : SlimUi::textDim());
    serverDetail.setText (running ? "Port 10999  ·  peer discovery only, no audio" : "Port 10999  ·  off",
                          dontSendNotification);
}

void ConnectPage::buttonClicked (Button* button)
{
    if (button == &serverToggle)
    {
        if (serverToggle.getToggleState())
            processor.startAooServer();
        else
            processor.stopAooServer();

        refreshServerCard();
    }
}

void ConnectPage::visibilityChanged()
{
    if (isVisible())
    {
        connectView.setVisible (true);
        connectView.updateState();
        refreshServerCard();
    }
    else
    {
        connectView.giveAwayKeyboardFocus();
    }
}

void ConnectPage::paint (Graphics& g)
{
    g.fillAll (SlimUi::background());
    SlimUi::paintCard (g, serverCard.getBounds());
}

void ConnectPage::resized()
{
    auto bounds = getLocalBounds().reduced (16);

    auto card = bounds.removeFromBottom (72);
    serverCard.setBounds (card);

    auto inner = serverCard.getLocalBounds().reduced (16, 10);
    serverToggle.setBounds (inner.removeFromRight (90).reduced (0, 8));
    inner.removeFromRight (8);
    serverHealth.setBounds (inner.removeFromRight (90));
    serverTitle.setBounds (inner.removeFromTop (inner.getHeight() / 2));
    serverDetail.setBounds (inner);

    bounds.removeFromBottom (12);
    connectView.setBounds (bounds);
}
