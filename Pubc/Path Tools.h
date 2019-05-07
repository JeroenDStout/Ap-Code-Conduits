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

        size_t last_index = str.npos;
        for (auto index = str.find('/', 0); index != str.npos; index = str.find('/', index)) {
            if (index == 0 || index == last_index + 1) {
                str.erase(str.begin()+index);
                continue;
            }
            if (index == str.size()-1)
                break;
            last_index = index;
            index++;
        }

        return str;
    }

}
}