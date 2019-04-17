/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/JSON.h"
#include "Conduits/Pubc/Interface Raw Message.h"
#include "Conduits/Pubc/Message State.h"

namespace Conduits {

    class RelayMessageJSONWrap {
    public:
        using JSON   = BlackRoot::Format::JSON;
        using String = std::string;

    protected:
        Raw::IRelayMessage   *Internal_Message;
        
        RelayMessageJSONWrap(Raw::IRelayMessage*);
    public:
        virtual ~RelayMessageJSONWrap() { ; }

        JSON    Message;
        JSON    Response;

        void    get_message_segment(Raw::SegIndex, Raw::SegmentData&);
        void    set_response_segment_with_copy(Raw::SegIndex, const Raw::SegmentData);
        void    connect_sender_to_circuit(IConduit*);
        
        void    set_OK();
        void    set_FAILED();

        virtual void release();

        MessageState::Type get_internal_state();

        static RelayMessageJSONWrap wrap_around(Raw::IRelayMessage*);
        static RelayMessageJSONWrap wrap_around_safe(Raw::IRelayMessage*) noexcept;
    };

}