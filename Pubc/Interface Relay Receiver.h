/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>
#include <vector>
#include <map>

#include "BlackRoot/Pubc/JSON.h"

#include "Conduits/Pubc/Interface Raw Message.h"

namespace Conduits {

    class IRelayMessageReceiver {
    public:
        using JSON   = BlackRoot::Format::JSON;
        using RMRDir = JSON::array_t;

    protected:
        inline virtual bool internal_rmr_try_call_immediate(const char * path, Raw::IRelayMessage * msg) {
            return rmr_handle_call_failure_immediate(path, msg);
        }
        inline virtual bool internal_rmr_try_relay_immediate(const char * path, Raw::IRelayMessage * msg) {
            return rmr_handle_relay_failure_immediate(path, msg);
        }
        inline virtual bool internal_rmr_dir(RMRDir & dir) {
            return internal_rmr_dir_relay(dir);
        }
        inline virtual bool internal_rmr_dir_relay(RMRDir & dir) {
            return false;
        }
        inline virtual const char * internal_get_rmr_class_name() {
            return "IRelayMessageReceiver";
        }

        virtual bool rmr_handle_call_failure_immediate(const char * path, Raw::IRelayMessage *);
        virtual bool rmr_handle_relay_failure_immediate(const char * path, Raw::IRelayMessage *);

    public:
        virtual ~IRelayMessageReceiver() = 0 { ; }

        bool rmr_handle_message_immediate(Raw::IRelayMessage *);
    };

    namespace Helper {
        
        template<typename C>
        struct RelayReceiverFunctionMap {
            typedef void (C::*MFP)(Raw::IRelayMessage *);

                // A map which links string to specific class function
            std::map<std::string, MFP> Map;

                // Try to match a string to a function or return false
            inline bool try_call(C * c, const char * call, Raw::IRelayMessage * msg) {
                auto & it = Map.find(call);
                if (it != Map.end()) {
                    (c->*(it->second))(msg);
                    return true;
                }
                return false;
            }

            RelayReceiverFunctionMap() { ; }
        };

        template<typename C>
        struct RelayReceiverFunctionConnector {
            typedef void (C::*MFP)(Raw::IRelayMessage *);

            RelayReceiverFunctionConnector(RelayReceiverFunctionMap<C> * map, char * name, MFP f) {
                map->Map.insert( { name, f } );
            }
        };

    }

}

#define CON_RMR_DECLARE_CLASS(x, baseClass) \
    public: \
    using RelayReceiverClass = x; \
    using RelayReceiverBaseClass = baseClass; \
    using RawRelayMessage = Conduits::Raw::IRelayMessage; \
    \
    struct __RelayReceiverProps##x { \
        Conduits::Helper::RelayReceiverFunctionMap<x> Function_Map; \
    };\
    static __RelayReceiverProps##x _RelayReceiverProps##x; \
    \
    inline bool internal_rmr_try_call_immediate(const char * call, Conduits::Raw::IRelayMessage * msg) override { \
        if (_RelayReceiverProps##x.Function_Map.try_call(this, call, msg)) { \
            msg->release(); \
            return true; \
        } \
        return this->RelayReceiverBaseClass::internal_rmr_try_call_immediate(call, msg); \
    } \
    \
    inline bool internal_rmr_dir(RMRDir & dir) override { \
        this->RelayReceiverBaseClass::internal_rmr_dir(dir); \
        for (auto & func : _RelayReceiverProps##x.Function_Map.Map) { \
            dir.push_back(func.first); \
        } \
        return true; \
    } \
    \
    inline virtual const char * internal_get_rmr_class_name() { \
        return #x; \
    }

#define CON_RMR_DECLARE_FUNC(func) \
    public: void _##func(RawRelayMessage * msg) noexcept; \

#define CON_RMR_DEFINE_CLASS(x) \
    x::__RelayReceiverProps##x x::_RelayReceiverProps##x = {};
    
#define CON_RMR_REGISTER_FUNC(x, func) \
    static Conduits::Helper::RelayReceiverFunctionConnector<x> RelayReceiverFunctionConnector##x##func(&(x::_RelayReceiverProps##x.Function_Map), #func, &x::_##func);