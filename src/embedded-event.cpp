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
    this->signal();
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
    this->signal();
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
    this->signal();
}

event::event_map* event::group::find_map(int32_t event_id)
{
    for(size_t i = 0; i < this->handlers.size(); i++) {
        if(this->handlers.at(i)->event_id == event_id) {
            return this->handlers[i];
        }
    }
    return NULL;
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
        event::event_map *map = this->find_map(evt->event_id);
        if(map && map->callbacks.size() > 0) {
            for(size_t i = 0; i < map->callbacks.size(); i++) {
                event::callback cb = map->callbacks.at(i);
                cb.raise(this->name, evt->event_id, evt->data);
            }
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
        event::event_map *map = this->find_map(reg.event());

        // Check for a valid entry
        if(!map) {

            // Create a new map
            map = new event::event_map();
            map->event_id = reg.event();
            this->handlers.push_back(map);
        }

        // Lock the map
        map->mutex.lock();

        // Check for an existing registration
        size_t i;
        for(i = 0; i < map->callbacks.size(); i++) {
            if(map->callbacks.at(i) == reg) {
                break;
            }
        }
        if(i == map->callbacks.size()) {
            
            // The hander has not been registered
            // Add the handler
            map->callbacks.push_back(reg);
        }

        // Unlock the map
        map->mutex.unlock();
    }

    // Loop until all removes are adjuticated
    while(this->remove_queue.size() > 0) {

        // Load the registration
        event::registration reg = this->remove_queue.front();
        
        // Remove the entry from the queue
        this->remove_queue.pop_front();

        // Find the entry for the event ID
        event::event_map *map = this->find_map(reg.event());

        // Check for a valid entry
        if(map) {

            // Lock the map
            map->mutex.lock();

            // Check for any callbacks
            if(map->callbacks.size() > 0) {
                // Search for the registration
                size_t i = 0;
                while(i < map->callbacks.size()) {
                    if(map->callbacks.at(i) == reg) {

                        // Remove the registration
                        map->callbacks.erase(map->callbacks.begin() + i);
                    } else {

                        // Registration not found
                        i++;
                    }
                }
            }

            // Check for an empty map
            if(map->callbacks.size() == 0) {

                // Find the map index
                const size_t index = map - this->handlers[0];

                // Erase the element
                this->handlers.erase(this->handlers.begin() + index);

                // Unlock the map
                map->mutex.unlock();

                // Delete the map
                delete map;
                
            } else {
                
                // Unlock the map
                map->mutex.unlock();
            }
        }
    }

    // Unlock the removal queue
    this->registration_mutex.unlock();
}
