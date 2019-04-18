/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <functional>

#include "BlackRoot/Pubc/Number Types.h"

#include "Conduits/Pubc/Interface Raw Message.h"

namespace Conduits {
namespace Raw {
        
    using ConduitRef = uint32;
    static ConduitRef ConduitRefNone = ConduitRef(-1);

    class INexus {
    protected:
        virtual ~INexus() = 0 { ; }
        
    public:
        virtual void destroy() noexcept = 0;

        virtual bool receive_on_conduit(ConduitRef, Raw::IRelayMessage *) noexcept = 0;
        virtual ConduitRef reciprocate_opened_conduit(INexus *, ConduitRef) noexcept = 0;

        virtual void receive_closed_conduit_notify(ConduitRef) noexcept = 0;
    };
        
    static_assert(sizeof(INexus) == sizeof(void*), "Packing issue");

}
}