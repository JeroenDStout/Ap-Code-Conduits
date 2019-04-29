/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "BlackRoot/Pubc/Number Types.h"

#include "Conduits/Pubc/Message State.h"

namespace Conduits {

    class IConduit;

    namespace Raw {

        using SegLength = uint32;
        using SegIndex  = uint8;

        struct SegmentData {
            SegLength Length;
            void      *Data;
        };

        class IOpenConduitHandler;
        class INexus;

        class IRelayMessage {
        protected:
            const char * Adapting_Path;

        public:
            inline const char *           get_adapting_path() const noexcept;
            inline void                   move_adapting_path_start(size_t index) noexcept;

            virtual ResponseDesire::Type  get_response_expectation() noexcept = 0;

            virtual const char *          get_path_string() const noexcept = 0;
            virtual const SegmentData     get_message_segment(SegIndex index) const noexcept = 0;
            virtual uint8                 get_message_segment_indices(uint8 * out_segments, size_t max_segments) const noexcept = 0;

            virtual bool                  set_response_string_with_copy(const char*) noexcept = 0;
            virtual bool                  set_response_segment_with_copy(SegIndex index, const SegmentData) noexcept = 0;

            virtual const char *          get_response_string() const noexcept = 0;
            virtual const SegmentData     get_response_segment(SegIndex index) const noexcept = 0;

            virtual void                  open_conduit_for_sender(INexus*, IOpenConduitHandler*) = 0;

            virtual void                  set_OK() noexcept = 0;
            virtual void                  set_OK_opened_conduit() noexcept = 0;
            virtual void                  set_FAILED() noexcept = 0;
            virtual void                  set_FAILED_connexion() noexcept = 0;

            virtual void                  release() noexcept = 0;
        };

        const char * IRelayMessage::get_adapting_path() const noexcept
        {
            return this->Adapting_Path;
        }

        void IRelayMessage::move_adapting_path_start(size_t index) noexcept
        {
            this->Adapting_Path += index;
        }

        static_assert(sizeof(SegmentData) == sizeof(uint32) + sizeof(void*), "Packing issue");
        static_assert(sizeof(IRelayMessage) == sizeof(void*)*2, "Packing issue");

    }



























//    class IAsynchMessage {
//    public:
//        struct StateType {
//            typedef unsigned char Type;
//
//            enum : Type {
//                Queued  = 0x1 << 0,
//                OK      = 0x1 << 1,
//                Failed  = 0x1 << 2,
//                Aborted = 0x1 << 7
//            };
//
//            static inline bool IsOK(Type t) {
//                return (0 != (t & OK)) && (0 == (t & (Aborted | Failed)));
//            }
//        };
//
//    public:
//        StateType::Type     State;
//
//    public:
//        JSON Message;
//        JSON Response;
//        
//        virtual ~IAsynchMessage() { ; }
//        
//        virtual void SetOK() = 0;
//        virtual void SetFailed() = 0;
//        virtual void Close() = 0;
//    };
//
//    class IMessageReceiver {
//    protected:
//        inline virtual bool InternalMessageCallFunction(std::string s, Toolbox::Messaging::IAsynchMessage *m) {
//            return false;
//        }
//        inline virtual bool InternalMessageDir(std::vector<std::string> & v) {
//            return false;
//        }
//
//    public:
//        virtual ~IMessageReceiver() = 0 { ; }
//
//        void MessageReceiveImmediate(IAsynchMessage *);
//    };
//
//    inline void IMessageReceiver::MessageReceiveImmediate(IAsynchMessage *message)
//    {
//        std::string funcName = "!error!";
//
//        if (message->Message.is_string()) {
//            funcName = message->Message.get<std::string>();
//        }
//        else if (message->Message.is_array()) {
//            funcName = message->Message[0].get<std::string>();
//        }
//        else if (message->Message.is_object()) {
//            funcName = message->Message.begin().key();
//        }
//
//        if (this->InternalMessageCallFunction(funcName, message))
//            return;
//
//        std::stringstream reply;
//        reply << "No such function: '" << funcName << "'";
//
//        message->SetFailed();
//        message->Response = { "Failed", "NoFunc", reply.str() };
//        message->Close();
//    }
//
//    namespace Macros {
//        
//        template<typename C>
//        struct FunctionMap {
//            typedef void (C::*MFP)(Toolbox::Messaging::IAsynchMessage *);
//
//            std::map<std::string, MFP> functionMap;
//            inline bool TryCall(C * c, std::string s, Toolbox::Messaging::IAsynchMessage *m) {
//                std::map<std::string, MFP>::iterator it;
//                it = functionMap.find(s);
//                if (it != functionMap.end()) {
//                    (c->*(it->second))(m);
//                    return true;
//                }
//                return false;
//            }
//            FunctionMap() { ; }
//        };
//
//        template<typename C>
//        struct FunctionConnector {
//            typedef void (C::*MFP)(Toolbox::Messaging::IAsynchMessage *);
//
//            FunctionConnector(FunctionMap<C> * map, char * name, MFP f) {
//                map->functionMap.insert( { name, f } );
//            }
//        };
//
//    }
//
//#define TB_MESSAGES_CONCAT(x, y) x ## y
//#define TB_MESSAGES_CONCAT2(x, y) TB_MESSAGES_CONCAT(x, y)
//
//#define TB_MESSAGES_DECLARE_RECEIVER(x, baseclass) \
//    typedef x         MessengerCurClass; \
//    typedef baseclass MessengerBaseClass; \
//    public: \
//    struct __MessageReceiverProps##x { \
//        Toolbox::Messaging::Macros::FunctionMap<x> FunctionMap; \
//    };\
//    static __MessageReceiverProps##x _MessageReceiverProps##x; \
//    \
//    inline bool InternalMessageCallFunction(std::string s, Toolbox::Messaging::IAsynchMessage *m) override { \
//        if (_MessageReceiverProps##x.FunctionMap.TryCall(this, s, m)) { \
//            m->Close(); \
//            return true; \
//        } \
//        return this->MessengerBaseClass::InternalMessageCallFunction(s, m); \
//    } \
//    \
//    inline bool InternalMessageDir(std::vector<std::string> & v) override { \
//        this->MessengerBaseClass::InternalMessageDir(v); \
//        for(auto const & s : _MessageReceiverProps##x.FunctionMap.functionMap) { \
//            v.push_back(s.first); \
//        } \
//        return true; \
//    }
//
//#define TB_MESSAGES_DECLARE_MEMBER_FUNCTION(func) \
//    public: void _##func(Toolbox::Messaging::IAsynchMessage*);
//
//#define TB_MESSAGES_BEGIN_DEFINE(x) \
//    x::__MessageReceiverProps##x x::_MessageReceiverProps##x = { };
//
//#define TB_MESSAGES_END_DEFINE(x)
//    
//#define TB_MESSAGES_ENUM_BEGIN_MEMBER_FUNCTIONS(x)
//    
//#define TB_MESSAGES_ENUM_MEMBER_FUNCTION(x, func) \
//    static Toolbox::Messaging::Macros::FunctionConnector<x> FunctionConnector##x##func(&(x::_MessageReceiverProps##x.FunctionMap), #func, &x::_##func);
//
//#define TB_MESSAGES_ENUM_END_MEMBER_FUNCTIONS(x)

}