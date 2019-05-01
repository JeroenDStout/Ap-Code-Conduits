/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/Exception.h"
#include "BlackRoot/Pubc/Threaded IO Stream.h"

#include "Conduits/Pubc/Interface Relay Receiver.h"
#include "Conduits/Pubc/Disposable Message.h"

namespace Conduits {

    class SavvyRelayMessageReceiver : public virtual IRelayMessageReceiver {
        CON_RMR_DECLARE_CLASS(SavvyRelayMessageReceiver, IRelayMessageReceiver)

    public:
        virtual std::string html_create_action_relay_string();

        virtual void savvy_handle_http(const JSON httpRequest, JSON & httpReply, std::string & outBody);
        virtual void savvy_handle_http_call(const std::string call, Raw::IMessage *, const JSON httpRequest, JSON & httpReply, std::string & outBody);
        
            // As relay calls may not throw exceptions, this is
            // a nice easy wrapper to catch exceptions
        template <typename F>
        inline static void savvy_try_wrap(Raw::IMessage * msg, F && f) {
            std::string error;

            try {
                f();
                return;
            }
            catch (BlackRoot::Debug::Exception * e) {
                error = "Exception was thrown: ";
                error += e->what();
                delete e;
            }
            catch (std::exception e) {
                error = "Std exception was thrown: ";
                error += e.what();
            }
            catch (...) {
                error = "Unknown exception was thrown";
            }
            
            if (!Raw::ResponseDesire::response_is_possible(msg->get_response_expectation())) {
                using cout = BlackRoot::Util::Cout;
                cout{} << "Message had exception which could not be sent as response:" << std::endl
                    << error << std::endl;
            }
            
                // Auto create reply if needed

            auto * reply = new Conduits::DisposableMessage();
            reply->Message_String = error;

            reply->sender_prepare_for_send();

            msg->set_response(reply);
            msg->set_FAILED();
        }

            // Assume that segment <name> is a JSON string and pass
            // that to the function provided, wrapped in a try
        template <typename F>
        inline static void savvy_try_wrap_read_json(Raw::IMessage * msg, char * name, F && f) {
            savvy_try_wrap(msg, [&]{
                    // De-stringify the JSON: a zero-length segment becomes null
                auto segment = msg->get_message_segment(name);
                JSON json = (segment.Length > 0) ? JSON::parse((char*)segment.Data) : JSON(nullptr);

                    // Call internal func
                f(json);
            });
        }

        CON_RMR_DECLARE_FUNC(dir);
        CON_RMR_DECLARE_FUNC(http);
    };

}