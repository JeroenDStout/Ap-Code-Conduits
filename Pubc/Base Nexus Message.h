/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <mutex>
#include <condition_variable>

#include "Conduits/Pubc/Base Message.h"
#include "Conduits/Pubc/Interface Conduit.h"

namespace Conduits {

    class BaseNexusMessage : public IBaseMessage {
    public:
        using CallbackFunc = std::function<void(BaseNexusMessage * msg)>;
        
    protected:
        CallbackFunc    Callback;

    public:
            // Called by nexus to call the callback func
        void call_for_nexus();

            // Called by nexus to delete the object
        virtual void destroy_for_nexus();

        void release() noexcept override;
    };
    

}