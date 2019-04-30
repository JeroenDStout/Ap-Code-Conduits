/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/Number Types.h"

#include "Conduits/Pubc/Raw Message State.h"

namespace Conduits {

    class IConduit;

    namespace Raw {

        class IOpenConduitHandler;
        class INexus;

        class IMessage {
        protected:
            const char * Message_String;
            const char * Adapting_String;

        public:
            // --- Ease of use
        
            inline const char * get_message_string() const noexcept;
            inline const char * get_adapting_string() const noexcept;
            inline void         move_adapting_string_start(size_t index) noexcept;

            // --- Pure virtual

            virtual ResponseDesire::Type get_response_expectation() noexcept = 0;

            virtual const SegmentData get_message_segment(char*) const noexcept = 0;

            virtual size_t get_message_segment_count() const noexcept = 0;
            virtual size_t get_message_segment_list(SegmentData * out_segments, size_t max_segments) const noexcept = 0;

            virtual void open_conduit_for_sender(INexus*, IOpenConduitHandler*) = 0;

            virtual void set_OK(OKState::Type = OKState::ok) noexcept = 0;
            virtual void set_FAILED(FailState::Type = FailState::failed) noexcept = 0;

            virtual void add_response(IMessage *) noexcept = 0;

            virtual void release() noexcept = 0;
        };

        const char * IMessage::get_message_string() const noexcept {
            return this->Message_String;
        }

        const char * IMessage::get_adapting_string() const noexcept {
            return this->Adapting_String;
        }

        void IMessage::move_adapting_string_start(size_t index) noexcept {
            this->Adapting_String += index;
        }

    }
}