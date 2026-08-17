// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#include "RemoteControlPage.h"

namespace
{
    constexpr int kBankSize = 6;
    constexpr int kStripW = 108;
}

RemoteControlPage::MixStrip::MixStrip (RemoteControlPage& o, MixerConsoleLookAndFeel& lnf)
    : owner (o)
{
    nameLabel.setJustificationType (Justification::centred);
    nameLabel.setFont (SlimUi::displayBold (12.0f));
    nameLabel.setColour (Label::textColourId, Colours::white);
    addAndMakeVisible (nameLabel);

    fader.setSliderStyle (Slider::LinearVertical);
    fader.setTextBoxStyle (Slider::TextBoxBelow, true, 72, 18);
    fader.setRange (0.0, 2.0, 0.0);
    fader.setSkewFactor (0.5);
    fader.setDoubleClickReturnValue (true, 1.0);
    fader.setScrollWheelEnabled (false);
    fader.textFromValueFunction = [] (double v) {
        return Decibels::toString (Decibels::gainToDecibels ((float) v), 1);
    };
    fader.valueFromTextFunction = [] (const String& s) {
        return (double) Decibels::decibelsToGain (s.getFloatValue());
    };
    fader.setLookAndFeel (&lnf);
    fader.addListener (this);
    addAndMakeVisible (fader);

    mute.setClickingTogglesState (true);
    mute.setColour (TextButton::buttonOnColourId, Colour (0xffC4453C));
    mute.setColour (TextButton::buttonColourId, Colour (0xff2A2C32));
    mute.addListener (this);
    addAndMakeVisible (mute);

    auto flags = foleys::LevelMeter::Minimal;
    meter = std::make_unique<foleys::LevelMeter> (flags);
    meter->setLookAndFeel (&meterLnf);
    meter->setRefreshRateHz (8);
    addAndMakeVisible (meter.get());
}

RemoteControlPage::MixStrip::~MixStrip()
{
    fader.setLookAndFeel (nullptr);
    if (meter != nullptr)
        meter->setLookAndFeel (nullptr);
}

void RemoteControlPage::MixStrip::setSource (const RemoteMix::SourceState& src, int colourIndex)
{
    sourceId = src.id;
    nameLabel.setText (src.name.isNotEmpty() ? src.name : src.id, dontSendNotification);
    stripColor = SlimUi::mixerStripColour (colourIndex);
    fader.getProperties().set ("stripColour", (int) stripColor.getARGB());
    fader.setColour (Slider::trackColourId, stripColor);
    if (! dragging && ! pending)
    {
        fader.setValue (src.gain, dontSendNotification);
        mute.setToggleState (src.muted, dontSendNotification);
        confirmedGain = src.gain;
        confirmedMute = src.muted;
    }
    repaint();
}

void RemoteControlPage::MixStrip::setConfirmed (float gain, bool muted)
{
    confirmedGain = gain;
    confirmedMute = muted;
    if (! dragging)
    {
        fader.setValue (gain, dontSendNotification);
        mute.setToggleState (muted, dontSendNotification);
    }
    setPending (false);
}

void RemoteControlPage::MixStrip::setPending (bool flag)
{
    if (pending == flag)
        return;
    pending = flag;
    nameLabel.setColour (Label::textColourId, pending ? Colours::white.withAlpha (0.55f) : Colours::white);
    repaint();
}

void RemoteControlPage::MixStrip::setMeterSource (foleys::LevelMeterSource* src)
{
    if (meter == nullptr)
        return;
    meter->setMeterSource (src);
    meter->setFixedNumChannels (1);
}

void RemoteControlPage::MixStrip::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (SlimUi::mixerStrip());
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (Colour (0x22ffffff));
    g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

    auto header = r.removeFromTop (28.0f);
    Path hp;
    hp.addRoundedRectangle (header.getX(), header.getY(), header.getWidth(), header.getHeight(),
                            8.0f, 8.0f, true, true, false, false);
    g.setColour (stripColor);
    g.fillPath (hp);

    g.setColour (stripColor);
    g.fillRect (r.getX(), getHeight() - 6.0f, r.getWidth(), 6.0f);

    if (pending)
    {
        g.setColour (Colours::white.withAlpha (0.7f));
        g.fillEllipse (header.getRight() - 14.0f, header.getY() + 10.0f, 7.0f, 7.0f);
    }
}

