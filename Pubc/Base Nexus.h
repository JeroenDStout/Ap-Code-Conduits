/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <map>
#include <functional>
#include <mutex>
#include <queue>
#include <atomic>

#include "BlackRoot/Pubc/Number Types.h"

#include "Conduits/Pubc/Interface Nexus.h"

namespace Conduits {
    
    struct ConduitState {
        typedef unsigned char Type;
        enum : Type {
            open    = 0x1 << 0,
            closed  = 0x1 << 1
        };
    };

    struct ConduitUpdateInfo {
        ConduitState::Type    State;
    };

    class BaseNexusMessage;

    class BaseNexus : public Raw::INexus {
    public:
        struct QueueObjectType {
            using Type = uint8;
            enum : Type {
                ad_hoc_message,
                conduit_message,
                message_reply,
                conduit_update
            };
        };

        using InfoFunc       = std::function<void(Raw::ConduitRef, const ConduitUpdateInfo)>;
        using AdHocMsgFunc   = std::function<void(Raw::IMessage*)>;
        using ConduitMsgFunc = std::function<void(Raw::ConduitRef, const ConduitUpdateInfo)>;

        using MessageFunc    = std::function<void(Raw::ConduitRef, Raw::IMessage*)>;
        
        using ConduitRefPair = std::pair<Raw::ConduitRef, Raw::ConduitRef>;

        using InfoFuncMap    = std::map<Raw::ConduitRef, InfoFunc>;
        using MessageFuncMap = std::map<Raw::ConduitRef, MessageFunc>;
        using SendConduitMap = std::map<Raw::ConduitRef, std::pair<Raw::INexus*, Raw::ConduitRef>>;
        using NoveltyCdMap   = std::map<std::pair<Raw::INexus*, Raw::ConduitRef>, Raw::ConduitRef>;
        using ClosedCList    = std::vector<Raw::ConduitRef>;

        struct QueueObject {
            QueueObjectType::Type   Type;
            Raw::IMessage           *Message;
            Raw::ConduitRef         Received_On_Conduit;
            ConduitUpdateInfo       *Conduit_Update_Info;
        };
        using ObjectQueue  = std::queue<QueueObject>;

    protected:
        // --- Internal management

            // Nexus-changing access

        std::mutex  Mx_Access;
        bool        Is_Available_For_Incoming;
        bool        Threads_Should_Keep_Waiting;

        bool        Is_Orphan, Threads_Should_Return;

            // Message queues

        std::condition_variable  Cv_Queue_And_Events;
        ObjectQueue              Primary_Queue;

            // Relay handling

        AdHocMsgFunc    Ad_Hoc_Handler;

        InfoFuncMap     Info_Map;
        MessageFuncMap  Message_Map;
        SendConduitMap  Send_Map;

            // References

        std::atomic<Raw::ConduitRef> Next_Free_Conduit_Ref;
        Raw::ConduitRef internal_get_free_conduit_id();

            // Utility

        void            internal_push_to_queue(QueueObject);
        void            internal_sync_handle_being_orphan();
        
        void            internal_sync_handle_conduit_update(QueueObject);
        void            internal_sync_dismiss_unaccepted_object(QueueObject);

            // ???

        ClosedCList     Closed_Conduits_There;

    public:
        BaseNexus();
        virtual ~BaseNexus() override;
        
        // --- User utility

        virtual void set_ad_hoc_message_handling(AdHocMsgFunc);

                // Closes all conduits
                // Stops accepting messages
                // Handles and fails remaining messages
                //  (call stop_accepting_threads_and_return first)
        virtual void stop_gracefully();

            // Messages

        virtual void start_accepting_messages();
        virtual void stop_accepting_messages();
        virtual void handle_and_fail_remaining_messages();

            // Conduits

        virtual ConduitRefPair  manual_open_conduit_to(INexus *, InfoFunc, MessageFunc);
        virtual void            manual_acknowledge_conduit(Raw::ConduitRef, InfoFunc, MessageFunc);

        virtual void send_on_conduit(Raw::ConduitRef, Raw::IMessage*);
        virtual void send_on_conduit_and_handle_response(Raw::ConduitRef, BaseNexusMessage*);
        virtual void close_conduit(Raw::ConduitRef);
        virtual void close_all_conduits();

        virtual void async_add_ad_hoc_message(Raw::IMessage*);
        
        virtual void make_orphan();

            // User thread usage

        virtual void start_accepting_threads();
        virtual void stop_accepting_threads_and_return();

                // Handles (at most) one message from the queue
                // and returns; must be called in loop to keep
                // handling all messages
        virtual void await_message_and_handle();
        
        // --- Implementation of INexus

        void destroy() noexcept override;
        Raw::ConduitRef reciprocate_opened_conduit(INexus *, Raw::ConduitRef) noexcept override;
        bool receive_on_conduit(Raw::ConduitRef, Raw::IMessage *) noexcept override;
        void receive_closed_conduit_notify(Raw::ConduitRef) noexcept override;
    };

    template<typename nexus_type = BaseNexus>
    struct NexusHolder {
    public:
        nexus_type  *Inner_Nexus;

        inline NexusHolder() {
            Inner_Nexus = new nexus_type;
        }

        inline ~NexusHolder() {
            this->Inner_Nexus->make_orphan();
        }

        inline nexus_type* operator->() {
            return this->Inner_Nexus;
        }

        inline operator nexus_type * () {
            return this->Inner_Nexus;
        }
    };

}