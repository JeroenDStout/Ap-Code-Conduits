/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>
#include <algorithm>

namespace Conduits {
namespace Util {

    inline std::string Sanitise_Path(std::string str)
    {
        str.erase(std::remove(str.begin(), str.end(),' '), str.end());
        return str;
    }

}
}