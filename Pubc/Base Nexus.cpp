/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "BlackRoot/Pubc/Assert.h"

#include "Conduits/Pubc/Base Nexus.h"
#include "Conduits/Pubc/Disposable Message.h"
#include "Conduits/Pubc/Base Nexus Message.h"

using namespace Conduits;

    //  Setup
    // --------------------

BaseNexus::BaseNexus()
{
    this->Is_Available_For_Incoming   = false;
    this->Threads_Should_Keep_Waiting = false;

    this->set_ad_hoc_message_handling([](Raw::IMessage * msg){
        if (Raw::ResponseDesire::response_is_possible(msg->get_response_expectation())) {
            auto * reply = new Conduits::DisposableMessage();
            reply->Message_String = "This nexus does not listen to ad hoc messages";
            msg->set_response(reply);
        }
        
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

    //  Lifetime
    // --------------------

void BaseNexus::make_orphan()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->Is_Orphan = true;
}

void BaseNexus::internal_sync_handle_being_orphan()
{
}

    //  Settings
    // --------------------

void BaseNexus::set_ad_hoc_message_handling(AdHocMsgFunc func)
{
    this->Ad_Hoc_Handler = func;
}

    //  Threads
    // --------------------

void BaseNexus::await_message_and_handle()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);

        // If we should not attend to the queue
        // let the caller deal with this thread
    if (!this->Threads_Should_Keep_Waiting)
        return;

        // If our queue is empty simply wait for
        // a new addition or a return command
    if (this->Primary_Queue.size() == 0) {
        this->Cv_Queue_And_Events.wait(lk, [&]{
            return this->Primary_Queue.size() > 0 || this->Threads_Should_Return;
        });
    }

        // During our lock the end signal may have
        // changed; so check again
    if (!this->Threads_Should_Keep_Waiting)
        return;

        // If there was no new message we were
        // either accidentally woken up, or we
        // intentionally should return; either way
        // we let our caller have the thread back
    if (this->Primary_Queue.size() == 0)
        return;

    auto object = std::move(this->Primary_Queue.front());
    this->Primary_Queue.pop();

    switch (object.Type) {
        default:
            DbAssertMsgFatal(0, "Illegal object type in primary queue");

            // Simply forward an ad hoc message to the handler
        case QueueObjectType::ad_hoc_message: {
            lk.unlock();
            this->Ad_Hoc_Handler(object.Message);

            return;
        }
        
            // Try to find the message delivery call
            // corresponding to the circuit id
        case QueueObjectType::conduit_message: {
            auto & it = this->Message_Map.find(object.Received_On_Conduit);
            if (it == this->Message_Map.end()) {
                lk.unlock();

                object.Message->set_FAILED(Raw::FailState::failed_no_connexion);
                object.Message->release();
                return;
            }

            auto call = *it;
            lk.unlock();

            call.second(object.Received_On_Conduit, object.Message);

            return;
        }
        
            // Try to find the update delivery call
            // corresponding to the circuit id
        case QueueObjectType::conduit_update: {
            auto & it = this->Info_Map.find(object.Received_On_Conduit);
            if (it == this->Info_Map.end()) {
                lk.unlock();

                object.Message->set_FAILED(Raw::FailState::failed_no_connexion);
                object.Message->release();
                return;
            }

            auto call = *it;
            lk.unlock();

            call.second(object.Received_On_Conduit, *object.Conduit_Update_Info);

            return;
        }
        
            // Use this thread to handle the callback
            // for the message & delete it
        case QueueObjectType::message_reply: {
            lk.unlock();

            auto msg = (BaseNexusMessage*)object.Message;
            msg->call_for_nexus();
            msg->destroy_for_nexus();

            return;
        }
    }
}

    //  Control
    // --------------------

void BaseNexus::start_accepting_messages()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->Is_Available_For_Incoming = true;
}

void BaseNexus::stop_accepting_messages()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->Is_Available_For_Incoming = false;
}

void BaseNexus::handle_and_fail_remaining_messages()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);

        // We simply loop through all queue events and
        // fail them all; this means message replies
        // will not be received at all
    while (this->Primary_Queue.size() > 0) {
        auto object = std::move(this->Primary_Queue.front());
        this->Primary_Queue.pop();

        this->internal_sync_dismiss_unaccepted_object(object);
    }
}

