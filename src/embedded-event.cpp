#include "embedded-event.h"
#include "embedded-event-mutex.h"

event::group::group(const char* name)
:   name(name)
{

}

event::group::~group()
{
    // Clear the queue
    this->event_mutex.lock();
    while(this->event_queue.size() > 0) {
        delete this->event_queue.front();
        this->event_queue.pop_front();
    }
    this->event_mutex.unlock();
}

void event::group::add(const event::registration reg)
{
    // Lock the queues
    this->registration_mutex.lock();

    // Check for an unregistration
    size_t i = 0;
    while(i < this->remove_queue.size()) {

        // Check for same event
        if(this->remove_queue.at(i) == reg) {

            // Remove the unregistration
            this->remove_queue.erase(this->remove_queue.begin() + i);
        } else {
            i++;
        }
    }

    // Add the registration
    this->add_queue.push_back(reg);

    // Unlock the queues
    this->registration_mutex.unlock();

    // Signal the addition
    this->sync_point.signal();
}

event::registration event::group::add(int32_t event_id, event::handler_fun fun, void* context)
{
    event::registration reg(event_id, fun, context);
    
    this->add(reg);

    // Return the registration
    return reg;
}

void event::group::remove(event::registration reg)
{
    // Lock the queues
    this->registration_mutex.lock();

    // Check for an registration
    size_t i = 0;
    while(i < this->add_queue.size()) {

        // Check for same event
        if(this->add_queue.at(i) == reg) {

            // Remove the unregistration
            this->add_queue.erase(this->add_queue.begin() + i);
        } else {
            i++;
        }
    }

    // Remove the registration
    this->remove_queue.push_back(reg);

    // Unlock the queues
    this->registration_mutex.unlock();

    // Signal the removal
    this->sync_point.signal();
}

void event::group::post(int32_t event, const void* data, const size_t data_length)
{
    // Lock the event queue
    this->event_mutex.lock();

    // Add the event
    this->event_queue.push_back(
        new event::container(event, data, data_length)
    );

    // Unlock the event queue
    this->event_mutex.unlock();

    // Signal the event
    this->sync_point.signal();
}

event::link* event::group::find_link(int32_t event_id)
{
    for(size_t i = 0; i < this->handlers.size(); i++) {
        if(this->handlers.at(i)->event == event_id) {
            return this->handlers[i];
        }
    }
    return NULL;
}

void event::group::wait_for(int32_t event)
{
    this->wait_any(&event, 1);
}

void event::group::wait_any(const int32_t *events, const size_t num_events)
{
    // Lock the addition queue
    this->registration_mutex.lock();

    // Create a barrier
    event::barrier *trigger = new event::barrier();

    // Iterate through the events
    for(size_t i = 0; i < num_events; i++) {

        // Find the entry for the event ID
        event::link *map = this->find_link(events[i]);

        // Check for a valid entry
        if(!map) {

            // Create a new map
            map = new event::link(events[i]);
            this->handlers.push_back(map);
        }

        // Lock the trigger
        map->lock();

        // Add the trigger
        map->add(trigger);

        // Unlock the map
        map->unlock();
    }

    // Unlock the registration mutex
    this->registration_mutex.unlock();

    // Wait for the event
    trigger->wait();

    // Lock the addition queue
    this->registration_mutex.lock();

    // Iterate through the events
    for(size_t i = 0; i < num_events; i++) {

        // Find the entry for the event ID
        event::link *map = this->find_link(events[i]);

        // Check for a valid entry
        if(!map) {

            // Trigger is cleared
            continue;
        }

        // Lock the trigger
        map->lock();

        // Add the trigger
        map->remove(trigger);

        // Unlock the map
        map->unlock();
    }

    // Unlock the registration mutex
    this->registration_mutex.unlock();

    // Delete the barrier
    delete trigger;
}

