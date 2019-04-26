/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Conduits/Pubc/Interface Nexus.h"

namespace Conduits {
namespace Raw {

    class IOpenConduitHandler {
    public:
        struct ResultData {
        };

        virtual void handle_success(ResultData*, ConduitRef) noexcept = 0;
        virtual void handle_failure(ResultData*) noexcept = 0;
    };

}
}