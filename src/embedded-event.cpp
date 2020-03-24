#include "embedded-event.h"
#include "embedded-event-mutex.h"

event::group::group(const char* name)
:   name(name)
{
    #ifdef ESP_PLATFORM
    esp_event_loop_args_t loop_without_task_args = {
        .queue_size = 5,
        .task_name = NULL // no task will be created
    };
    ESP_ERROR_CHECK(esp_event_loop_create(&loop_without_task_args, &this->loop_handle));
    #endif
}

event::group::~group()
{
    #ifdef ESP_PLATFORM
    ESP_ERROR_CHECK(esp_event_loop_delete(this->loop_handle));
    #else
    // Clear the queue
    this->event_mutex.lock();
    while(this->event_queue.size() > 0) {
        delete this->event_queue.front();
        this->event_queue.pop_front();
    }
    this->event_mutex.unlock();
    #endif
}

event::registration event::group::add(int32_t event_id, event::handler_fun fun, void* context)
{
    event::registration reg;
    reg.handler = fun;
    reg.context = context;
    reg.event = event_id;

    #ifdef ESP_PLATFORM
    ESP_ERROR_CHECK(esp_event_handler_register(
        this->loop_handle,
        this->name, 
        event_id, 
        handler,
        context,
        reg.instance;
        ));
    #else
    // Lock the queues
    this->registration_mutex.lock();

    // Check for an unregistration
    size_t i = 0;
    while(i < this->remove_queue.size()) {

        // Check for same event
        if(this->remove_queue.at(i).event != reg.event) {
            i++;
            continue;
        }

        // Check for same handler
        if(this->remove_queue.at(i).handler != reg.handler) {
            i++;
            continue;
        }

        // Remove the unregistration
        this->remove_queue.erase(this->remove_queue.begin() + i);
    }

    // Add the registration
    this->add_queue.push_back(reg);

    // Unlock the queues
    this->registration_mutex.unlock();
    #endif

    // Return the registration
    return reg;
}

void event::group::remove(event::registration reg)
{
    #ifdef ESP_PLATFORM
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(
        this->loop_handle,
        this->name,
        reg.event,
        reg.instance));
    #else
    // Lock the queues
    this->registration_mutex.lock();

    // Check for an registration
    size_t i = 0;
    while(i < this->add_queue.size()) {

        // Check for same event
        if(this->add_queue.at(i).event != reg.event) {
            i++;
            continue;
        }

        // Check for same handler
        if(this->add_queue.at(i).handler != reg.handler) {
            i++;
            continue;
        }

        // Remove the unregistration
        this->add_queue.erase(this->add_queue.begin() + i);
    }

    // Remove the registration
    this->remove_queue.push_back(reg);

    // Unlock the queues
    this->registration_mutex.unlock();
    #endif
}

void event::group::post(int32_t event, const void* data, const size_t data_length)
{
    #ifdef ESP_PLATFORM
    ESP_ERROR_CHECK(esp_event_post_to(
        this->loop_handle, 
        this->name, 
        event,
        *data, 
        data_length, 
        1000
        );
    #else
    this->event_mutex.lock();
    this->event_queue.push_back(
        new event::container(event, data, data_length)
    );
    this->event_mutex.unlock();
    #endif
}

#ifdef ESP_PLATFORM

void event::group::set_dispatch_timeout(const unsigned int ms_timeout)
{
    this->timeout = ms_timeout;
}

#else

event::event_map* event::group::find_map(int32_t event_id)
{
    for(size_t i = 0; i < this->handlers.size(); i++) {
        if(this->handlers.at(i).event_id == event_id) {
            return &this->handlers[i];
        }
    }
    return NULL;
}

void event::group::dispatch()
{
    #ifdef ESP_PLATFORM
    ESP_ERROR_CHECK(esp_event_loop_run(
        this->loop_handle
        this->timeout / portTICK_PERIOD_MS
        );
    #else
    this->process_handler_changes();
    this->event_mutex.lock();
    while(this->event_queue.size() > 0) {
        event::container *evt = this->event_queue.front();
        this->event_queue.pop_front();
        this->event_mutex.unlock();
        event::event_map *map = this->find_map(evt->event_id);
        if(map && map->callbacks.size() > 0) {
            for(size_t i = 0; i < map->callbacks.size(); i++) {
                event::callback cb = map->callbacks.at(i);
                cb.handler(cb.context, this->name, evt->event_id, evt->data);
            }
        }
        delete evt;
        this->process_handler_changes();
        this->event_mutex.lock();
    }
    this->event_mutex.unlock();
    #endif
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
        event::event_map *map = this->find_map(reg.event);

        // Check for a valid entry
        if(map) {

            // Check for an existing registration
            size_t i;
            for(i = 0; i < map->callbacks.size(); i++) {
                if(map->callbacks.at(i).handler == reg.handler) {
                    break;
                }
            }
            if(i < map->callbacks.size()) {
                
                // The hander is already registered
                continue;
            }

            // Add the handler
            map->callbacks.push_back(reg);

        } else {

            // Add the map
            event::event_map entry;
            entry.event_id = reg.event;
            entry.callbacks.push_back(reg);
            this->handlers.push_back(entry);
        }
    }

    // Loop until all removes are adjuticated
    while(this->remove_queue.size() > 0) {

        // Load the registration
        event::registration reg = this->remove_queue.front();
        
        // Remove the entry from the queue
        this->remove_queue.pop_front();

        // Find the entry for the event ID
        event::event_map *map = this->find_map(reg.event);

        // Check for a valid entry
        if(map && map->callbacks.size() > 0) {

            // Search for the registration
            size_t i = 0;
            while(i < map->callbacks.size()) {
                if(map->callbacks.at(i).handler == reg.handler) {

                    // Remove the registration
                    map->callbacks.erase(map->callbacks.begin() + i);
                } else {

                    // Registration not found
                    i++;
                }
            }
        }
    }

    // Unlock the removal queue
    this->registration_mutex.unlock();
}

#endif