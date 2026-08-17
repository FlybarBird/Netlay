// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#include "AppShell.h"
#include "ConnectPage.h"
#include "PeersPage.h"
#include "RemoteControlPage.h"
#include "SoundboardPage.h"
#include "SettingsPage.h"

namespace
{
    void styleNavButton (TextButton& button)
    {
        button.setClickingTogglesState (true);
        button.setRadioGroupId (1001);
        button.setColour (TextButton::buttonColourId, Colours::transparentBlack);
        button.setColour (TextButton::buttonOnColourId, SlimUi::accentPurple());
        button.setColour (TextButton::textColourOffId, SlimUi::textDim());
        button.setColour (TextButton::textColourOnId, SlimUi::accentLavender());
        button.setTriggeredOnMouseDown (true);
        button.setMouseClickGrabsKeyboardFocus (false);
        button.setWantsKeyboardFocus (false);
    }
}

AppShell::AppShell()
{
    setWantsKeyboardFocus (false);
    setMouseClickGrabsKeyboardFocus (false);
    header.setWantsKeyboardFocus (false);
    header.setInterceptsMouseClicks (false, true);
    pageHost.setWantsKeyboardFocus (false);

    const auto wordmark = ImageCache::getFromMemory (BinaryData::netlay_wordmark_png, BinaryData::netlay_wordmark_pngSize);
    logo.setOpaque (false);
    logo.setImage (wordmark.isValid() ? wordmark
                                     : ImageCache::getFromMemory (BinaryData::netlay_logo_png, BinaryData::netlay_logo_pngSize));
    logo.setImagePlacement (RectanglePlacement::xLeft | RectanglePlacement::centred
                            | RectanglePlacement::onlyReduceInSize);
    logo.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (logo);

    styleNavButton (networkButton);
    styleNavButton (peersButton);
    styleNavButton (groupControlButton);
    styleNavButton (soundboardButton);
    styleNavButton (settingsButton);

    networkButton.addListener (this);
    peersButton.addListener (this);
    groupControlButton.addListener (this);
    soundboardButton.addListener (this);
    settingsButton.addListener (this);

    addAndMakeVisible (networkButton);
    addAndMakeVisible (peersButton);
    addAndMakeVisible (groupControlButton);
    addAndMakeVisible (soundboardButton);
    addAndMakeVisible (settingsButton);

    footer.setFont (SlimUi::displayRegular (12.0f));
    footer.setColour (Label::textColourId, SlimUi::textDim());
    footer.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (footer);

    credit.setFont (SlimUi::displayRegular (12.0f));
    credit.setColour (Label::textColourId, SlimUi::textDim());
    credit.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (credit);

    header.setOpaque (false);
    addAndMakeVisible (header);

    statusPill.setFont (SlimUi::displayBold (13.0f));
    statusPill.setJustificationType (Justification::centred);
    statusPill.setColour (Label::textColourId, SlimUi::text());
    statusPill.setColour (Label::backgroundColourId, SlimUi::cardRaised());
    header.addAndMakeVisible (statusPill);

    groupLabel.setFont (SlimUi::displayBold (16.0f));
    groupLabel.setColour (Label::textColourId, SlimUi::text());
    header.addAndMakeVisible (groupLabel);

    userLabel.setFont (SlimUi::displayRegular (14.0f));
    userLabel.setColour (Label::textColourId, SlimUi::textDim());
    header.addAndMakeVisible (userLabel);

    extraLabel.setFont (SlimUi::displayRegular (13.0f));
    extraLabel.setColour (Label::textColourId, SlimUi::textDim());
    extraLabel.setJustificationType (Justification::centredRight);
    header.addAndMakeVisible (extraLabel);

    addAndMakeVisible (pageHost);

    networkButton.setToggleState (true, dontSendNotification);
}

void AppShell::setPages (ConnectPage* connect,
                         PeersPage* peers,
                         RemoteControlPage* groupControl,
                         SoundboardPage* soundboard,
                         SettingsPage* settings)
{
    connectPage = connect;
    peersPage = peers;
    groupControlPage = groupControl;
    soundboardPage = soundboard;
    settingsPage = settings;

    auto addPage = [this] (Component* page)
    {
        if (page != nullptr)
        {
            pageHost.addChildComponent (page);
            page->setVisible (false);
        }
    };

    addPage (connectPage);
    addPage (peersPage);
    addPage (groupControlPage);
    addPage (soundboardPage);
    addPage (settingsPage);

    setPage (AppPage::Network);
}

void AppShell::setPage (AppPage page)
{
    currentPage = page;

    if (connectPage)       connectPage->setVisible (page == AppPage::Network);
    if (peersPage)         peersPage->setVisible (page == AppPage::Peers);
    if (groupControlPage)  groupControlPage->setVisible (page == AppPage::GroupControl);
    if (soundboardPage)    soundboardPage->setVisible (page == AppPage::Soundboard);
    if (settingsPage)      settingsPage->setVisible (page == AppPage::Settings);

    footer.setVisible (page == AppPage::Network);
    updateNavState();
    resized();

    if (onPageChanged)
        onPageChanged (page);
}

