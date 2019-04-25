/* This Source Code Form is subject to the terms of the Mozilla Public
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
        
        static const Type   Has_String          = 0x1;
        static const Type   Has_Segments        = 0x2;
        static const Type   Is_Response         = 0x4;

            // if response...
        static const Type   Has_Succeeded       = 0x8;

            // if not response...
        static const Type   Requires_Response   = 0x8;
    };

        // ------ Message

    struct MessageScratch {
        using State       = MessageState::Type;
        using SegmentElem = std::pair<uint8, Raw::SegmentData>;
        using SegmentList = std::vector<SegmentElem>;

        uint32            Recipient_ID;
        uint32            Reply_To_Me_ID;

        StateFlags::Type  Flags;

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
        void set_has_succeeded(bool b) {
            if (b) { this->Flags |= StateFlags::Has_Succeeded; }
            else { this->Flags &= ~StateFlags::Has_Succeeded; }
        }
        void set_requires_response(bool b) {
            if (b) { this->Flags |= StateFlags::Requires_Response; }
            else { this->Flags &= ~StateFlags::Requires_Response; }
        }

        bool get_is_resonse()        { return 0 != (this->Flags & StateFlags::Is_Response); }
        bool get_has_succeeded()     { return 0 != (this->Flags & StateFlags::Has_Succeeded); }
        bool get_requires_response() { return 0 != (this->Flags & StateFlags::Requires_Response); }
        
        static MessageScratch try_parse_message(void * msg, size_t length);
        static std::string    try_stringify_message(const MessageScratch &);
    };

        // Implement Message from client

    inline MessageScratch MessageScratch::try_parse_message(void * _msg, size_t length)
    {
        MessageScratch ms;
        auto * msg = (char*)(_msg);

            // Fixed header (0) (9 bytes)

        ms.Recipient_ID             = *(uint32*)(msg);  // H+0 Recipient ID (4)
        msg += 4;

        ms.Reply_To_Me_ID           = *(uint32*)(msg);  // H+4 Reply-To-Me ID (4)
        msg += 4;

        ms.Flags                    = msg[0];           // H+8 Flags (1)
        msg += 1;
            
            // Optional if has string (H+9) (2+x)

        if (0 != (ms.Flags & StateFlags::Has_String)) {
            ms.String_Length        = *(uint16*)(msg);  // St+0 String length (2)
            msg += 2;

            ms.String               = msg;              // St+2 String (x)
            msg += ms.String_Length;
        }
        else {
            ms.String               = nullptr;
            ms.String_Length        = 0;
        }

            // Optional if has segments (H+9 or St+2+x) (5*n+Σx_n)
            
        if (0 != (ms.Flags & StateFlags::Has_Segments)) {
            uint8  segmentCount     = msg[0];           // Sg+0 Segment count (1)
            msg += 1;
        
            for (uint8 seg = 0; seg < segmentCount; seg++) {
                uint8 index         = msg[0];           // Sg+(5*n+Σx_n)+0 Index (1)
                msg += 1;

                uint32 length       = *(uint32*)(msg);  // Sg+(5*n+Σx_n)+1 Length (4)
                msg += 4;
                
                Raw::SegmentData data;
                data.Data = (void*)(msg);               // Sg+(5*n+Σx_n)+5 Data (x_n)
                data.Length = length;

                ms.Segments.push_back( { index, data } );

                msg += length;
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

            // Fixed header, 9 bytes (0) (9 bytes)

        required_size = 9;
        
            // Optional if has message (H+9) (2+x)

        if (msg.String_Length > 0) {
            flags |= StateFlags::Has_String;
            required_size += 2 + msg.String_Length;
        }
        
            // Optional if has segments (H+9 or St+2+x) (5*n+Σx_n)

        if (msg.Segments.size() > 0) {
            flags |= StateFlags::Has_Segments;
            required_size += 1;
            for (const auto & seg : msg.Segments) {
                DbAssertFatal(seg.second.Length <= 0xFFFFFFFF);
                required_size += 5 + seg.second.Length;
            }
        }

        std::string ret;
        ret.reserve(required_size);

            // Fixed header (0) (9 bytes)
        
        ret.append((char*)(&msg.Recipient_ID), 4);              // H+0 Recipient ID (4)
        ret.append((char*)(&msg.Reply_To_Me_ID), 4);            // H+4 Reply-To-Me ID (4)
        ret.append((char*)(&flags), 1);                         // H+8 Flags (1)
            
            // Optional if has string (H+9) (2+x)

        if (msg.String_Length > 0) {
            ret.append((char*)(&msg.String_Length), 2);         // St+0 String length (2)
            ret.append(msg.String, msg.String_Length);          // St+2 String (x)
        }

            // Optional if has segments (H+9 or St+2+x) (5*n+Σx_n)

        if (msg.Segments.size() > 0) {
            uint8 segmentCount = (uint8)(msg.Segments.size());
            ret.append((char*)(&segmentCount), 1);              // St+0 String length (2)
            
            for (const auto & seg : msg.Segments) {
                ret.append((char*)(&seg.first), 1);             // Sg+(5*n+Σx_n)+0 Index (1)

                uint32 segSize = (uint32)(seg.second.Length);
                ret.append((char*)(&segSize), 4);               // Sg+(5*n+Σx_n)+1 Size (4)
                
                ret.append((char*)(seg.second.Data), segSize); // Sg+(5*n+Σx_n)+5 Data (x_n)
            }
        }

            // Sanity check
        DbAssert(ret.size() == required_size);

        return ret;
    }


}
}