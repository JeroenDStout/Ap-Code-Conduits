/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BlackRoot/Pubc/Stringstream.h"
#include "BlackRoot/Pubc/Assert.h"

#include "Conduits/Pubc/Savvy Relay Receiver.h"

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

	body.append("  <p><b>").append(msg->get_response_string()).append("</b></p>\n");

        // Do a raw output
        // TODO: prettify this?
    for (int i = 0; i < 5; i++) {
        const auto message_seg = msg->get_response_segment(i);
        if (message_seg.Length == 0)
            continue;

        std::string data;
        data.assign((char*)message_seg.Data, message_seg.Length);
        size_t pos = 0;
        while ((pos = data.find("\\n", pos)) != data.npos) {
            data.replace(pos, 2, "<br/>");
        }

            // For now, just data raw
	    body.append("  <p><b>Data</b><br/>").append(data).append(" (Base Relay)</p>\n");
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
            html.append("<a href='?call=").append(action_name).append("'>").append(action_name).append("</a>"); 
        }
    }

    return html;
}

    //  Calls
    // --------------------

void SavvyRelayMessageReceiver::_dir(RawRelayMessage * msg) noexcept
{
    savvy_try_wrap_write_json(msg, 0, [&]{
        RMRDir relay;
        this->internal_rmr_dir_relay(relay);

        RMRDir dir;
        this->internal_rmr_dir(dir);

        JSON response = {};

        if (relay.size() > 0) {
            response["relay"] = relay;
        }
        if (dir.size() > 0) {
            response["dir"] = dir;
        }

        msg->set_OK();

        return response;
    });

}

void SavvyRelayMessageReceiver::_http(RawRelayMessage * msg) noexcept
{
    savvy_try_wrap_read_write_json(msg, 0, 0, [&](JSON http_request) {
        JSON http_reponse;
        std::string message_body;

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
                    std::string & call_str = call->get<std::string>();

                        // We check if the call is http, which makes the http handler
                        // call itself indefinitely; for obvious reasons this is bad
                    if (0 != call_str.compare("http")) {
                        this->savvy_handle_http_call(call_str, msg, http_request, http_reponse, message_body);
                        handledHtml = true;
                    }
                }
            }
        }
        catch (...) {
            msg->set_response_string_with_copy("malformed query");
            msg->set_FAILED();
            return JSON(0);
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

        msg->set_OK();

        return http_reponse;
    });
}