void RemoteControlPage::MixStrip::resized()
{
    auto b = getLocalBounds().reduced (6);
    nameLabel.setBounds (b.removeFromTop (22));
    mute.setBounds (b.removeFromBottom (32).reduced (4, 2));
    b.removeFromBottom (8);
    meter->setBounds (b.removeFromRight (10));
    b.removeFromRight (4);
    fader.setBounds (b);
}

void RemoteControlPage::MixStrip::sliderValueChanged (Slider*)
{
    owner.sendStripChange (sourceId, (float) fader.getValue(), mute.getToggleState(), dragging);
}

void RemoteControlPage::MixStrip::sliderDragStarted (Slider*)
{
    dragging = true;
}

void RemoteControlPage::MixStrip::sliderDragEnded (Slider*)
{
    dragging = false;
    owner.sendStripChange (sourceId, (float) fader.getValue(), mute.getToggleState(), false);
}

void RemoteControlPage::MixStrip::buttonClicked (Button*)
{
    owner.sendStripChange (sourceId, (float) fader.getValue(), mute.getToggleState(), false);
}

RemoteControlPage::GroupCard::GroupCard()
{
    name.setFont (SlimUi::displayBold (15.0f));
    name.setColour (Label::textColourId, SlimUi::text());
    name.setInterceptsMouseClicks (false, false);
    meta.setFont (SlimUi::displayRegular (12.0f));
    meta.setColour (Label::textColourId, SlimUi::textDim());
    meta.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (name);
    addAndMakeVisible (meta);
}

void RemoteControlPage::GroupCard::setGroup (const RemoteMix::ControlGroup& g, bool sel, int onlineCount)
{
    groupId = g.id;
    selected = sel;
    name.setText (g.name, dontSendNotification);

    const bool anyUnreachable = onlineCount < g.members.size();
    String metaText = String (g.members.size()) + " members · " + String (onlineCount) + " online";
    if (anyUnreachable)
        metaText << " · " << String (g.members.size() - onlineCount) << " not responding";

    meta.setText (metaText, dontSendNotification);
    meta.setColour (Label::textColourId, anyUnreachable ? Colour (0xffC4453C) : SlimUi::textDim());
    repaint();
}

void RemoteControlPage::GroupCard::paint (Graphics& g)
{
    SlimUi::paintCard (g, getLocalBounds(), 8.0f);
    if (selected)
    {
        g.setColour (SlimUi::accentBlue());
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 8.0f, 2.0f);
    }
}

void RemoteControlPage::GroupCard::resized()
{
    auto b = getLocalBounds().reduced (10, 8);
    name.setBounds (b.removeFromTop (20));
    meta.setBounds (b);
}

void RemoteControlPage::GroupCard::mouseUp (const MouseEvent&)
{
    if (onSelect)
        onSelect();
}

RemoteControlPage::SourceRow::SourceRow()
{
    name.setFont (SlimUi::displayRegular (13.0f));
    name.setColour (Label::textColourId, SlimUi::text());
    tag.setFont (SlimUi::displayRegular (11.0f));
    tag.setColour (Label::textColourId, SlimUi::textDim());
    tag.setJustificationType (Justification::centredRight);
    addAndMakeVisible (check);
    addAndMakeVisible (name);
    addAndMakeVisible (tag);
}

void RemoteControlPage::SourceRow::setSource (const RemoteMix::SourceState& s, bool inGroup)
{
    sourceId = s.id;
    name.setText (s.name, dontSendNotification);
    tag.setText (s.kind == "peer" ? "Remote" : "Local", dontSendNotification);
    check.setToggleState (inGroup, dontSendNotification);
}

void RemoteControlPage::SourceRow::paint (Graphics& g)
{
    g.setColour (SlimUi::cardRaised());
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
}

void RemoteControlPage::SourceRow::resized()
{
    auto b = getLocalBounds().reduced (8, 4);
    check.setBounds (b.removeFromLeft (22));
    tag.setBounds (b.removeFromRight (56));
    name.setBounds (b);
}

