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

    class IBaseMessage : public Raw::IMessage {
    public:
        struct MessageState {
            using Type = uint8;
            enum : Type {
                pending,
                ok,
                failed
            };
        };
        
        using SegmentData    = Raw::SegmentData;

        using SegmentName    = std::string;
        using SegmentIndex   = uint32;
        using SegmentMap     = std::map<SegmentName, std::string>;
        using SegmentList    = std::vector<SegmentData>;

        using RespDesire     = Raw::ResponseDesire::Type;
        using OKState        = Raw::OKState::Type;
        using FailState      = Raw::FailState::Type;

    public:
        std::string         Message_String;

        SegmentMap          Segment_Map;

        RespDesire          Response_Desire;

        MessageState::Type  Message_State;
        OKState             State_OK;
        FailState           State_Fail;

        Raw::IMessage      *Response;

        IBaseMessage()
        : Response_Desire(Raw::ResponseDesire::not_allowed),
          Message_State(MessageState::pending),
          Response(nullptr)
        { }
        virtual ~IBaseMessage() { ; }
        
        // --- Sender utility

        virtual void     set_message_string_as_path(std::string);
        
        virtual void     add_message_segment(SegmentData);
        virtual void     add_message_segments_from_list(const SegmentList &);

        void             sender_prepare_for_send();
        
        // --- Implementation of IMessage
        
        RespDesire get_response_expectation() noexcept override;
        
        const SegmentData get_message_segment(char*) const noexcept override;

        size_t get_message_segment_count() const noexcept override;
        size_t get_message_segment_list(SegmentData * out_segments, size_t max_segments) const noexcept override;

        void set_OK(OKState) noexcept override;
        void set_FAILED(FailState) noexcept override;

        void add_response(IMessage *) noexcept override;
    };

}