/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Conduits/Pubc/Version.h"

#include "Conduits/-genc/def_repo_version.h"
#include "Conduits/-genc/def_contribute.h"

namespace Conduits {
	namespace Core {
		
		BR_VERSION_DEFINE(Conduits);
        BR_CONTRIBUTE_DEFINE(Conduits);
        BR_PROJECT_DEFINE(Conduits);

	}
} 
