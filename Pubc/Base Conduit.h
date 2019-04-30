/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/Number Types.h"

#include "Conduits/Pubc/Interface Conduit.h"

namespace Conduits {

    class FunctionOpenConduitHandler : public Raw::IOpenConduitHandler {
    public:
        struct Result {
            bool             Is_Success;
            Raw::ConduitRef  Ref;
        };

        using CallbackFunc = std::function<void(Result)>;

    protected:
        CallbackFunc    Callback;

    public:
        FunctionOpenConduitHandler(CallbackFunc f)
        : Callback(f) { ; }

        void handle_success(Raw::IOpenConduitHandler::ResultData*, Raw::ConduitRef ref) noexcept override {
            Result r;
            r.Is_Success = true;
            r.Ref = ref;
            Callback(r);
        }

        void handle_failure(Raw::IOpenConduitHandler::ResultData*) noexcept override {
            Result r;
            r.Is_Success = false;
            r.Ref = Raw::ConduitRefNone;
            Callback(r);
        }
    };

}