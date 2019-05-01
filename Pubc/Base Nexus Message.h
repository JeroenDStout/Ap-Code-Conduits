/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <mutex>
#include <condition_variable>

#include "Conduits/Pubc/Base Message.h"
#include "Conduits/Pubc/Base Nexus.h"
#include "Conduits/Pubc/Interface Conduit.h"

namespace Conduits {

    class BaseNexusMessage : public IBaseMessage {
    public:
        using   OpenConduitFunc = std::function<void(Raw::INexus*, BaseNexusMessage*, Raw::IOpenConduitHandler*)>;
        using   OnReleaseFunc   = std::function<void(BaseNexusMessage*)>;
        
        OpenConduitFunc  Open_Conduit_Func;
        OnReleaseFunc    On_Release_Func;

        BaseNexus        *Associated_Nexus;

        Raw::ConduitRef  Conduit_ID;

        BaseNexusMessage()
        : Open_Conduit_Func(nullptr),
          On_Release_Func(nullptr),
          Associated_Nexus(nullptr)
        { ; }

            // Called by nexus to call the callback func
        void call_for_nexus();

            // Called by nexus to delete the object
        virtual void destroy_for_nexus();
        
        void open_conduit_for_sender(Raw::INexus*, Raw::IOpenConduitHandler*) noexcept override;
        void release() noexcept override;
    };
    
    inline void BaseNexusMessage::call_for_nexus() {
        if (this->On_Release_Func) {
            this->On_Release_Func(this);
        }
    }
    
    inline void BaseNexusMessage::destroy_for_nexus() {
        delete this;
    }
    
    inline void BaseNexusMessage::open_conduit_for_sender(Raw::INexus * nexus, Raw::IOpenConduitHandler * handler) noexcept {
        if (this->Open_Conduit_Func) {
            this->Open_Conduit_Func(nexus, this, handler);
            return;
        }

        Raw::IOpenConduitHandler::ResultData data;
        handler->handle_failure(&data);
    }

    inline void BaseNexusMessage::release() noexcept {
        if (this->Associated_Nexus) {
            this->Associated_Nexus->async_handle_message_released(this);
            return;
        }

            // Fallback; handle own events
        this->call_for_nexus();
        this->destroy_for_nexus();
    }

}