void AppShell::updateHeader (bool connected, const String& group, const String& user, const String& extra, const String& statusText)
{
    statusPill.setText (statusText.isNotEmpty()
                            ? statusText
                            : (connected ? String ("Connected") : String ("Disconnected")),
                        dontSendNotification);
    statusPill.setColour (Label::backgroundColourId,
                          connected ? SlimUi::accentBlue() : SlimUi::cardRaised());
    statusPill.setColour (Label::textColourId, connected ? SlimUi::accentLavender() : SlimUi::textDim());

    groupLabel.setText (group.isNotEmpty() ? group : String ("Not in a group"), dontSendNotification);
    userLabel.setText (user, dontSendNotification);
    extraLabel.setText (extra, dontSendNotification);
}

void AppShell::updateNavState()
{
    networkButton.setToggleState (currentPage == AppPage::Network, dontSendNotification);
    peersButton.setToggleState (currentPage == AppPage::Peers, dontSendNotification);
    groupControlButton.setToggleState (currentPage == AppPage::GroupControl, dontSendNotification);
    soundboardButton.setToggleState (currentPage == AppPage::Soundboard, dontSendNotification);
    settingsButton.setToggleState (currentPage == AppPage::Settings, dontSendNotification);
}

void AppShell::buttonClicked (Button* button)
{
    if (button == &networkButton)               setPage (AppPage::Network);
    else if (button == &peersButton)            setPage (AppPage::Peers);
    else if (button == &groupControlButton)     setPage (AppPage::GroupControl);
    else if (button == &soundboardButton)       setPage (AppPage::Soundboard);
    else if (button == &settingsButton)         setPage (AppPage::Settings);
}

void AppShell::paint (Graphics& g)
{
    g.fillAll (SlimUi::background());

    auto bounds = getLocalBounds();
    auto side = bounds.removeFromLeft (SlimUi::sidebarWidth);

    g.setColour (SlimUi::sidebar());
    g.fillRect (side);

    g.setColour (SlimUi::outline());
    g.drawLine ((float) side.getRight(), 0.0f, (float) side.getRight(), (float) getHeight(), 1.0f);

    auto head = bounds.removeFromTop (SlimUi::headerHeight);
    g.setColour (SlimUi::header());
    g.fillRect (head);
    g.setColour (SlimUi::outline());
    g.drawLine ((float) head.getX(), (float) head.getBottom(), (float) head.getRight(), (float) head.getBottom(), 1.0f);
}

void AppShell::resized()
{
    auto bounds = getLocalBounds();
    auto side = bounds.removeFromLeft (SlimUi::sidebarWidth).reduced (16, 18);

    logo.setBounds (side.removeFromTop (58));
    side.removeFromTop (16);

    const int navH = 36;
    const int navGap = 6;
    networkButton.setBounds (side.removeFromTop (navH));
    side.removeFromTop (navGap);
    peersButton.setBounds (side.removeFromTop (navH));
    side.removeFromTop (navGap);
    groupControlButton.setBounds (side.removeFromTop (navH));
    side.removeFromTop (navGap);
    soundboardButton.setBounds (side.removeFromTop (navH));
    side.removeFromTop (navGap);
    settingsButton.setBounds (side.removeFromTop (navH));

    credit.setBounds (side.removeFromBottom (22));
    footer.setBounds (side.removeFromBottom (36));

    auto head = bounds.removeFromTop (SlimUi::headerHeight).reduced (16, 10);
    statusPill.setBounds (head.removeFromLeft (jmin (188, jmax (118, head.getWidth() / 4))));
    head.removeFromLeft (12);
    extraLabel.setBounds (head.removeFromRight (jmin (220, head.getWidth() / 3)));
    head.removeFromRight (8);

    auto names = head;
    groupLabel.setBounds (names.removeFromTop (names.getHeight() / 2));
    userLabel.setBounds (names);

    header.setBounds (getLocalBounds().withTrimmedLeft (SlimUi::sidebarWidth).removeFromTop (SlimUi::headerHeight));
    pageHost.setBounds (getLocalBounds().withTrimmedLeft (SlimUi::sidebarWidth).withTrimmedTop (SlimUi::headerHeight));

    auto pageBounds = pageHost.getLocalBounds();
    if (connectPage)       connectPage->setBounds (pageBounds);
    if (peersPage)         peersPage->setBounds (pageBounds);
    if (groupControlPage)  groupControlPage->setBounds (pageBounds);
    if (soundboardPage)    soundboardPage->setBounds (pageBounds);
    if (settingsPage)      settingsPage->setBounds (pageBounds);
}
