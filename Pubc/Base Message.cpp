/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
 
#include "BlackRoot/Pubc/Exception.h"

#include "Conduits/Pubc/Base Message.h"
#include "Conduits/Pubc/Path Tools.h"

using namespace Conduits;

    //  Sender
    // --------------------

void IBaseMessage::add_message_segment(SegmentData data)
{
    this->Segment_Map[data.Name].assign((char*)data.Data, data.Length);
}

void IBaseMessage::add_message_segments_from_list(const SegmentList & list)
{
    for (const auto & elem : list) {
        this->add_message_segment(elem);
    }
}

void IBaseMessage::set_message_string_as_path(std::string str)
{
    this->Message_String    = Util::Sanitise_Path(str);
}

void IBaseMessage::sender_prepare_for_send()
{
    this->IMessage::Message_String = this->Message_String.c_str();
    this->Adapting_String          = this->Message_String.c_str();
}

    //  Receiver
    // --------------------
    
IBaseMessage::RespDesire IBaseMessage::get_response_expectation() noexcept
{
    return this->Response_Desire;
}

const IBaseMessage::SegmentData IBaseMessage::get_message_segment(const char * name) const noexcept
{
        // Ensure name is not a nullptr
    name = name ? name : "";
    
        // Find the segment and return its info, if existing
    SegmentData data;

    const auto & seg = this->Segment_Map.find(name);
    if (seg == this->Segment_Map.end()) {
        data.Name   = nullptr;
        data.Data   = nullptr;
        data.Length = 0;
        return data;
    }

    data.Name   = seg->first.c_str();
    data.Data   = (void*)seg->second.c_str();
    data.Length = seg->second.size();
    return data;
}

size_t IBaseMessage::get_message_segment_count() const noexcept
{
    return this->Segment_Map.size();
}

size_t IBaseMessage::get_message_segment_list(SegmentData * out_segments, size_t max_segments) const noexcept
{
    size_t count = 0;

    for (auto & elem : this->Segment_Map) {
        if (count >= max_segments)
            return count;
            
        out_segments[count].Name    = elem.first.c_str();
        out_segments[count].Data    = (void*)elem.second.c_str();
        out_segments[count].Length  = elem.second.size();
    }

    return count;
}

void IBaseMessage::set_OK(OKState state) noexcept
{
    this->Message_State = MessageState::ok;
    this->State_OK = state;
}

void IBaseMessage::set_FAILED(FailState state) noexcept
{
    this->Message_State = MessageState::failed;
    this->State_Fail = state;
}

void IBaseMessage::set_response(IMessage * msg) noexcept
{
    if (this->Response_Desire == Raw::ResponseDesire::not_allowed) {
        msg->set_FAILED(Raw::FailState::failed_no_response_expected);
        msg->release();
        return;
    }

    this->Response = msg;
}

    //  Utility
    // --------------------

bool IBaseMessage::result_is_OK()
{
    return this->Message_State == MessageState::ok;
}