/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/Number Types.h"

namespace Conduits {
namespace Raw {

    struct SegmentData {
        using LengthType = uint32;

        const char  *Name;
        void        *Data;
        LengthType  Length;
    };

    struct ResponseDesire {
        using Type = uint8;
        enum : Type {
            required,
            allowed,
            not_allowed
        };

        inline static bool response_is_possible(Type v) {
            return (v != not_allowed);
        }
    };

    struct OKState {
        using Type = uint8;
        enum : Type {
            ok,
            ok_opened_conduit
        };
    };

    struct FailState {
        using Type = uint8;
        enum : Type {
            failed,
            failed_no_conduit,
            failed_no_connexion,
            failed_no_response_expected,
            failed_timed_out,
            receiver_will_not_handle
        };
    };

}
}