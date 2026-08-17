// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#include <JuceHeader.h>
#include <map>
#include "SlimUi.h"
#include "SonoLookAndFeel.h"
#include "RemoteMixTypes.h"
#include "SonobusPluginProcessor.h"

class RemoteControlPage : public Component,
                          public Timer,
                          public SonobusAudioProcessor::ClientListener
{
public:
    explicit RemoteControlPage (SonobusAudioProcessor& processor);
    ~RemoteControlPage() override;

    void paint (Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;
    void timerCallback() override;

    void remoteMixStateReceived (SonobusAudioProcessor*, const String& fromUser,
                                 const Array<RemoteMix::SourceState>& sources) override;
    void remoteMixControlAck (SonobusAudioProcessor*, const String& targetUser,
                              const String& sourceId, float gain, bool muted) override;
    void remoteMixControlFailed (SonobusAudioProcessor*, const String& targetUser,
                                 const String& sourceId, int reason) override;
    void remoteMixBeingControlled (SonobusAudioProcessor*, const String& fromUser) override;
    void aooClientPeerJoined (SonobusAudioProcessor*, const String&, const String&) override;
    void aooClientPeerLeft (SonobusAudioProcessor*, const String&, const String&) override;

    void refreshFromProcessor();

private:
    class MixStrip : public Component, public Slider::Listener, public Button::Listener
    {
    public:
        MixStrip (RemoteControlPage& owner, MixerConsoleLookAndFeel& lnf);
        ~MixStrip() override;
        void setSource (const RemoteMix::SourceState& src, int colourIndex);
        void setConfirmed (float gain, bool muted);
        void setPending (bool flag);
        bool isPending() const { return pending; }
        void setMeterSource (foleys::LevelMeterSource* src);
        void paint (Graphics& g) override;
        void resized() override;
        void sliderValueChanged (Slider*) override;
        void sliderDragStarted (Slider*) override;
        void sliderDragEnded (Slider*) override;
        void buttonClicked (Button*) override;
        String sourceId;
        bool dragging = false;

    private:
        RemoteControlPage& owner;
        Label nameLabel;
        Slider fader;
        TextButton mute { "MUTE" };
        std::unique_ptr<foleys::LevelMeter> meter;
        SonoLookAndFeel meterLnf;
        Colour stripColor { SlimUi::accentBlue() };
        float confirmedGain = 1.0f;
        bool confirmedMute = false;
        bool pending = false;
    };

    class GroupCard : public Component
    {
    public:
        GroupCard();
        void setGroup (const RemoteMix::ControlGroup& g, bool selected, int onlineCount);
        void paint (Graphics& g) override;
        void resized() override;
        void mouseUp (const MouseEvent&) override;
        std::function<void()> onSelect;
        String groupId;
    private:
        Label name;
        Label meta;
        bool selected = false;
    };

    class SourceRow : public Component
    {
    public:
        SourceRow();
        void setSource (const RemoteMix::SourceState& s, bool inGroup);
        void paint (Graphics& g) override;
        void resized() override;
        ToggleButton check;
        String sourceId;
        Label name;
        Label tag;
    };

    void rebuildGroups();
    void rebuildSources();
    void rebuildStrips();
    void selectGroup (int index);
    void querySelectedMembers();
    void sendStripChange (const String& sourceId, float gain, bool muted, bool live);
    void applySnapshot (const Array<RemoteMix::SourceState>& sources);
    void restoreStrip (const String& sourceId);
    void addGroupPressed();
    void manageMembersPressed();
    void addSelectedSources();
    void resetLevels();
    RemoteMix::ControlGroup currentGroup() const;
    StringArray currentTargets() const;

    SonobusAudioProcessor& processor;
    MixerConsoleLookAndFeel mixerLnf;

    Label groupsHeading { {}, "Groups" };
    TextButton newGroupButton { "+ New Group" };
    TextButton manageMembersButton { "Manage Members" };
    Viewport groupsViewport;
    Component groupsList;
    OwnedArray<GroupCard> groupCards;
    Label membersHeading { {}, "Members" };
    Viewport membersViewport;
    Component membersList;
    OwnedArray<Label> memberLabels;

    Label mixerHeading { {}, "Remote Levels" };
    TextButton prevBank { "< Inputs 1-6" };
    TextButton nextBank { "Inputs 7-12 >" };
    TextButton resetButton { "Reset Levels" };
    Viewport stripsViewport;
    Component stripsHost;
    OwnedArray<MixStrip> strips;
    Label emptyLabel { {}, "Join a group and add sources to mix remotely." };
    Label footerHint;

    Label sourcesHeading { {}, "Sources" };
    TextEditor search;
    TextButton filterAll { "All" };
    TextButton filterLocal { "Local" };
    TextButton filterRemote { "Remote" };
    Viewport sourcesViewport;
    Component sourcesList;
    OwnedArray<SourceRow> sourceRows;
    TextButton addSelectedButton { "Add Selected to Group" };

    Label controlledBy;

    int bankPage = 0;
    int filterMode = 0;
    uint32 lastQueryMs = 0;
    uint32 lastLiveMs = 0;
    std::map<String, RemoteMix::SourceState> confirmed;
    String lastSnapshotFrom;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RemoteControlPage)
};
