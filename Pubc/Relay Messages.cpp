///* This Source Code Form is subject to the terms of the Mozilla Public
// * License, v. 2.0. If a copy of the MPL was not distributed with this
// * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
//
//#include "BlackRoot/Pubc/Assert.h"
//
//#include "Conduits/Pubc/Relay Messages.h"
//
//using namespace Conduits;
//
//RelayMessage::RelayMessage(Raw::IRawReceivingMessage * msg)
//: Internal_Message(msg)
//{
//        // We wrap round our internal (C-style) message
//        // with nice comfortable JSON
//
//        // The first data segment must be the JSON part
//    Raw::SegmentData data;
//    msg->get_message_segment(0, data);
//    JSON json = JSON::parse((char*)(data.Data), (char*)(data.Data) + data.Length);
//
//        // Extract the URI
//    JSON & uri = json["uri"];
//    if (uri.is_string()) {
//        this->Uri = uri.get<std::string>();
//    }
//
//        // Extract the call name
//    JSON & func = json["func"];
//    if (func.is_string()) {
//        this->Func = func.get<std::string>();
//    }
//
//        // Extract the message
//    JSON & message = json["message"];
//    this->Message = std::move(message);
//}
//
//void RelayMessage::get_message_raw_data(Raw::SegIndex index, Raw::SegmentData & data)
//{
//        // Index cannot be 0; we store the JSON there!
//    DbAssert(index != 0);
//
//        // Refer to the internal message
//    this->Internal_Message->get_message_segment(index, data);
//}
//
//void RelayMessage::set_message_raw_data(Raw::SegIndex index, const Raw::SegmentData data)
//{
//        // Index cannot be 0; we store the JSON there!
//    DbAssert(index != 0);
//
//        // Refer to the internal message
//    this->Internal_Message->set_response_segment(index, data);
//}
//
//void RelayMessage::connect_sender_to_circuit(IConduit * conduit)
//{
//    this->Internal_Message->connect_sender_to_circuit(conduit);
//}
//
//void RelayMessage::set_OK_and_keep()
//{
//    this->Internal_State = MessageState::OK;
//}
//
//void RelayMessage::set_FAILED_and_keep()
//{
//    this->Internal_State = MessageState::Failed;
//}
//
//void RelayMessage::release()
//{
//        // If the receiver doesn't care about our response,
//        // don't do anything here
//    if (!this->Internal_Message->get_expects_response()) {
//        delete this;
//        return;
//    }
//
//        // Create string from response
//    std::string response = this->Response.dump();
//    
//        // Describe it as segment data
//    Raw::SegmentData data;
//    data.Data    = (void*)(response.c_str());
//    data.Length  = response.length();
//
//    this->Internal_Message->set_response_segment(0, data);
//
//        // Set the internal message to a state and release
//    switch (this->Internal_State) {
//    default:
//        DbAssertMsg(0, "Unknown state");
//    case MessageState::OK:
//        this->Internal_Message->set_OK_and_release();
//        break;
//    case MessageState::Failed:
//        this->Internal_Message->set_FAILED_and_release();
//        break;
//    };
//
//    delete this;
//}
//
//MessageState::Type RelayMessage::get_internal_state()
//{
//    return this->Internal_State;
//}