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

    class BaseNexus : public Raw::INexus {
    public:
        using InfoFunc       = std::function<void(Raw::ConduitRef, const ConduitUpdateInfo)>;
        using MessageFunc    = std::function<void(Raw::ConduitRef, Raw::IRelayMessage*)>;
        
        using ConduitRefPair = std::pair<Raw::ConduitRef, Raw::ConduitRef>;

        using InfoFuncMap    = std::map<Raw::ConduitRef, InfoFunc>;
        using MessageFuncMap = std::map<Raw::ConduitRef, MessageFunc>;
        using SendConduitMap = std::map<Raw::ConduitRef, std::pair<Raw::INexus*, Raw::ConduitRef>>;
        using NoveltyCdMap   = std::map<std::pair<Raw::INexus*, Raw::ConduitRef>, Raw::ConduitRef>;
        using ClosedCList    = std::vector<Raw::ConduitRef>;

        struct MessageObject {
            Raw::ConduitRef     Ref;
            ConduitUpdateInfo   *Info;
            Raw::IRelayMessage  *Message;
        };

        using MessageOQueue  = std::queue<MessageObject>;

    protected:
        std::mutex               Mx_Access;
        std::condition_variable  Cv_Queue_And_Events;

        bool            Is_Orphan, Threads_Should_Return;

        MessageOQueue   Message_Queue;

        InfoFuncMap     Info_Map;
        MessageFuncMap  Message_Map;
        SendConduitMap  Send_Map;
        ClosedCList     Closed_Conduits_There;

        MessageFunc     Ad_Hoc_Handler;

        std::atomic<Raw::ConduitRef> Next_Free_Conduit_Ref;

        Raw::ConduitRef internal_get_free_conduit_id();
        void            internal_push_message(MessageObject);
        void            internal_sync_handle_being_orphan();
    public:
        BaseNexus();
        virtual ~BaseNexus() override;

        virtual void set_ad_hoc_message_handling(MessageFunc);

        virtual ConduitRefPair manual_open_conduit_to(INexus *, InfoFunc, MessageFunc);
        virtual void manual_acknowledge_conduit(Raw::ConduitRef, InfoFunc, MessageFunc);

        virtual void send_on(Raw::ConduitRef, Raw::IRelayMessage*);

        virtual void close(Raw::ConduitRef);

        virtual void async_add_ad_hoc_message(Raw::IRelayMessage*);
        
        virtual void make_orphan();

        virtual void start_accepting_threads();
        virtual void stop_accepting_threads_and_return();
        virtual void await_message_and_handle();

        void destroy() noexcept override;
        
        Raw::ConduitRef reciprocate_opened_conduit(INexus *, Raw::ConduitRef) noexcept override;

        bool receive_on_conduit(Raw::ConduitRef, Raw::IRelayMessage *) noexcept override;

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