/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>

#include "Conduits/Pubc/Interface Raw Message.h"

namespace Conduits {

    class IBaseMessage : public Raw::IRelayMessage {
    protected:
        using SegIndex    = Raw::SegIndex;
        using SegmentData = std::string;
        using SegmentMap  = std::map<SegIndex, SegmentData>;
        using ResDes      = ResponseDesire::Type;
        using State       = MessageState::Type;
        using SegmentRef  = Raw::SegmentData;

    public:
        std::string  Path;
        
        std::string  Message_String;
        std::string  Response_String;

        SegmentMap   Message_Segments;
        SegmentMap   Response_Segments;

        State        Message_State;
        ResDes       Response_Desire;

        IBaseMessage();
        virtual ~IBaseMessage() { ; }

        void             sender_prepare_for_send();

            // Implementation of message handle functions

        ResDes           get_response_expectation() noexcept override;

        const char *     get_path_string() const noexcept override;
        const SegmentRef get_message_segment(SegIndex index) const noexcept override;

        bool             set_response_string_with_copy(const char*) noexcept override;
        bool             set_response_segment_with_copy(SegIndex index, const SegmentRef) noexcept override;

        const char *     get_response_string() const noexcept override;
        const SegmentRef get_response_segment(SegIndex index) const noexcept override;

        void             set_OK() noexcept override;
        void             set_FAILED() noexcept override;
        void             set_FAILED_connexion() noexcept override;
    };

}