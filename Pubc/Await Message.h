/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <mutex>
#include <condition_variable>

#include "Conduits/Pubc/Base Message.h"

namespace Conduits {

    class BaseAwaitMessage : public IBaseMessage {
    protected:
        bool                     Has_Been_Released;
        std::mutex               Mx_Condition_Variable;
        std::condition_variable  Cv_Condition_Variable;

    public:
        void sender_await() noexcept;

        void release() noexcept override;
    };

}