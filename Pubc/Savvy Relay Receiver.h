/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Conduits/Pubc/Interface Relay Receiver.h"

namespace Conduits {

    class SavvyRelayMessageReceiver : public IRelayMessageReceiver {
        CON_RMR_DECLARE_CLASS(SavvyRelayMessageReceiver, IRelayMessageReceiver)

    public:        
        virtual std::string html_create_action_relay_string();

        virtual void savvy_handle_http(const JSON httpRequest, JSON & httpReply, std::string & outBody);
        virtual void savvy_handle_http_call(const std::string call, Raw::IRelayMessage *, const JSON httpRequest, JSON & httpReply, std::string & outBody);
        
        CON_RMR_DECLARE_FUNC(dir);
        CON_RMR_DECLARE_FUNC(http);
    };

}