RemoteControlPage::RemoteControlPage (SonobusAudioProcessor& proc)
    : processor (proc)
{
    auto setupHead = [] (Label& l, float size)
    {
        l.setFont (SlimUi::displayBold (size));
        l.setColour (Label::textColourId, SlimUi::text());
    };
    setupHead (groupsHeading, 16.0f);
    setupHead (membersHeading, 13.0f);
    setupHead (mixerHeading, 16.0f);
    setupHead (sourcesHeading, 16.0f);

    addAndMakeVisible (groupsHeading);
    addAndMakeVisible (newGroupButton);
    addAndMakeVisible (manageMembersButton);
    groupsViewport.setViewedComponent (&groupsList, false);
    groupsViewport.setScrollBarsShown (true, false);
    addAndMakeVisible (groupsViewport);
    addAndMakeVisible (membersHeading);
    membersViewport.setViewedComponent (&membersList, false);
    membersViewport.setScrollBarsShown (true, false);
    addAndMakeVisible (membersViewport);

    addAndMakeVisible (mixerHeading);
    addAndMakeVisible (prevBank);
    addAndMakeVisible (nextBank);
    addAndMakeVisible (resetButton);
    stripsViewport.setViewedComponent (&stripsHost, false);
    stripsViewport.setScrollBarsShown (false, true);
    addAndMakeVisible (stripsViewport);
    emptyLabel.setColour (Label::textColourId, SlimUi::textDim());
    emptyLabel.setJustificationType (Justification::centred);
    addAndMakeVisible (emptyLabel);
    footerHint.setFont (SlimUi::displayRegular (12.0f));
    footerHint.setColour (Label::textColourId, SlimUi::textDim());
    addAndMakeVisible (footerHint);

    addAndMakeVisible (sourcesHeading);
    search.setTextToShowWhenEmpty ("Find a source", SlimUi::textDim());
    search.onTextChange = [this] { rebuildSources(); };
    addAndMakeVisible (search);
    filterAll.setClickingTogglesState (true);
    filterLocal.setClickingTogglesState (true);
    filterRemote.setClickingTogglesState (true);
    filterAll.setRadioGroupId (7701);
    filterLocal.setRadioGroupId (7701);
    filterRemote.setRadioGroupId (7701);
    filterAll.setToggleState (true, dontSendNotification);
    filterAll.onClick = [this] { filterMode = 0; rebuildSources(); };
    filterLocal.onClick = [this] { filterMode = 1; rebuildSources(); };
    filterRemote.onClick = [this] { filterMode = 2; rebuildSources(); };
    addAndMakeVisible (filterAll);
    addAndMakeVisible (filterLocal);
    addAndMakeVisible (filterRemote);
    sourcesViewport.setViewedComponent (&sourcesList, false);
    sourcesViewport.setScrollBarsShown (true, false);
    addAndMakeVisible (sourcesViewport);
    addAndMakeVisible (addSelectedButton);

    controlledBy.setFont (SlimUi::displayRegular (12.0f));
    controlledBy.setColour (Label::textColourId, SlimUi::accentBlue());
    addAndMakeVisible (controlledBy);

    newGroupButton.onClick = [this] { addGroupPressed(); };
    manageMembersButton.onClick = [this] { manageMembersPressed(); };
    addSelectedButton.onClick = [this] { addSelectedSources(); };
    resetButton.onClick = [this] { resetLevels(); };
    prevBank.onClick = [this] { bankPage = jmax (0, bankPage - 1); rebuildStrips(); };
    nextBank.onClick = [this] { ++bankPage; rebuildStrips(); };

    processor.addClientListener (this);
    startTimerHz (8);
}

RemoteControlPage::~RemoteControlPage()
{
    stopTimer();
    processor.removeClientListener (this);
}

void RemoteControlPage::visibilityChanged()
{
    if (isVisible())
        refreshFromProcessor();
}

void RemoteControlPage::timerCallback()
{
    if (! isVisible())
        return;

    processor.ensureDefaultRemoteMixGroup();

    if (processor.isBeingRemoteMixControlled())
        controlledBy.setText ("Being controlled by " + processor.getRemoteMixControllerName(), dontSendNotification);
    else
        controlledBy.setText ({}, dontSendNotification);

    const auto now = Time::getMillisecondCounter();
    if (now - lastQueryMs > 2000)
        querySelectedMembers();

    rebuildGroups();
}

