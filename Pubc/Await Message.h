/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <mutex>
#include <condition_variable>

#include "Conduits/Pubc/Base Message.h"
#include "Conduits/Pubc/Interface Conduit.h"

namespace Conduits {

    class BaseAwaitMessage : public IBaseMessage {
    protected:
        std::mutex               Mx_Condition_Variable;
        std::condition_variable  Cv_Condition_Variable;
        bool                     Has_Been_Released;

    public:
        void sender_await() noexcept {
            std::unique_lock<std::mutex> lk(this->Mx_Condition_Variable);
            this->Cv_Condition_Variable.wait(lk, [&]{
                return this->Has_Been_Released;
            });
        }
        
            // We (for now) do not allow an await message to open a conduit
        void open_conduit_for_sender(Raw::INexus*, Raw::IOpenConduitHandler * handler) noexcept override {
            Raw::IOpenConduitHandler::ResultData data;
            handler->handle_failure(&data);
        }

        void release() noexcept override {
            std::unique_lock<std::mutex> lk(this->Mx_Condition_Variable);
            this->Has_Been_Released = true;
            this->Cv_Condition_Variable.notify_all();
        }
    };
    

}