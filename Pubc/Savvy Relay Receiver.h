/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/Exception.h"

#include "Conduits/Pubc/Interface Relay Receiver.h"

namespace Conduits {

    class SavvyRelayMessageReceiver : public virtual IRelayMessageReceiver {
        CON_RMR_DECLARE_CLASS(SavvyRelayMessageReceiver, IRelayMessageReceiver)

    public:        
        virtual std::string html_create_action_relay_string();

        virtual void savvy_handle_http(const JSON httpRequest, JSON & httpReply, std::string & outBody);
        virtual void savvy_handle_http_call(const std::string call, Raw::IRelayMessage *, const JSON httpRequest, JSON & httpReply, std::string & outBody);
        
            // As relay calls cannot throw exceptions, this is
            // a nice easy writter to catch exceptions
        template <typename F>
        inline static void savvy_try_wrap(Raw::IRelayMessage * msg, F && f) {
            try {
                f();
            }
            catch (BlackRoot::Debug::Exception * e) {
                std::string error = "Exception was thrown: ";
                error += e->what();
                delete e;
                msg->set_response_string_with_copy(error.c_str());
                msg->set_FAILED();
            }
            catch (std::exception e) {
                std::string error = "Exception was thrown: ";
                error += e.what();
                msg->set_response_string_with_copy(error.c_str());
                msg->set_FAILED();
            }
            catch (...) {
                msg->set_response_string_with_copy("Unknown exception thrown");
                msg->set_FAILED();
            }
        }

            // Assume that SegIndex i is a JSON string and pass
            // that to the function provided, wrapped in a try
        template <typename F>
        inline static void savvy_try_wrap_read_json(Raw::IRelayMessage * msg, Raw::SegIndex i, F && f) {
            savvy_try_wrap(msg, [&]{
                    // De-stringify the JSON: a zero-length segment becomes null
                auto segment = msg->get_message_segment(i);
                JSON json = (segment.Length > 0) ? JSON::parse((char*)segment.Data) : JSON(nullptr);

                    // Call internal func
                f(json);
            });
        }

            // Write the output of the function as a stringified
            // JSON block to the SegIndex i, wrapped in a try
        template <typename F>
        inline static void savvy_try_wrap_write_json(Raw::IRelayMessage * msg, Raw::SegIndex i, F && f) {
            savvy_try_wrap(msg, [&]{
                    // Call internal func
                JSON json = f();

                    // Dumb the JSON and set it
                std::string dump = json.dump();

                Conduits::Raw::SegmentData data;
                data.Data = (void*)dump.c_str();
                data.Length = dump.length();

                msg->set_response_segment_with_copy(i, data);
            });
        }

            // Combination of the above; read from read_i,
            // write to write_j, wrapped in a try
        template <typename F>
        inline static void savvy_try_wrap_read_write_json(Raw::IRelayMessage * msg,
                                                           Raw::SegIndex read_i, Raw::SegIndex write_i, F && f) {
            savvy_try_wrap(msg, [&]{
                    // De-stringify the JSON: a zero-length segment becomes null
                auto segment = msg->get_message_segment(read_i);
                auto json_in = (segment.Length > 0) ? JSON::parse((char*)segment.Data) : JSON(nullptr);

                    // Call internal func
                auto json_out = f(json_in);

                    // If the JSON is null we don't bother
                    // writing it to the segment
                if (json_out.is_null()) {
                    return;
                }
                
                    // Dumb the JSON and set it
                std::string dump = json_out.dump();

                Conduits::Raw::SegmentData data;
                data.Data = (void*)dump.c_str();
                data.Length = dump.length();

                msg->set_response_segment_with_copy(write_i, data);
            });
        }

        CON_RMR_DECLARE_FUNC(dir);
        CON_RMR_DECLARE_FUNC(http);
    };

}