void RemoteControlPage::refreshFromProcessor()
{
    processor.ensureDefaultRemoteMixGroup();
    rebuildGroups();
    rebuildSources();
    rebuildStrips();
    querySelectedMembers();
}

RemoteMix::ControlGroup RemoteControlPage::currentGroup() const
{
    return processor.getRemoteMixControlGroup (processor.getSelectedRemoteMixControlGroup());
}

StringArray RemoteControlPage::currentTargets() const
{
    return currentGroup().members;
}

void RemoteControlPage::selectGroup (int index)
{
    processor.setSelectedRemoteMixControlGroup (index);
    bankPage = 0;
    confirmed.clear();
    rebuildGroups();
    rebuildSources();
    rebuildStrips();
    querySelectedMembers();
}

void RemoteControlPage::querySelectedMembers()
{
    lastQueryMs = Time::getMillisecondCounter();
    for (auto& m : currentTargets())
        processor.requestRemoteMixState (m);
}

void RemoteControlPage::sendStripChange (const String& sourceId, float gain, bool muted, bool live)
{
    const auto now = Time::getMillisecondCounter();
    if (live && now - lastLiveMs < (uint32) (1000 / RemoteMix::liveHz))
        return;
    if (live)
        lastLiveMs = now;

    if (! live)
        for (auto* strip : strips)
            if (strip->sourceId == sourceId)
                strip->setPending (true);

    processor.sendRemoteMixControl (currentTargets(), sourceId, gain, muted, live);
}

void RemoteControlPage::applySnapshot (const Array<RemoteMix::SourceState>& sources)
{
    for (auto& s : sources)
        confirmed[s.id] = s;

    for (auto* strip : strips)
    {
        if (strip->isPending())
            continue;
        auto it = confirmed.find (strip->sourceId);
        if (it != confirmed.end())
            strip->setConfirmed (it->second.gain, it->second.muted);
    }
}

void RemoteControlPage::restoreStrip (const String& sourceId)
{
    auto it = confirmed.find (sourceId);
    for (auto* strip : strips)
    {
        if (strip->sourceId != sourceId)
            continue;
        strip->setPending (false);
        if (it != confirmed.end())
            strip->setConfirmed (it->second.gain, it->second.muted);
    }
}

void RemoteControlPage::rebuildGroups()
{
    const int n = processor.getRemoteMixControlGroupCount();
    while (groupCards.size() < n)
    {
        auto* card = groupCards.add (new GroupCard());
        groupsList.addAndMakeVisible (card);
        const int idx = groupCards.size() - 1;
        card->onSelect = [this, idx] { selectGroup (idx); };
    }
    while (groupCards.size() > n)
        groupCards.removeLast();

    int y = 0;
    const int sel = processor.getSelectedRemoteMixControlGroup();
    for (int i = 0; i < n; ++i)
    {
        auto g = processor.getRemoteMixControlGroup (i);
        int online = 0;
        for (auto& m : g.members)
            if (processor.getRemoteMixMemberStatus (m) != RemoteMix::MemberStatus::Unreachable)
                ++online;
        groupCards[i]->setGroup (g, i == sel, online);
        groupCards[i]->onSelect = [this, i] { selectGroup (i); };
        groupCards[i]->setBounds (0, y, jmax (1, groupsViewport.getMaximumVisibleWidth() - 4), 56);
        y += 62;
    }
    groupsList.setSize (jmax (1, groupsViewport.getMaximumVisibleWidth() - 4), jmax (y, 10));

    auto g = currentGroup();
    membersHeading.setText (g.name.isNotEmpty() ? g.name + " members" : "Members", dontSendNotification);
    while (memberLabels.size() < g.members.size())
    {
        auto* lab = memberLabels.add (new Label());
        lab->setFont (SlimUi::displayRegular (13.0f));
        membersList.addAndMakeVisible (lab);
    }
    while (memberLabels.size() > g.members.size())
        memberLabels.removeLast();

    int my = 0;
    for (int i = 0; i < g.members.size(); ++i)
    {
        const auto st = processor.getRemoteMixMemberStatus (g.members[i]);
        String extra;
        if (st == RemoteMix::MemberStatus::Unreachable)
            extra = "  unreachable";
        else if (st == RemoteMix::MemberStatus::Pending)
            extra = "  syncing";
        memberLabels[i]->setText (g.members[i] + extra, dontSendNotification);
        memberLabels[i]->setColour (Label::textColourId,
                                    st == RemoteMix::MemberStatus::Unreachable ? Colour (0xffC4453C) : SlimUi::text());
        memberLabels[i]->setBounds (0, my, jmax (1, membersViewport.getMaximumVisibleWidth() - 4), 22);
        my += 24;
    }
    membersList.setSize (jmax (1, membersViewport.getMaximumVisibleWidth() - 4), jmax (my, 10));

    footerHint.setText ("Levels control what " + (g.name.isNotEmpty() ? g.name : String ("the group"))
                            + " receives from each source.",
                        dontSendNotification);
}

