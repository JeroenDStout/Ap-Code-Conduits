﻿/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/Assert.h"
#include "BlackRoot/Pubc/JSON.h"
#include "BlackRoot/Pubc/Number Types.h"
#include "BlackRoot/Pubc/Exception.h"

#include "Conduits/Pubc/Websocket Protocol Shared.h"
#include "Conduits/Pubc/Interface Raw Message.h"

namespace Conduits {
namespace Protocol {

    struct StateFlags {
        using Type = uint8;
        
        static const Type   Has_String               = 0x01;
        static const Type   Has_Segments             = 0x02;
        static const Type   Is_Response              = 0x04;

            // if response...
        static const Type   Is_OK                    = 0x08;

            // if response is OK...
        static const Type   Confirm_Opened_Conduit   = 0x10;

            // if response not OK...
        static const Type   Connexion_Failure        = 0x10;
        static const Type   No_response_Expected     = 0x20;
        static const Type   Timed_Out                = 0x40;
        static const Type   Receiver_Will_Not_Handle = 0x80;

            // if not response...
        static const Type   Accepts_Response         = 0x08;
        static const Type   Ping_Conduit             = 0x10;
        static const Type   Close_Conduit            = 0x20;
    };

        // ------ Message

    struct MessageScratch {
        using OKState       = Raw::OKState::Type;
        using FailState     = Raw::FailState::Type;

        using SegmentElem   = Raw::SegmentData;
        using SegmentList   = std::vector<SegmentElem>;

        StateFlags::Type  Flags;

        uint32  Recipient_ID;
        uint32  Reply_To_Me_ID;
        uint32  Opened_Conduit_ID;

        const char        *String;
        uint16            String_Length;

        SegmentList       Segments;

        MessageScratch()
        : Flags(0x0), String_Length(0)
        { }
        
        void set_is_response(bool b) {
            if (b) { this->Flags |= StateFlags::Is_Response; }
            else { this->Flags &= ~StateFlags::Is_Response; }
        }
        void set_is_OK(bool b) {
            if (b) { this->Flags |= StateFlags::Is_OK; }
            else { this->Flags &= ~StateFlags::Is_OK; }
        }
        void set_confirm_open_conduit(bool b) {
            if (b) { this->Flags |= StateFlags::Confirm_Opened_Conduit; }
            else { this->Flags &= ~StateFlags::Confirm_Opened_Conduit; }
        }
        void set_accepts_response(bool b) {
            if (b) { this->Flags |= StateFlags::Accepts_Response; }
            else { this->Flags &= ~StateFlags::Accepts_Response; }
        }
        void set_connexion_failure(bool b) {
            if (b) { this->Flags |= StateFlags::Connexion_Failure; }
            else { this->Flags &= ~StateFlags::Connexion_Failure; }
        }
        void set_no_response_expected(bool b) {
            if (b) { this->Flags |= StateFlags::No_response_Expected; }
            else { this->Flags &= ~StateFlags::No_response_Expected; }
        }
        void set_timed_out(bool b) {
            if (b) { this->Flags |= StateFlags::Timed_Out; }
            else { this->Flags &= ~StateFlags::Timed_Out; }
        }
        void set_receiver_will_not_handle(bool b) {
            if (b) { this->Flags |= StateFlags::Receiver_Will_Not_Handle; }
            else { this->Flags &= ~StateFlags::Receiver_Will_Not_Handle; }
        }

        bool get_is_response()  const               { return 0 != (this->Flags & StateFlags::Is_Response); }
        bool get_is_OK() const                      { return 0 != (this->Flags & StateFlags::Is_OK); }
        bool get_confirm_open_conduit() const       { return 0 != (this->Flags & StateFlags::Confirm_Opened_Conduit); }
        bool get_accepts_response() const           { return 0 != (this->Flags & StateFlags::Accepts_Response); }
        bool get_connexion_failure() const          { return 0 != (this->Flags & StateFlags::Connexion_Failure); }
        bool get_no_response_expected() const       { return 0 != (this->Flags & StateFlags::No_response_Expected); }
        bool get_timed_out() const                  { return 0 != (this->Flags & StateFlags::Timed_Out); }
        bool get_receiver_will_not_handle() const   { return 0 != (this->Flags & StateFlags::Receiver_Will_Not_Handle); }
        
        void add_segments_from_message(Raw::IMessage*);

        static MessageScratch try_parse_message(void * msg, size_t length);
        static std::string    try_stringify_message(const MessageScratch &);
    };
    
