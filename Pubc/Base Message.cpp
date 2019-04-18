/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Conduits/Pubc/Base Message.h"
#include "Conduits/Pubc/Path Tools.h"

using namespace Conduits;

    //  Setup
    // --------------------

IBaseMessage::IBaseMessage()
{
    this->Message_State     = Conduits::MessageState::pending;
    this->Response_Desire   = Conduits::ResponseDesire::not_needed;
}

    //  Sender
    // --------------------

void IBaseMessage::sender_prepare_for_send()
{
        // Santise path and expose to c-like variable
    this->Path = Util::Sanitise_Path(this->Path);
    this->Adapting_Path = this->Path.c_str();
}

    //  Receiver
    // --------------------

IBaseMessage::ResDes IBaseMessage::get_response_expectation() noexcept
{
    return this->Response_Desire;
}

const char * IBaseMessage::get_path_string() const noexcept
{
    return this->Path.c_str();
}

const IBaseMessage::SegmentRef IBaseMessage::get_message_segment(SegIndex index) const noexcept
{
    const auto & seg = this->Message_Segments.find(index);
    if (seg == this->Message_Segments.end()) {
        return { 0, nullptr };
    }
    return { seg->second.size(), (void*)seg->second.c_str() };
}

bool IBaseMessage::set_response_string_with_copy(const char * str) noexcept
{
    this->Response_String = str;
    return true;
}

bool IBaseMessage::set_response_segment_with_copy(SegIndex index, const SegmentRef ref) noexcept
{
    auto & seg = this->Response_Segments[index];
    seg.assign((char*)ref.Data, ref.Length);
    return true;
}

const char * IBaseMessage::get_response_string() const noexcept
{
    return this->Response_String.c_str();
}

const IBaseMessage::SegmentRef IBaseMessage::get_response_segment(SegIndex index) const noexcept 
{
    const auto & seg = this->Response_Segments.find(index);
    if (seg == this->Response_Segments.end()) {
        return { 0, nullptr };
    }
    return { seg->second.size(), (void*)seg->second.c_str() };
}

void IBaseMessage::set_OK() noexcept
{
    this->Message_State = Conduits::MessageState::ok;
}

void IBaseMessage::set_FAILED() noexcept
{
    this->Message_State = Conduits::MessageState::failed;
}

void IBaseMessage::set_FAILED_connexion() noexcept
{
    this->Message_State = Conduits::MessageState::connexion_failure;
}