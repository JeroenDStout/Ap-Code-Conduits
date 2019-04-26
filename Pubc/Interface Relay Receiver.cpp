/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Conduits/Pubc/Interface Relay Receiver.h"

using namespace Conduits;

    //  Handle
    // --------------------

bool IRelayMessageReceiver::rmr_handle_message_immediate(Raw::IRelayMessage * msg)
{
    const auto * path = msg->get_adapting_path();

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
        msg->move_adapting_path_start(relayLength + 1);
        return !this->internal_rmr_try_relay_immediate(relayName.c_str(), msg);
    }

        // The remainder of the path is our call; get it and try to call
    const auto * call = msg->get_adapting_path();
    this->internal_rmr_try_call_immediate(call, msg);
    return true;
}

void IRelayMessageReceiver::rmr_handle_message_immediate_and_release(Raw::IRelayMessage * msg)
{
    if (this->rmr_handle_message_immediate(msg)) {
        msg->release();
    }
}

    //  Failure
    // --------------------

bool IRelayMessageReceiver::rmr_handle_call_failure_immediate(const char * path, Raw::IRelayMessage * msg)
{
    std::string res = "Could not call <";
    res.append(path).append("> on <").append(this->internal_get_rmr_class_name()).append(">");
    msg->set_response_string_with_copy(res.c_str());
    msg->set_FAILED();
    return false;
}

bool IRelayMessageReceiver::rmr_handle_relay_failure_immediate(const char * path, Raw::IRelayMessage * msg)
{
    std::string res = "Could not relay <";
    res.append(path).append("> with ").append(this->internal_get_rmr_class_name()).append(">");
    msg->set_response_string_with_copy(res.c_str());
    msg->set_FAILED();
    return false;
}