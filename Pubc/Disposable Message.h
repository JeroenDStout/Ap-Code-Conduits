/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <mutex>
#include <condition_variable>

#include "Conduits/Pubc/Base Message.h"

namespace Conduits {

        // TODO: for now this adds overhead, being based on
        // IBaseMesage, which *does* expect a response
        // some future refactoring will have to fold these
        // into some better structure set

    class DisposableMessage : public IBaseMessage {
    protected:
    public:
        inline ResDes get_response_expectation() noexcept override {
            return ResponseDesire::not_needed;
        }

        inline void release() noexcept override {
            delete this;
        }
    };

}