    inline void MessageScratch::add_segments_from_message(Raw::IMessage * msg) {
        auto count = msg->get_message_segment_count();
        if (0 == count)
            return;
            
            // Make enough free space to add segments
        auto prev_size = this->Segments.size();
        this->Segments.resize(prev_size + count);

            // Directly set the segments in our list
        msg->get_message_segment_list(this->Segments.data() + prev_size, count);
    }

        // Implement Message from client

    inline MessageScratch MessageScratch::try_parse_message(void * _msg, size_t length)
    {
        MessageScratch ms;
        auto * msg = (char*)(_msg);

        ms.Flags = msg[0];                                  // Flags (1)
        msg += 1;

        ms.Recipient_ID = *(uint32*)(msg);                  // Recipient (4)
        msg += 4;

        if (ms.get_accepts_response()) {
            ms.Reply_To_Me_ID = *(uint32*)(msg);            // Response ID (4)
            msg += 4;
        }
        
        if (ms.get_confirm_open_conduit()) {
            ms.Opened_Conduit_ID = *(uint32*)(msg);         // Conduit ID (4)
            msg += 4;
        }

        if (0 != (ms.Flags & StateFlags::Has_String)) {
            ms.String = msg;                                // String value (n, ends with \0)
            ms.String_Length = (uint16)strlen(msg);
            msg += ms.String_Length + 1;
        }
        else {
            ms.String               = nullptr;
            ms.String_Length        = 0;
        }

        if (0 != (ms.Flags & StateFlags::Has_Segments)) {
            uint8  segmentCount     = msg[0];               // Segment count (1)
            msg += 1;
            
            for (uint8 seg = 0; seg < segmentCount; seg++) {
                SegmentElem elem;               
                
                elem.Name   = msg;                          // Segment name (n, ends with \0)
                msg += (uint16)strlen(elem.Name) + 1;

                elem.Length = *(uint32*)(msg);              // Segment length (4)
                msg += 4;

                elem.Data   = (void*)msg;                   // Segment data (n)
                msg += elem.Length;

                ms.Segments.push_back(elem);
            }
        }

        return ms;
    }

        // Implement message from server

    inline std::string MessageScratch::try_stringify_message(const MessageScratch & msg)
    {
        DbAssertFatal(msg.Segments.size() <= 0xFF);

        int required_size;
        StateFlags::Type flags = msg.Flags;

            // Calculate needed size

        required_size = 5;                                  // Flags + Recipient ID (5)

        if (msg.get_accepts_response()) {
            required_size += 4;                             // Response ID (4)
        }
        if (msg.get_confirm_open_conduit()) {
            required_size += 4;                             // Conduit ID (4)
        }
        
        if (0 != msg.String && msg.String_Length > 0) {
            flags |= StateFlags::Has_String;
            required_size += msg.String_Length + 1;         // String value ending in \0 (n + 1)
        }

        if (msg.Segments.size() > 0) {
            flags |= StateFlags::Has_Segments;
            required_size += 1;                             // Segment count (1)
            for (const auto & seg : msg.Segments) {
                DbAssertFatal(seg.Length <= 0xFFFFFFFF);
                required_size += strlen(seg.Name) + 1;      // String name ending in \0 (n + 1)
                required_size += 4;                         // Segment size (4)
                required_size += seg.Length;                // Segment data (n)
            }
        }

            // Write message

        std::string ret;
        ret.reserve(required_size);

        ret.append((char*)&flags, 1);                       // Flags (1)

        ret.append((char*)&msg.Recipient_ID, 4);            // Recipient ID (4)
        
        if (msg.get_accepts_response()) {
            ret.append((char*)&msg.Reply_To_Me_ID, 4);      // Response ID (4)
        }
        if (msg.get_confirm_open_conduit()) {
            ret.append((char*)&msg.Opened_Conduit_ID, 4);   // Conduit ID (4)
        }
        
        if (0 != (flags & StateFlags::Has_String)) {
            ret.append(msg.String, msg.String_Length);
            ret.append("\0", 1);                            // String value ending in \0 (n + 1)
        }
        
        if (msg.Segments.size() > 0) {
            uint8 segment_count = (uint8)(msg.Segments.size());
            ret.append((char*)&segment_count, 1);           // Segment count (1)
            
            for (const auto & seg : msg.Segments) {
                ret.append(seg.Name);
                ret.append("\0", 1);                        // String name ending in \0 (n + 1)

                ret.append((char*)&seg.Length, 4);          // Segment size (4)
                ret.append((char*)seg.Data, seg.Length);    // Segment data (n)
            }
        }

            // Sanity check
        DbAssert(ret.size() == required_size);

        return ret;
    }


}
}