// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#include <JuceHeader.h>
#include "SlimUi.h"

class ConnectPage;
class PeersPage;
class RemoteControlPage;
class SoundboardPage;
class SettingsPage;

class AppShell : public Component,
                 public Button::Listener
{
public:
    AppShell();
    ~AppShell() override = default;

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* button) override;

    void setPages (ConnectPage* connect,
                   PeersPage* peers,
                   RemoteControlPage* groupControl,
                   SoundboardPage* soundboard,
                   SettingsPage* settings);

    void setPage (AppPage page);
    AppPage getPage() const { return currentPage; }

    void updateHeader (bool connected,
                       const String& group,
                       const String& user,
                       const String& extra,
                       const String& statusText = {});

    std::function<void(AppPage)> onPageChanged;

    Rectangle<int> getPageBounds() const { return pageHost.getBounds(); }

private:
    void updateNavState();

    ImageComponent logo;
    TextButton networkButton { "Network" };
    TextButton peersButton { "Peers" };
    TextButton groupControlButton { "Group Control" };
    TextButton soundboardButton { "Soundboard" };
    TextButton settingsButton { "Settings" };
    Label footer { {}, "Private group / Direct IP" };
    Label credit { {}, "Based on SonoBus" };

    Component header;
    Label statusPill { {}, "Disconnected" };
    Label groupLabel { {}, "Not in a group" };
    Label userLabel;
    Label extraLabel;

    Component pageHost;

    ConnectPage* connectPage = nullptr;
    PeersPage* peersPage = nullptr;
    RemoteControlPage* groupControlPage = nullptr;
    SoundboardPage* soundboardPage = nullptr;
    SettingsPage* settingsPage = nullptr;

    AppPage currentPage = AppPage::Network;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppShell)
};