void RemoteControlPage::rebuildSources()
{
    auto all = processor.getAvailableRemoteMixSources();
    auto g = currentGroup();
    const String q = search.getText().trim().toLowerCase();

    Array<RemoteMix::SourceState> filtered;
    for (auto& s : all)
    {
        if (q.isNotEmpty() && ! s.name.toLowerCase().contains (q) && ! s.id.toLowerCase().contains (q))
            continue;
        if (filterMode == 1 && s.kind == "peer")
            continue;
        if (filterMode == 2 && s.kind != "peer")
            continue;
        filtered.add (s);
    }

    while (sourceRows.size() < filtered.size())
    {
        auto* row = sourceRows.add (new SourceRow());
        sourcesList.addAndMakeVisible (row);
    }
    while (sourceRows.size() > filtered.size())
        sourceRows.removeLast();

    int y = 0;
    for (int i = 0; i < filtered.size(); ++i)
    {
        sourceRows[i]->setSource (filtered[i], g.sourceIds.contains (filtered[i].id));
        sourceRows[i]->setBounds (0, y, jmax (1, sourcesViewport.getMaximumVisibleWidth() - 4), 32);
        y += 36;
    }
    sourcesList.setSize (jmax (1, sourcesViewport.getMaximumVisibleWidth() - 4), jmax (y, 10));
}

void RemoteControlPage::rebuildStrips()
{
    auto g = currentGroup();
    const int total = g.sourceIds.size();
    const int pages = jmax (1, (total + kBankSize - 1) / kBankSize);
    bankPage = jlimit (0, pages - 1, bankPage);
    const int start = bankPage * kBankSize;
    const int count = jmin (kBankSize, total - start);

    prevBank.setButtonText ("< Inputs " + String (start + 1) + "-" + String (start + jmax (count, 1)));
    nextBank.setButtonText ("Inputs " + String (jmin (total, start + kBankSize + 1)) + "-"
                            + String (jmin (total, start + kBankSize * 2)) + " >");
    nextBank.setEnabled (bankPage < pages - 1);
    prevBank.setEnabled (bankPage > 0);

    auto available = processor.getAvailableRemoteMixSources();
    std::map<String, RemoteMix::SourceState> byId;
    for (auto& s : available)
        byId[s.id] = s;

    while (strips.size() < count)
    {
        auto* strip = strips.add (new MixStrip (*this, mixerLnf));
        stripsHost.addAndMakeVisible (strip);
    }
    while (strips.size() > count)
        strips.removeLast();

    mixerHeading.setText ("Remote Levels · " + g.name, dontSendNotification);

    if (processor.getNumberRemotePeers() <= 0)
        emptyLabel.setText ("Connect to a group to control someone's mix.", dontSendNotification);
    else if (g.members.isEmpty())
        emptyLabel.setText ("Add members to " + g.name + " to control their mix.", dontSendNotification);
    else
        emptyLabel.setText ("Add sources from the right to build this board.", dontSendNotification);

    emptyLabel.setVisible (count <= 0);

    const int h = jmax (420, stripsViewport.getHeight());
    for (int i = 0; i < count; ++i)
    {
        const auto id = g.sourceIds[start + i];
        RemoteMix::SourceState src;
        src.id = id;
        src.name = RemoteMix::isPeerSource (id) ? RemoteMix::peerNameFromId (id) : id;
        src.kind = RemoteMix::isPeerSource (id) ? "peer" : "local";

        // Name and kind come from what we can see locally, but gain/mute must always
        // reflect the target's confirmed mix, never our own copy of that source.
        auto local = byId.find (id);
        if (local != byId.end())
        {
            src.name = local->second.name;
            src.kind = local->second.kind;
        }

        auto conf = confirmed.find (id);
        if (conf != confirmed.end())
        {
            src.gain = conf->second.gain;
            src.muted = conf->second.muted;
            if (conf->second.name.isNotEmpty())
                src.name = conf->second.name;
        }

        strips[i]->setSource (src, start + i);
        if (RemoteMix::isPeerSource (id))
        {
            const auto uname = RemoteMix::peerNameFromId (id);
            for (int p = 0; p < processor.getNumberRemotePeers(); ++p)
                if (processor.getRemotePeerUserName (p) == uname)
                {
                    strips[i]->setMeterSource (processor.getRemotePeerRecvMeterSource (p));
                    break;
                }
        }
        else if (RemoteMix::isLocalSource (id))
        {
            strips[i]->setMeterSource (&processor.getPostInputMeterSource());
        }
        else if (id == RemoteMix::fileSourceId())
            strips[i]->setMeterSource (&processor.getFilePlaybackMeterSource());
        else if (id == RemoteMix::soundboardSourceId() && processor.getSoundboardProcessor())
            strips[i]->setMeterSource (&processor.getSoundboardProcessor()->getMeterSource());

        strips[i]->setBounds (8 + i * (kStripW + 8), 8, kStripW, h - 16);
    }
    stripsHost.setSize (jmax (stripsViewport.getMaximumVisibleWidth(), 16 + count * (kStripW + 8)), h);
}

