/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BlackRoot/Pubc/Assert.h"

#include "Conduits/Pubc/Base Nexus.h"

using namespace Conduits;

    //  Setup
    // --------------------

BaseNexus::BaseNexus()
{
    this->Threads_Should_Return;

    this->set_ad_hoc_message_handling([](Raw::ConduitRef, Raw::IRelayMessage * msg){
        msg->set_response_string_with_copy("This nexus does not accept ad hoc messages.");
        msg->set_FAILED();
        msg->release();
    });
}

BaseNexus::~BaseNexus()
{
}

void BaseNexus::destroy() noexcept
{
    delete this;
}

void BaseNexus::internal_sync_handle_being_orphan()
{
}

    //  Settings
    // --------------------

void BaseNexus::set_ad_hoc_message_handling(MessageFunc func)
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->Ad_Hoc_Handler = func;
}

    //  Piping
    // --------------------

void BaseNexus::start_accepting_threads()
{
    this->Threads_Should_Return = false;
}

void BaseNexus::stop_accepting_threads_and_return()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->Threads_Should_Return = true;
    lk.unlock();

    this->Cv_Queue_And_Events.notify_all();
}

void BaseNexus::internal_push_message(MessageObject obj)
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->Message_Queue.push(obj);
    lk.unlock();

    this->Cv_Queue_And_Events.notify_one();
}

void BaseNexus::await_message_and_handle()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);

    if (this->Threads_Should_Return)
        return;

    if (this->Message_Queue.size() == 0) {
        this->Cv_Queue_And_Events.wait(lk, [&]{
            return this->Message_Queue.size() > 0 || this->Threads_Should_Return;
        });
    }

    if (this->Message_Queue.size() == 0)
        return;

    auto message = std::move(this->Message_Queue.front());
    this->Message_Queue.pop();

    if (message.Ref == Raw::ConduitRefNone) {
        lk.unlock();
        this->Ad_Hoc_Handler(message.Ref, message.Message);
        return;
    }

    auto & it = this->Message_Map.find(message.Ref);
    if (it == this->Message_Map.end()) {
        lk.unlock();
        
        message.Message->set_response_string_with_copy("This nexus does not accept messages with this conduit reference.");
        message.Message->set_FAILED_connexion();
        message.Message->release();
        return;
    }

    auto call = *it;
    lk.unlock();

    call.second(message.Ref, message.Message);
}

    //  Connexions
    // --------------------

BaseNexus::ConduitRefPair BaseNexus::manual_open_conduit_to(INexus * nexus, InfoFunc info, MessageFunc message)
{
    auto new_id = this->internal_get_free_conduit_id();
    auto ret_id = nexus->reciprocate_opened_conduit(this, new_id);

    std::unique_lock<std::mutex> lk(this->Mx_Access);

    this->Info_Map[new_id]    = info;    
    this->Message_Map[new_id] = message;
    this->Send_Map[new_id]    = { nexus, ret_id };

    return ConduitRefPair(new_id, ret_id);
}

void BaseNexus::manual_acknowledge_conduit(Raw::ConduitRef ref, InfoFunc info, MessageFunc message)
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    
    auto & it = this->Send_Map.find(ref);
    DbAssertMsgFatal(it != this->Send_Map.end(), "Conduit reference is not known");
    
    this->Info_Map[ref]    = info;    
    this->Message_Map[ref] = message;
}

void BaseNexus::close(Raw::ConduitRef ref)
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);

    auto & it = this->Send_Map.find(ref);
    if (it == this->Send_Map.end())
        return;
    auto sm = it->second;

    this->Info_Map.erase(ref);
    this->Message_Map.erase(ref);
    this->Send_Map.erase(it);

    lk.unlock();

    sm.first->receive_closed_conduit_notify(sm.second);
}

Raw::ConduitRef BaseNexus::reciprocate_opened_conduit(INexus * nexus, Raw::ConduitRef ref) noexcept
{
    auto new_id = this->internal_get_free_conduit_id();

    std::unique_lock<std::mutex> lk(this->Mx_Access);
    
    this->Send_Map[new_id] = { nexus, ref };

    return new_id;
}

    //  Sending / Receiving
    // --------------------

void BaseNexus::send_on(Raw::ConduitRef ref, Raw::IRelayMessage * msg)
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);

    auto & it = this->Send_Map.find(ref);
    if (it == this->Send_Map.end()) {
        lk.unlock();

        msg->set_response_string_with_copy("Conduit was not open");
        msg->set_FAILED_connexion();
        msg->release();

        return;
    }

    auto sm = it->second;
    lk.unlock();

    if (sm.first->receive_on_conduit(sm.second, msg))
        return;
    
    DbAssertMsgFatal(0, "Conduit did not accept message");
}

bool BaseNexus::receive_on_conduit(Raw::ConduitRef ref, Raw::IRelayMessage * msg) noexcept
{
    this->internal_push_message({ ref, nullptr, msg });
    return true;
}

void BaseNexus::receive_closed_conduit_notify(Raw::ConduitRef ref) noexcept
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
 
    auto & it = this->Info_Map.find(ref);
    if (it == this->Info_Map.end()) {
        if (this->Is_Orphan) {
            this->internal_sync_handle_being_orphan();
        }
        return;
    }

    auto sm = it->second;
    
    this->Info_Map.erase(it);
    this->Message_Map.erase(ref);
    this->Send_Map.erase(ref);

    lk.unlock();

    ConduitUpdateInfo info;
    info.State = ConduitState::closed;

    sm(ref, info);
}

    //  Util
    // --------------------

Raw::ConduitRef BaseNexus::internal_get_free_conduit_id()
{
    // {paranoia} Theoretically could overflow
    // after prolonged use

    auto ref = ++this->Next_Free_Conduit_Ref;
    DbAssert(ref < Raw::ConduitRefNone);
    return ref;
}

void BaseNexus::async_add_ad_hoc_message(Raw::IRelayMessage * msg)
{
    this->internal_push_message({ Raw::ConduitRefNone, nullptr, msg });
}

void BaseNexus::make_orphan()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->Is_Orphan = true;
}