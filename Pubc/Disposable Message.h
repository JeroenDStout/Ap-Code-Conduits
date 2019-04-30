/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Conduits/Pubc/Base Message.h"
#include "Conduits/Pubc/Interface Conduit.h"

namespace Conduits {

    class DisposableMessage : public IBaseMessage {
    protected:
    public:
            // We do not allow a disposable message to open a conduit
        void open_conduit_for_sender(Raw::INexus*, Raw::IOpenConduitHandler * handler) noexcept override {
            Raw::IOpenConduitHandler::ResultData data;
            handler->handle_failure(&data);
        }

            // As there is no state related to the sender,
            // we can safely just delete upon release
        void release() noexcept override {
            delete this;
        }
    };

}