void RemoteControlPage::addGroupPressed()
{
    auto* aw = new AlertWindow ("New Group", "Name this mix group", AlertWindow::NoIcon);
    aw->addTextEditor ("name", "Band Mix");
    aw->addButton ("Create", 1, KeyPress (KeyPress::returnKey));
    aw->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
    aw->enterModalState (true, ModalCallbackFunction::create ([this, aw] (int r)
    {
        if (r == 1)
            processor.addRemoteMixControlGroup (aw->getTextEditorContents ("name"));
        refreshFromProcessor();
    }), true);
}

void RemoteControlPage::manageMembersPressed()
{
    auto g = currentGroup();
    StringArray names;
    for (int i = 0; i < processor.getNumberRemotePeers(); ++i)
        names.add (processor.getRemotePeerUserName (i));

    auto* aw = new AlertWindow ("Manage Members", "Comma-separated usernames", AlertWindow::NoIcon);
    aw->addTextEditor ("members", g.members.joinIntoString (", "));
    aw->addButton ("Save", 1, KeyPress (KeyPress::returnKey));
    aw->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
    if (names.size() > 0)
        aw->addButton ("All connected", 2);
    aw->enterModalState (true, ModalCallbackFunction::create ([this, aw, names] (int r)
    {
        auto g2 = currentGroup();
        if (r == 2)
            processor.setRemoteMixControlGroupMembers (g2.id, names);
        else if (r == 1)
        {
            auto text = aw->getTextEditorContents ("members");
            StringArray mem;
            mem.addTokens (text, ",", "\"");
            mem.trim();
            mem.removeEmptyStrings();
            processor.setRemoteMixControlGroupMembers (g2.id, mem);
        }
        refreshFromProcessor();
    }), true);
}

void RemoteControlPage::addSelectedSources()
{
    auto g = currentGroup();
    for (auto* row : sourceRows)
        if (row->check.getToggleState())
            g.sourceIds.addIfNotAlreadyThere (row->sourceId);
    processor.setRemoteMixControlGroupSources (g.id, g.sourceIds);
    rebuildStrips();
    rebuildSources();
}

void RemoteControlPage::resetLevels()
{
    auto g = currentGroup();
    for (auto& id : g.sourceIds)
        processor.sendRemoteMixControl (g.members, id, 1.0f, false, false);
}

void RemoteControlPage::remoteMixStateReceived (SonobusAudioProcessor*, const String& fromUser,
                                                const Array<RemoteMix::SourceState>& sources)
{
    MessageManager::callAsync ([safe = SafePointer<RemoteControlPage> (this), fromUser, sources]()
    {
        if (safe == nullptr)
            return;
        safe->lastSnapshotFrom = fromUser;
        safe->applySnapshot (sources);
    });
}

