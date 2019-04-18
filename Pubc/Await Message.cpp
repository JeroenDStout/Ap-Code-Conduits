/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Conduits/Pubc/Await Message.h"

using namespace Conduits;

    //  Await
    // --------------------

void BaseAwaitMessage::sender_await() noexcept
{
    std::unique_lock<std::mutex> lk(this->Mx_Condition_Variable);
    this->Cv_Condition_Variable.wait(lk, [&]{
        return this->Has_Been_Released;
    });
}

void BaseAwaitMessage::release() noexcept
{
    std::unique_lock<std::mutex> lk(this->Mx_Condition_Variable);
    this->Has_Been_Released = true;
    this->Cv_Condition_Variable.notify_all();
}