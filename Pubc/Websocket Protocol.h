/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/JSON.h"
#include "BlackRoot/Pubc/Exception.h"

namespace Conduits {
namespace Protocol {

	static const char * Welcome_Message = "what-ho!";
	static const char * Welcome_Message_Resonse = "what-ho, what-ho!";

	struct ClientProperties {
		using JSON = BlackRoot::Format::JSON;

		std::string		Client_Name;
		std::string		Client_Version;
		JSON			Client_All_Properties;

		std::string		Protocol_Version;

		static ClientProperties try_parse_welcome_message(void *, size_t length);
	};

	struct ServerProperties {
		using JSON = BlackRoot::Format::JSON;

		std::string		Server_Name;
		std::string		Server_Version;

		static std::string create_welcome_message_response(ServerProperties);
	};

	inline ClientProperties ClientProperties::try_parse_welcome_message(void * _message, size_t length)
	{
		char * message = (char*)_message;

			// String must start with welcome message
		if (0 != strncmp(message, Welcome_Message, strlen(Welcome_Message))) {
			throw new BlackRoot::Debug::Exception("Badly formatted welcome message", BRGenDbgInfo);
		}
		message += strlen(Welcome_Message);

			// What follows is just stringified JSON,
			// which we attempt to parse
		ClientProperties prop;

		try {
			prop.Client_All_Properties = JSON::parse(message);
		}
		catch (std::exception e) {
			std::string ex = "Badly formatted JSON in welcome mesasge: ";
			ex += e.what();
			throw new BlackRoot::Debug::Exception(ex, BRGenDbgInfo);
		}
		catch (...) {
			std::string ex = "Unknown error in parsing JSON of welcome mesasge";
			throw new BlackRoot::Debug::Exception(ex, BRGenDbgInfo);
		}

			// Extract properties
		{
			auto & it = prop.Client_All_Properties.find("client-name");
			if (it == prop.Client_All_Properties.end() || !it.value().is_string()) {
				throw new BlackRoot::Debug::Exception("Client failed to define name", BRGenDbgInfo);
			}
			prop.Client_Name = it.value().get<std::string>();
		}
		{
			auto & it = prop.Client_All_Properties.find("client-version");
			if (it == prop.Client_All_Properties.end() || !it.value().is_string()) {
				throw new BlackRoot::Debug::Exception("Client failed to define version", BRGenDbgInfo);
			}
			prop.Client_Version = it.value().get<std::string>();
		}
		{
			auto & it = prop.Client_All_Properties.find("protocol-version");
			if (it == prop.Client_All_Properties.end() || !it.value().is_string()) {
				throw new BlackRoot::Debug::Exception("Client failed to define protocol version", BRGenDbgInfo);
			}
			prop.Protocol_Version = it.value().get<std::string>();
		}

		return prop;
	}

	inline std::string ServerProperties::create_welcome_message_response(ServerProperties prop)
	{
		std::string ret = Welcome_Message_Resonse;
		
		JSON json = {
			{ "server-name", prop.Server_Name },
			{ "server-version", prop.Server_Version },
			{ "protocol-version", "v0.0.1" }
		};

		ret += json.dump();
		return ret;
	}

}
}