void RemoteControlPage::remoteMixControlAck (SonobusAudioProcessor*, const String&,
                                             const String& sourceId, float gain, bool muted)
{
    MessageManager::callAsync ([safe = SafePointer<RemoteControlPage> (this), sourceId, gain, muted]()
    {
        if (safe == nullptr)
            return;
        auto& dst = safe->confirmed[sourceId];
        dst.id = sourceId;
        dst.gain = gain;
        dst.muted = muted;
        for (auto* strip : safe->strips)
            if (strip->sourceId == sourceId)
                strip->setConfirmed (gain, muted);
    });
}

void RemoteControlPage::remoteMixControlFailed (SonobusAudioProcessor*, const String&,
                                                const String& sourceId, int)
{
    MessageManager::callAsync ([safe = SafePointer<RemoteControlPage> (this), sourceId]()
    {
        if (safe != nullptr)
            safe->restoreStrip (sourceId);
    });
}

void RemoteControlPage::remoteMixBeingControlled (SonobusAudioProcessor*, const String& fromUser)
{
    MessageManager::callAsync ([safe = SafePointer<RemoteControlPage> (this), fromUser]()
    {
        if (safe != nullptr)
            safe->controlledBy.setText ("Being controlled by " + fromUser, dontSendNotification);
    });
}

void RemoteControlPage::aooClientPeerJoined (SonobusAudioProcessor*, const String&, const String&)
{
    MessageManager::callAsync ([safe = SafePointer<RemoteControlPage> (this)]()
    {
        if (safe != nullptr)
            safe->refreshFromProcessor();
    });
}

void RemoteControlPage::aooClientPeerLeft (SonobusAudioProcessor*, const String&, const String&)
{
    MessageManager::callAsync ([safe = SafePointer<RemoteControlPage> (this)]()
    {
        if (safe != nullptr)
            safe->refreshFromProcessor();
    });
}

void RemoteControlPage::paint (Graphics& g)
{
    g.fillAll (SlimUi::mixerWell());
}

void RemoteControlPage::resized()
{
    auto bounds = getLocalBounds().reduced (12);

    auto left = bounds.removeFromLeft (220);
    bounds.removeFromLeft (10);
    auto right = bounds.removeFromRight (240);
    bounds.removeFromRight (10);

    groupsHeading.setBounds (left.removeFromTop (24));
    auto gbtns = left.removeFromTop (28);
    newGroupButton.setBounds (gbtns.removeFromLeft (gbtns.getWidth() / 2).reduced (0, 2));
    manageMembersButton.setBounds (gbtns.reduced (4, 2));
    left.removeFromTop (6);
    membersHeading.setBounds (left.removeFromBottom (22));
    membersViewport.setBounds (left.removeFromBottom (jmin (120, left.getHeight() / 3)));
    left.removeFromBottom (6);
    groupsViewport.setBounds (left);

    sourcesHeading.setBounds (right.removeFromTop (24));
    search.setBounds (right.removeFromTop (28));
    right.removeFromTop (6);
    auto filt = right.removeFromTop (26);
    const int fw = filt.getWidth() / 3;
    filterAll.setBounds (filt.removeFromLeft (fw).reduced (1, 0));
    filterLocal.setBounds (filt.removeFromLeft (fw).reduced (1, 0));
    filterRemote.setBounds (filt.reduced (1, 0));
    right.removeFromTop (6);
    addSelectedButton.setBounds (right.removeFromBottom (36));
    right.removeFromBottom (8);
    sourcesViewport.setBounds (right);

    auto top = bounds.removeFromTop (28);
    mixerHeading.setBounds (top.removeFromLeft (jmax (160, top.getWidth() - 360)));
    resetButton.setBounds (top.removeFromRight (110));
    top.removeFromRight (8);
    nextBank.setBounds (top.removeFromRight (120));
    top.removeFromRight (6);
    prevBank.setBounds (top.removeFromRight (120));
    bounds.removeFromTop (8);
    controlledBy.setBounds (bounds.removeFromTop (18));
    footerHint.setBounds (bounds.removeFromBottom (22));
    bounds.removeFromBottom (6);
    stripsViewport.setBounds (bounds);
    emptyLabel.setBounds (bounds);
    rebuildStrips();
}
