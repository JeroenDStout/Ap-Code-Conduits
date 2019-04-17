/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/Number Types.h"

#include "Conduits/Pubc/Message State.h"
#include "Conduits/Pubc/Interface Raw Message.h"

namespace Conduits {
namespace Raw {

    class IConduit {
    public:
        virtual void ReceiveRawMessage(IRelayMessage*) = 0;
    };

}
}