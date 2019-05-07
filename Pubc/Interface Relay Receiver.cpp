/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Conduits/Pubc/Interface Relay Receiver.h"
#include "Conduits/Pubc/Disposable Message.h"

using namespace Conduits;

    //  Handle
    // --------------------

bool IRelayMessageReceiver::rmr_handle_message_immediate(Raw::IMessage * msg)
{
        // We treat the adapting string as a path which we
        // move over to bring it to its destination
    const auto * path = msg->get_adapting_string();

        // See if our path has a /, which indicates we need
        // to try and relay this message
    const auto * relay = strchr(path, '/');
    if (relay) {
        auto relayLength = relay - path;

        std::string relayName;
        relayName.assign(path, relayLength);

            // Remove the relay name from the path and try to relay;
            // the message is now wholly the responsibility of the
            // relay function
        msg->move_adapting_string_start(relayLength + 1);
        return !this->internal_rmr_try_relay_immediate(relayName.c_str(), msg);
    }

        // The remainder of the path is our call; get it and try to call
    this->internal_rmr_try_call_immediate(path, msg);
    return true;
}

void IRelayMessageReceiver::rmr_handle_message_immediate_and_release(Raw::IMessage * msg)
{
    if (this->rmr_handle_message_immediate(msg)) {
        msg->release();
    }
}

    //  Failure
    // --------------------

bool IRelayMessageReceiver::rmr_handle_call_failure_immediate(const char * path, Raw::IMessage * msg)
{
    msg->set_FAILED(Raw::FailState::failed);

    if (Raw::ResponseDesire::response_is_possible(msg->get_response_expectation())) {
        auto * reply = new Conduits::DisposableMessage();
        reply->Message_String = "Could not call <";
        reply->Message_String.append(path).append("> on <").append(this->internal_get_rmr_class_name()).append(">");
        reply->sender_prepare_for_send();
        msg->set_response(reply);
    }

    return false;
}

bool IRelayMessageReceiver::rmr_handle_relay_failure_immediate(const char * path, Raw::IMessage * msg)
{
    msg->set_FAILED(Raw::FailState::failed);

    if (Raw::ResponseDesire::response_is_possible(msg->get_response_expectation())) {
        auto * reply = new Conduits::DisposableMessage();
        reply->Message_String = "Could not relay <";
        reply->Message_String.append(path).append("> with <").append(this->internal_get_rmr_class_name()).append(">");
        reply->sender_prepare_for_send();
        msg->set_response(reply);
    }

    return false;
}