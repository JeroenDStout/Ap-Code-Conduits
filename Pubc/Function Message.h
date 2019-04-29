/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <mutex>
#include <condition_variable>

#include "Conduits/Pubc/Base Message.h"

namespace Conduits {

    class BaseFunctionMessage : public IBaseMessage {
    protected:
        using CallbackFunc = std::function<void(BaseFunctionMessage * msg)>;

        CallbackFunc    Callback;
    public:
        BaseFunctionMessage(CallbackFunc);

        void release() noexcept override;
    };

    inline BaseFunctionMessage::BaseFunctionMessage(CallbackFunc f)
    : Callback(f)
    {
    }

    inline void BaseFunctionMessage::release() noexcept
    {
        if (nullptr != Callback) {
            this->Callback(this);
        }
    }

}