void BaseNexus::start_accepting_threads()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->Threads_Should_Keep_Waiting = true;
}

void BaseNexus::stop_accepting_threads_and_return()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->Threads_Should_Keep_Waiting = false;
    lk.unlock();

    this->Cv_Queue_And_Events.notify_all();
}

void BaseNexus::close_all_conduits()
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    
        // Transfer the conduit refs to a temporary vector
    std::vector<std::pair<INexus*, Raw::ConduitRef>> closed;
    closed.reserve(this->Send_Map.size());

    for (auto & elem : this->Send_Map) {
        closed.push_back(elem.second);
    }

        // Clear all elements
    this->Send_Map.clear();
    this->Info_Map.clear();
    this->Message_Map.clear();

    lk.unlock();

        // Finally notify the others we removed them all
    for (auto & elem : closed) {
        elem.first->receive_closed_conduit_notify(elem.second);
    }
}

void BaseNexus::stop_gracefully()
{
    DbAssert(!this->Threads_Should_Keep_Waiting);
    
    this->stop_accepting_messages();
    this->close_all_conduits();
    this->handle_and_fail_remaining_messages();
}

    //  Conduits for users of class
    // --------------------
    
BaseNexus::ConduitRefPair BaseNexus::manual_open_conduit_to(INexus * nexus, InfoFunc info, MessageFunc message)
{
        // Get id for calling us on the new conduit
    auto new_id = this->internal_get_free_conduit_id();

        // Get id for calling the other nexus on the conduit;
        // this handles opening the conduit on the other end
    auto ret_id = nexus->reciprocate_opened_conduit(this, new_id);

        // Update our maps
    std::unique_lock<std::mutex> lk(this->Mx_Access);

    this->Info_Map[new_id]    = info;    
    this->Message_Map[new_id] = message;
    this->Send_Map[new_id]    = { nexus, ret_id };

        // Return the pair ( OUR id, OTHER id ); the other still
        // needs to acknowledge the opened conduit, which is what
        // the id is for
    return ConduitRefPair(new_id, ret_id);
}

void BaseNexus::manual_acknowledge_conduit(Raw::ConduitRef ref, InfoFunc info, MessageFunc message)
{
        // Update our maps for receiving messages and
        // info for this conduit
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    
    auto & it = this->Send_Map.find(ref);
    DbAssertMsgFatal(it != this->Send_Map.end(), "Conduit reference is not known");
    
    this->Info_Map[ref]    = info;    
    this->Message_Map[ref] = message;
}

void BaseNexus::close_conduit(Raw::ConduitRef ref)
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);

        // See if this conduit is actually known; if not
        // we can just ignore its closure
    auto & send_it = this->Send_Map.find(ref);
    if (send_it == this->Send_Map.end())
        return;
    
    auto send_handle = send_it->second;

        // Erase the references from the maps
    this->Send_Map.erase(send_it);
    this->Info_Map.erase(ref);
    this->Message_Map.erase(ref);

    lk.unlock();

        // Notify the other nexus the conduit
        // closed in this unilateral way; we
        // don't send a message to our own closed
        // handler seeing as we began the closure
    send_handle.first->receive_closed_conduit_notify(send_handle.second);
}

    //  Conduits internal
    // --------------------

Raw::ConduitRef BaseNexus::reciprocate_opened_conduit(INexus * nexus, Raw::ConduitRef ref) noexcept
{
        // Get id for calling us on the new conduit
    auto new_id = this->internal_get_free_conduit_id();

    std::unique_lock<std::mutex> lk(this->Mx_Access);
    
        // Add us to the send map - we don't know yet
        // what functions to call until the conduit
        // is acknowledged with manual_acknowledge_conduit
    this->Send_Map[new_id] = { nexus, ref };

    return new_id;
}

