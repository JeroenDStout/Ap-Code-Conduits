/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BlackRoot/Pubc/Stringstream.h"
#include "BlackRoot/Pubc/Assert.h"

#include "Conduits/Pubc/Savvy Relay Receiver.h"
#include "Conduits/Pubc/Base Nexus Message.h"

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

void SavvyRelayMessageReceiver::savvy_handle_http_call(std::string call, Raw::IMessage * msg, const JSON http_request, JSON & http_reply, std::string & body)
{
        // Internally forward this as a message through
        // a dummy base nexus message; we fully assume
        // it will be handled immediately. However, 
        // RMR calls may not release the message itself,
        // so we do not worry about deallocation.
    Conduits::BaseNexusMessage await_msg;
    await_msg.Response_Desire = Conduits::Raw::ResponseDesire::required;
    this->internal_rmr_try_call_immediate(call.c_str(), &await_msg);

    const auto & class_name = this->internal_get_rmr_class_name();

	body = "<!doctype html>\n";
    body.append("<html>\n");
    body.append(" <head>\n");
    body.append("  <title>").append(class_name).append("</title>\n");
    body.append(" </head>\n");
    body.append(" <body>\n");
	body.append("  <h1>").append(class_name).append(" :: ").append(call).append(" — ");
    body.append(await_msg.result_is_OK() ? "OK" : "Failed").append("</h1>\n");

    if (await_msg.Response) {
        auto * response = await_msg.Response;

        const char * str = response->get_message_string();
        if (str && str[0]) {
	        body.append("  <p><b>").append(str).append("</b></p>\n");
        }

            // Do a raw output
        size_t segment_count = response->get_message_segment_count();
        if (segment_count > 0) {
            std::vector<Conduits::Raw::SegmentData> segments;
            segments.resize(segment_count);
            response->get_message_segment_list(&segments.front(), segment_count);

                // Output every segment as a string
            for (auto & elem : segments) {
	            body.append("  <p><b>Segment '").append(elem.Name).append("'</b><br/>\n   ");
                body.append((char*)elem.Data, elem.Length).append("</p>\n");
            }
        }

        response->set_OK();
        response->release();
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
    savvy_try_wrap(msg, [&]{
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
        
        auto * reply = new Conduits::DisposableMessage();
        reply->Segment_Map[""] = response.dump();

        msg->set_response(reply);
        msg->set_OK();
    });
}

void SavvyRelayMessageReceiver::_http(RawRelayMessage * msg) noexcept
{
    savvy_try_wrap_read_json(msg, "header", [&](JSON http_request) {
        JSON http_reponse;
        std::string message_body;

        bool handledHtml = false;
   
        std::unique_ptr<DisposableMessage> reply(new Conduits::DisposableMessage());

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
            reply->Message_String = "Malformed query";
            msg->set_response(reply.release());
            msg->set_FAILED();
            return;
        }

            // Internally deal with it
        if (!handledHtml) {
            this->savvy_handle_http(http_request, http_reponse, message_body);
        }
    
            // http_response means we may have to pass headers back;
            // we stringify the json and put it in segment 'header'
            // while the body evidently goes in segment 'body'
        if (!http_reponse.is_null()) {
            reply->Segment_Map["header"] = http_reponse.dump();
        }
        reply->Segment_Map["body"] = std::move(message_body);

        msg->set_response(reply.release());
        msg->set_OK();
    });
}