void event::group::wait_all(const int32_t *events, const size_t num_events)
{
    // Create the array of barriers
    event::barrier *triggers = new event::barrier[num_events];

    // Iterate through the events
    for(size_t i = 0; i < num_events; i++) {

        // Find the entry for the event ID
        event::link *map = this->find_link(events[i]);

        // Check for a valid entry
        if(!map) {

            // Create a new map
            map = new event::link(events[i]);
            this->handlers.push_back(map);
        }

        // Lock the trigger
        map->lock();

        // Add the trigger
        map->add(triggers + i);

        // Unlock the map
        map->unlock();
    }

    // Unlock the registration mutex
    this->registration_mutex.unlock();

    // Wait for each barrier
    for(size_t i = 0; i < num_events; i++)
    {
        (triggers + i)->wait();
    }

    // Lock the addition queue
    this->registration_mutex.lock();

    // Iterate through the events
    for(size_t i = 0; i < num_events; i++) {

        // Find the entry for the event ID
        event::link *map = this->find_link(events[i]);

        // Check for a valid entry
        if(!map) {

            // Trigger is cleared
            continue;
        }

        // Lock the trigger
        map->lock();

        // Add the trigger
        map->remove(triggers + i);

        // Unlock the map
        map->unlock();
    }

    // Unlock the registration mutex
    this->registration_mutex.unlock();

    // Delete the triggers
    delete[] triggers;
}

void event::group::dispatch()
{
    // Check for changes to the handlers before dispatching events
    this->process_handler_changes();

    // Lock the event queue
    this->event_mutex.lock();

    // Loop until the event queue is empty
    while(this->event_queue.size() > 0) {

        // Pop the event from the front
        event::container *evt = this->event_queue.front();
        this->event_queue.pop_front();

        // Unlock the event queue
        this->event_mutex.unlock();

        // Lock the registration queue
        this->registration_mutex.lock();

        // Load the map
        event::link *map = this->find_link(evt->event_id);
        if(map) {

            // Lock the link
            map->lock();

            // Unlock the registration mutex
            this->registration_mutex.unlock();

            // Process the event
            map->process(this->name, evt->data);

            // Unlock the map
            map->unlock();

        } else {
            this->registration_mutex.unlock();
        }
        delete evt;
        this->process_handler_changes();
        this->event_mutex.lock();
    }
    this->event_mutex.unlock();
}

void event::group::process_handler_changes()
{
    // Lock the addition queue
    this->registration_mutex.lock();

    // Loop until all adds are adjuticated
    while(this->add_queue.size() > 0) {

        // Load the registration
        event::registration reg = this->add_queue.front();
        
        // Remove the entry from the queue
        this->add_queue.pop_front();

        // Find the entry for the event ID
        event::link *map = this->find_link(reg.event());

        // Check for a valid entry
        if(!map) {

            // Create a new map
            map = new event::link(reg.event());
            this->handlers.push_back(map);
        }

        // Lock the map
        map->lock();

        // Add the registration
        map->add(reg);

        // Unlock the map
        map->unlock();

    }

    // Loop until all removes are adjuticated
    while(this->remove_queue.size() > 0) {

        // Load the registration
        event::registration reg = this->remove_queue.front();
        
        // Remove the entry from the queue
        this->remove_queue.pop_front();

        // Find the entry for the event ID
        event::link *map = this->find_link(reg.event());

        // Check for a valid entry
        if(map) {

            // Lock the map
            map->lock();

            // Remove the registration
            map->remove(reg);

            // Check for an empty map
            if(map->is_empty()) {

                // Search for the map
                for(size_t i = 0; i < this->handlers.size(); i++) {
                    
                    if(map == this->handlers.at(i)) {
                        this->handlers.erase(this->handlers.begin() + i);
                        break;
                    }

                }

                // Unlock the map
                map->unlock();

                // Delete the map
                delete map;
                
            } else {
                
                // Unlock the map
                map->unlock();
            }
        }
    }

    // Unlock the removal queue
    this->registration_mutex.unlock();
}

void event::group::clear_events()
{
    this->event_mutex.lock();
    while(!this->event_queue.empty()) {
        delete this->event_queue.front();
        this->event_queue.pop_front();
    }
    this->event_mutex.unlock();
}