void BaseNexus::receive_closed_conduit_notify(Raw::ConduitRef ref) noexcept
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);
 
        // This reference should always be known but
        // we handle its failure gracefully
    auto & it = this->Info_Map.find(ref);
    if (it == this->Info_Map.end()) {
        return;
    }
    auto sm = it->second;
    
        // Erase the conduit from all the maps
    this->Info_Map.erase(it);
    this->Message_Map.erase(ref);
    this->Send_Map.erase(ref);

        // If we are not available for incoming we
        // will not report to the function that the
        // conduit has been closed
    if (!this->Is_Available_For_Incoming) {
        return;
    }

    lk.unlock();

        // Send a message to the appropriate conduit
        // info function that it was closed
    ConduitUpdateInfo info;
    info.State = ConduitState::closed;

    sm(ref, info);
}

    //  Messages for users of class
    // --------------------

void BaseNexus::send_on_conduit(Raw::ConduitRef ref, Raw::IMessage * msg)
{
    std::unique_lock<std::mutex> lk(this->Mx_Access);

        // Ensure we can find the conduit; if not we
        // fail the message with a connexion problem
    auto & it = this->Send_Map.find(ref);
    if (it == this->Send_Map.end()) {
        lk.unlock();

        msg->set_FAILED(Raw::FailState::failed_no_connexion);
        msg->release();

        return;
    }

    auto sm = it->second;
    lk.unlock();

        // Call the other nexus to record the conduit
    if (sm.first->receive_on_conduit(sm.second, msg))
        return;
    
    DbAssertMsgFatal(0, "Conduit did not accept message");
}

void BaseNexus::setup_use_queue_for_release_event(BaseNexusMessage * msg)
{
    msg->Associated_Nexus = this;
    
        // Add to the pending list; this nexus
        // may not die until these mesages have
        // returned as they hold a pointer to us
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->Handling_Messages.push_back(msg);
}

void BaseNexus::async_handle_message_released(BaseNexusMessage * msg)
{
    QueueObject obj;
    obj.Type = QueueObjectType::message_reply;
    obj.Message = msg;

    std::unique_lock<std::mutex> lk(this->Mx_Access);

        // Remove from the pending list; for the
        // purpose of failing gracefully we do not
        // actually care about whether it was here
    for (size_t i = 0; i < this->Handling_Messages.size(); i++) {
        if (this->Handling_Messages[i] != msg)
            continue;
        this->Handling_Messages.erase(this->Handling_Messages.begin() + i);
        break;
    }

        // Queue the object
    this->internal_sync_push_to_queue(obj);
}

    //  Messages internal
    // --------------------
    
bool BaseNexus::receive_on_conduit(Raw::ConduitRef ref, Raw::IMessage * msg) noexcept
{
        // Queue the object
    QueueObject obj;
    obj.Type = QueueObjectType::conduit_message;
    obj.Received_On_Conduit = ref;
    obj.Message = msg;
    
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->internal_sync_push_to_queue(obj);

    return true;
}

void BaseNexus::internal_sync_dismiss_unaccepted_object(QueueObject object)
{
    switch (object.Type) {
    case QueueObjectType::ad_hoc_message:
    case QueueObjectType::conduit_message:
        object.Message->set_FAILED(Raw::FailState::receiver_will_not_handle);
        object.Message->release();
        break;
    case QueueObjectType::conduit_update:
        break;
    case QueueObjectType::message_reply:
        ((BaseNexusMessage*)object.Message)->destroy_for_nexus();
        break;
    }
}

    //  Utility
    // --------------------

Raw::ConduitRef BaseNexus::internal_get_free_conduit_id()
{
    // {paranoia} Theoretically could overflow
    // after prolonged use

    auto ref = ++this->Next_Free_Conduit_Ref;
    DbAssert(ref < Raw::ConduitRefNone);
    return ref;
}

void BaseNexus::internal_sync_push_to_queue(QueueObject obj)
{
    if (!this->Is_Available_For_Incoming) {
        this->internal_sync_dismiss_unaccepted_object(obj);
        return;
    }

    this->Primary_Queue.push(obj);

    this->Cv_Queue_And_Events.notify_one();
}

void BaseNexus::async_add_ad_hoc_message(Raw::IMessage * msg)
{
    DbAssert(!this->Is_Orphan);
    DbAssert(this->Is_Available_For_Incoming);
    
    QueueObject obj;
    obj.Type = QueueObjectType::ad_hoc_message;
    obj.Message = msg;
    
    std::unique_lock<std::mutex> lk(this->Mx_Access);
    this->internal_sync_push_to_queue(std::move(obj));
}

