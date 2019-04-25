/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/JSON.h"
#include "BlackRoot/Pubc/Exception.h"

#include "Conduits/Pubc/Websocket Protocol Shared.h"

namespace Conduits {
namespace Protocol {

	struct ClientProperties {
		using JSON = BlackRoot::Format::JSON;

		std::string		Name;
		std::string		Version;
		std::string		Protocol_Version;

		JSON			All_Properties;


		static ClientProperties try_parse_what_ho_message(void *, size_t length);
	};

	struct ServerProperties {
		using JSON = BlackRoot::Format::JSON;

		std::string		Name;
		std::string		Version;

		static std::string create_what_ho_response(ServerProperties);
	};

	inline ClientProperties ClientProperties::try_parse_what_ho_message(void * _message, size_t length)
	{
        ClientProperties cp;
		char * message = (char*)_message;

        static const int what_ho_len = strlen(Protocol_What_Ho_Message);

		    // Message simply starts with what-ho string

		if (0 != strncmp(message, Protocol_What_Ho_Message, what_ho_len)) {
			throw new BlackRoot::Debug::Exception((std::stringstream{}
                << "Message did not start with correct what-ho string! ("
                << Protocol_What_Ho_Message << ")").str(), BRGenDbgInfo);
		}
		message += what_ho_len;

            // This is followed by stringified JSON

		try {
			cp.All_Properties = JSON::parse(message);
		}
		catch (std::exception e) {
			std::string ex = "Badly formatted JSON in what-ho message: ";
			ex += e.what();
			throw new BlackRoot::Debug::Exception(ex, BRGenDbgInfo);
		}
		catch (...) {
			std::string ex = "Unknown error in parsing JSON of what-ho mesasge";
			throw new BlackRoot::Debug::Exception(ex, BRGenDbgInfo);
		}

            // Extract properties

		{
			auto & it = cp.All_Properties.find(Protocol_JSON_What_Ho_Name);
			if (it == cp.All_Properties.end() || !it.value().is_string()) {
				throw new BlackRoot::Debug::Exception("Client failed to define name", BRGenDbgInfo);
			}
			cp.Name = it.value().get<std::string>();
		}
		{
			auto & it = cp.All_Properties.find(Protocol_JSON_What_Ho_Version);
			if (it == cp.All_Properties.end() || !it.value().is_string()) {
				throw new BlackRoot::Debug::Exception("Client failed to define version", BRGenDbgInfo);
			}
			cp.Version = it.value().get<std::string>();
		}
		{
			auto & it = cp.All_Properties.find(Protocol_JSON_What_Ho_Protocol_Version);
			if (it == cp.All_Properties.end() || !it.value().is_string()) {
				throw new BlackRoot::Debug::Exception("Client failed to define protocol version", BRGenDbgInfo);
			}
			cp.Protocol_Version = it.value().get<std::string>();
		}

		return cp;
	}

	inline std::string ServerProperties::create_what_ho_response(ServerProperties prop)
	{
		std::string ret = Protocol_What_Ho_Response;
		
		JSON json = {
			{ Protocol_JSON_What_Ho_Name , prop.Name },
			{ Protocol_JSON_What_Ho_Version, prop.Version },
			{ Protocol_JSON_What_Ho_Protocol_Version, Protocol_Version }
		};

		ret += json.dump();
		return ret;
	}

}
}