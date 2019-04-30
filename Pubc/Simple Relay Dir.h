/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>
#include <map>
#include <functional>

#include "Conduits/Pubc/Interface Relay Receiver.h"
#include "Conduits/Pubc/Interface Raw Message.h"

namespace Conduits {

    struct SimpleRelayDir {
        using RMRDir    = IRelayMessageReceiver::RMRDir;
        using CallType  = std::function<IRelayMessageReceiver::MessageHandleFunc>;

        std::map<std::string, CallType> Call_Map;

        bool try_relay_immediate(const char * path, Raw::IMessage * msg);
        void dir_relay(RMRDir & dir);
    };

    inline bool SimpleRelayDir::try_relay_immediate(const char * path, Raw::IMessage * msg)
    {
        auto & it = this->Call_Map.find(path);
        if (it == this->Call_Map.end())
            return false;
        it->second(msg);
        return true;
    }

    inline void SimpleRelayDir::dir_relay(RMRDir & dir)
    {
        for (auto & it : this->Call_Map) {
            dir.push_back(it.first);
        }
    }

}