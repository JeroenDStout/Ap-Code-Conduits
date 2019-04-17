/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/Number Types.h"

namespace Conduits {
    
    struct MessageState {
        typedef unsigned char Type;
        enum : Type {
            OK      = 0x1 << 0,
            Pending = 0x1 << 1,
            Failed  = 0x1 << 2,
            Aborted = 0x1 << 7
        };
    };
    
    struct ResponseState {
        typedef unsigned char Type;
        enum : Type {
            OK        = 0x1 << 0,
            NotNeeded = 0x1 << 1,
            Pending   = 0x1 << 2,
            Aborted   = 0x1 << 7
        };
    };
    
    struct ResponseDesire {
        typedef unsigned char Type;
        enum : Type {
            Needed    = 0x1 << 0,
            NotNeeded = 0x1 << 1,
        };
    };

}