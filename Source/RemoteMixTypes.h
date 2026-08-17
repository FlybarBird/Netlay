// SPDX-License-Identifier: GPLv3-or-later WITH Appstore-exception

#pragma once

#include <JuceHeader.h>

namespace RemoteMix
{
    constexpr int protoVer = 1;
    constexpr int muteFlag = 1;
    constexpr int maxRetries = 8;
    constexpr int liveHz = 25;

    enum class Kind
    {
        Set = 0,
        Query,
        State
    };

    enum class NackReason
    {
        Denied = 1,
        UnknownSource = 2,
        BadVer = 3
    };

    enum class MemberStatus
    {
        Ok = 0,
        Pending,
        Unreachable
    };

    struct SourceState
    {
        String id;
        String name;
        String kind;
        float gain = 1.0f;
        bool muted = false;
    };

    struct ControlGroup
    {
        String id;
        String name;
        StringArray members;
        StringArray sourceIds;
    };

    inline String peerSourceId (const String& user) { return "peer:" + user; }
    inline String localSourceId (int index) { return "local:" + String (index); }
    inline String fileSourceId() { return "file"; }
    inline String soundboardSourceId() { return "soundboard"; }

    inline bool isPeerSource (const String& id) { return id.startsWith ("peer:"); }
    inline bool isLocalSource (const String& id) { return id.startsWith ("local:"); }
    inline String peerNameFromId (const String& id) { return id.fromFirstOccurrenceOf (":", false, false); }
    inline int localIndexFromId (const String& id) { return id.fromFirstOccurrenceOf (":", false, false).getIntValue(); }
}
