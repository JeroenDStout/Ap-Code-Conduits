/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/Number Types.h"

namespace Conduits {
    
    struct MessageState {
        typedef unsigned char Type;
        enum : Type {
            pending           = 0,
            ok                = 1,
            ok_opened_conduit = 2,
            failed            = 3,
            connexion_failure = 4,
            aborted           = 5
        };
    };
    
    struct ResponseDesire {
        typedef unsigned char Type;
        enum : Type {
            not_needed        = 0,
            needed            = 1
        };
    };

}