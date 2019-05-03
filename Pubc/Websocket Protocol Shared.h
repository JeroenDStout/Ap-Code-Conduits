/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/JSON.h"
#include "BlackRoot/Pubc/Exception.h"

namespace Conduits {
namespace Protocol {

	static const char * Protocol_Version          = "v0.0.2";

	static const char * Protocol_What_Ho_Message  = "what-ho!";
	static const char * Protocol_What_Ho_Response = "what-ho, what-ho!";

    static const char * Protocol_JSON_What_Ho_Name              = "name";
    static const char * Protocol_JSON_What_Ho_Version           = "version";
    static const char * Protocol_JSON_What_Ho_Protocol_Version  = "protocol";

}
}