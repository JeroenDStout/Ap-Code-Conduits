/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BlackRoot/Pubc/Stringstream.h"
#include "BlackRoot/Pubc/Assert.h"

#include "Conduits/Pubc/Savvy Relay Receiver.h"
#include "Conduits/Pubc/Message Wrap JSON.h"

using namespace Conduits;

CON_RMR_DEFINE_CLASS(SavvyRelayMessageReceiver);
CON_RMR_REGISTER_FUNC(SavvyRelayMessageReceiver, dir);
CON_RMR_REGISTER_FUNC(SavvyRelayMessageReceiver, http);

    //  Html
    // --------------------

void SavvyRelayMessageReceiver::savvy_handle_http(const JSON http_request, JSON & http_reply, std::string & body)
{
    const auto & class_name = this->internal_get_rmr_class_name();
    
	body = "<!doctype html>\n";
    body.append("<html>\n");
    body.append(" <head>\n");
    body.append("  <title>").append(class_name).append("</title>\n");
    body.append(" </head>\n");
    body.append(" <body>\n");
	body.append("  <h1>").append(class_name).append(" (Base Relay)</h1>\n");
	body.append("  <p>").append(this->html_create_action_relay_string()).append("</p>\n");
    body.append(" </body>\n");
    body.append("</html>");
}

void SavvyRelayMessageReceiver::savvy_handle_http_call(std::string call, Raw::IRelayMessage * msg, const JSON http_request, JSON & http_reply, std::string & body)
{
    const auto & class_name = this->internal_get_rmr_class_name();

	body = "<!doctype html>\n";
    body.append("<html>\n");
    body.append(" <head>\n");
    body.append("  <title>").append(class_name).append("</title>\n");
    body.append(" </head>\n");
    body.append(" <body>\n");
	body.append("  <h1>").append(class_name).append(" :: ").append(call).append("</h1>\n");

        // We do not care about the result of the call;
        // we just call it, then report on the results
    this->internal_rmr_try_call_immediate(call.c_str(), msg);

	body.append("  <p><b>").append(msg->get_response_string()).append("</p>\n");

        // Do a raw output first few segments
        // TODO: prettify this?
    for (int i = 0; i < 5; i++) {
        const auto message_seg = msg->get_message_segment(i);
        if (message_seg.Length == 0)
            continue;

            // For now, just data raw
	    body.append("  <p><b>Data</b><br/>").append((char*)message_seg.Data, message_seg.Length).append(" (Base Relay)</p>\n");
    }

	body.append("  <h1>Relay</h1>\n");
	body.append("  <p>").append(this->html_create_action_relay_string()).append("</p>\n");
    body.append(" </body>\n");
    body.append("</html>");
}

std::string SavvyRelayMessageReceiver::html_create_action_relay_string()
{
    RMRDir relay;
    this->internal_rmr_dir_relay(relay);

    RMRDir dir;
    this->internal_rmr_dir(dir);
    
	std::string html;
    html = "Relay: <a href='/..'>..</a>";

    for (auto & elem : relay) {
        DbAssert(elem.is_string());
        
        auto & relay_name = elem.get<std::string>();

        html.append(" - <a href='/").append(relay_name).append("'>");
        html.append(relay_name).append("</a>"); 
    }
    
    if (dir.size() > 0) {
        html.append("<br/>Dir: ");

        bool anyDir = false;
        for (auto & elem : dir) {
            DbAssert(elem.is_string());

            if (anyDir) html.append(" - ");
            anyDir = true;

            auto & action_name = elem.get<std::string>();
            html.append("<a href='?").append(action_name).append("'>?").append(action_name).append("</a>"); 
        }
    }

    return html;
}

    //  Calls
    // --------------------

void SavvyRelayMessageReceiver::_dir(RawRelayMessage * _msg) noexcept
{
    auto msg = Wrap::RelayJSON::wrap_around_safe(_msg);

    RMRDir relay;
    this->internal_rmr_dir_relay(relay);

    RMRDir dir;
    this->internal_rmr_dir(dir);

    if (relay.size() > 0) {
        msg.Response["relay"] = relay;
    }
    if (dir.size() > 0) {
        msg.Response["dir"] = dir;
    }

    msg.set_OK();
}

void SavvyRelayMessageReceiver::_http(RawRelayMessage * msg) noexcept
{
        // Segment 0 (possibly) contains stringified JSON
        // along with the query parameters
    const auto message_seg = msg->get_message_segment(0);

    JSON http_request, http_reponse;
    std::string message_body;

        // A http command is expected to use JSON to pass
        // arguments, but they can be left out
    if (message_seg.Length > 0) {
        try {
            http_request = JSON::parse((char*)message_seg.Data);
        }
        catch (...) {
            msg->set_response_string_with_copy("malformed message");
            msg->set_FAILED();
            return;
        }
    }

    bool handledHtml = false;
   
        // If we have a http_request, it may contain the queries;
        // if it does, the 'call' variable is used to call functions
        // via http (which is not great but can be of use)
    try {
        auto qIt = http_request.find("query");
        if (qIt != http_request.end()) {
            auto call = qIt->find("call");
            if (call != qIt->end()) {
                DbAssert(call->is_string());
                this->savvy_handle_http_call(call->get<std::string>(), msg, http_request, http_reponse, message_body);
                handledHtml = true;
            }
        }
    }
    catch (...) {
        msg->set_response_string_with_copy("malformed query");
        msg->set_FAILED();
        return;
    }

        // Internally deal with it
    if (!handledHtml) {
        this->savvy_handle_http(http_request, http_reponse, message_body);
    }
    
        // http_response means we have to pass headers back;
        // we stringify the json and put it in segment 0
    if (!http_reponse.is_null()) {
        auto http_response_str = http_reponse.dump();

        Raw::SegmentData response_seg_http;
        response_seg_http.Data   = (void*)http_response_str.c_str();
        response_seg_http.Length = http_response_str.length();
        msg->set_response_segment_with_copy(0, response_seg_http);
    }
    
        // The message body part is copied to segment 1
    Raw::SegmentData response_seg_body;
    response_seg_body.Data   = (void*)message_body.c_str();
    response_seg_body.Length = message_body.length();
    msg->set_response_segment_with_copy(1, response_seg_body);

    msg->set_response_string_with_copy("OK");
    msg->set_OK();
}