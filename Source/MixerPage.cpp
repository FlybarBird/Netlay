// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#include "MixerPage.h"
#include "ChannelGroupsView.h"

MixerPage::MixerPage (ChannelGroupsView& mixerView,
                      Component& sendChoice,
                      Component& addGroupButton,
                      Component* muteButton,
                      Component* soloButton)
    : mixer (mixerView),
      sendChannels (sendChoice),
      addGroup (addGroupButton),
      inputMute (muteButton),
      inputSolo (soloButton)
{
    heading.setFont (SlimUi::displayBold (18.0f));
    heading.setColour (Label::textColourId, SlimUi::text());
    heading.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (heading);

    addAndMakeVisible (sendChannels);
    addAndMakeVisible (addGroup);

    if (inputMute != nullptr)
        addAndMakeVisible (inputMute);
    if (inputSolo != nullptr)
        addAndMakeVisible (inputSolo);

    mixerViewport.setViewedComponent (&mixer, false);
    mixerViewport.setScrollBarsShown (true, true);
    mixerViewport.setScrollOnDragEnabled (false);
    addAndMakeVisible (mixerViewport);
}

void MixerPage::visibilityChanged()
{
    if (isVisible())
    {
        mixer.setVisible (true);
        mixer.rebuildChannelViews();
        layoutMixer();
    }
}

void MixerPage::paint (Graphics& g)
{
    g.fillAll (SlimUi::mixerWell());

    auto top = getLocalBounds().removeFromTop (52);
    g.setColour (SlimUi::header());
    g.fillRect (top);
    g.setColour (SlimUi::outline());
    g.fillRect (top.removeFromBottom (1));
}

void MixerPage::resized()
{
    layoutMixer();
}

void MixerPage::layoutMixer()
{
    auto bounds = getLocalBounds();

    auto top = bounds.removeFromTop (52).reduced (16, 8);
    addGroup.setBounds (top.removeFromRight (188));
    top.removeFromRight (10);
    sendChannels.setBounds (top.removeFromRight (168));

    if (inputSolo != nullptr)
    {
        top.removeFromRight (8);
        inputSolo->setBounds (top.removeFromRight (72));
    }
    if (inputMute != nullptr)
    {
        top.removeFromRight (8);
        inputMute->setBounds (top.removeFromRight (72));
    }

    heading.setBounds (top);

    mixerViewport.setBounds (bounds);

    const int viewH = jmax (1, mixerViewport.getMaximumVisibleHeight());
    mixer.setEstimatedHeight (viewH);
    mixer.updateLayout (false);

    const auto minBounds = mixer.getMinimumContentBounds();
    const int viewW = jmax (1, mixerViewport.getMaximumVisibleWidth());
    mixer.setBounds (0, 0,
                     jmax (viewW, minBounds.getWidth()),
                     jmax (viewH, minBounds.getHeight()));
}
