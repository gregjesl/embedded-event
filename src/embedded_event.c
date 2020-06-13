#include "embedded-event.h"

void embedded_event_push(embedded_event_t *first, int32_t event, void *data, event_posted_callback_t callback)
{
    embedded_event_t evt = (embedded_event_t)malloc(sizeof(struct embedded_event_struct));
    evt->event = event;
    evt->data = data;
    evt->callback = callback;
    evt->next = NULL;

    if(*first == NULL) {
        *first = evt;
        return;
    }

    embedded_event_t current = *first;
    embedded_event_t next = NULL;
    while(current->next != NULL) {
        next = current->next;
        current = next;
    }

    current->next = evt;
}

embedded_event_t embedded_event_pop(embedded_event_t *first)
{
    if(*first == NULL) return NULL;
    
    embedded_event_t current = *first;
    embedded_event_t next = current->next;
    *first = next;
    current->next = NULL;
    return current;
}

embedded_event_group_t embedded_event_group_init()
{
    embedded_event_group_t result = (embedded_event_group_t)malloc(sizeof(struct embedded_event_group_struct));
    result->registration_mutex = macrothread_mutex_init();
    result->add_queue = NULL;
    result->remove_queue = NULL;
    result->handlers = NULL;
    result->event_mutex = macrothread_mutex_init();
    result->event_queue = NULL;
    result->sync_point = macrothread_condition_init();
    result->cancel_token = false;
    return result;
}

embedded_event_callback_t embedded_event_group_add(embedded_event_group_t group, int32_t event, embedded_event_handler_t handler, void *context, macrothread_condition_t signal)
{
    // Validate the input
    if(group == NULL || handler == NULL) return NULL;

    // Lock the mutex
    macrothread_mutex_lock(group->registration_mutex);

    // Create the callback
    embedded_event_callback_t cb = embedded_event_callback_init(handler, context);

    // Create the registration
    embedded_event_registration_t reg = embedded_event_registration_init(event);
    reg->callbacks = cb;
    reg->signal = signal;

    // Record the registration
    embedded_event_registration_append(&group->add_queue, reg);

    // Unlock the mutex
    macrothread_mutex_unlock(group->registration_mutex);

    // Signal
    macrothread_condition_signal(group->sync_point);

    // Return the registration
    return cb;
}

void embedded_event_group_remove(embedded_event_group_t group, int32_t event, embedded_event_callback_t callback, macrothread_condition_t signal)
{
    // Validate the input
    if(group == NULL || callback == NULL) return;

    // Lock the mutex
    macrothread_mutex_lock(group->registration_mutex);

    // Create the registration
    embedded_event_registration_t reg = embedded_event_registration_init(event);
    reg->callbacks = callback;
    reg->signal = signal;

    // Record the registration
    embedded_event_registration_append(&group->remove_queue, reg);

    // Unlock the mutex
    macrothread_mutex_unlock(group->registration_mutex);

    // Signal
    macrothread_condition_signal(group->sync_point);
}

void embedded_event_group_post(embedded_event_group_t group, int32_t event, void *data, event_posted_callback_t *callback)
{
    // Lock the mutex
    macrothread_mutex_lock(group->event_mutex);

    // Queue the event
    embedded_event_push(&group->event_queue, event, data, callback);

    // Unlock the mutex
    macrothread_mutex_unlock(group->event_mutex);

    // Signal
    macrothread_condition_signal(group->sync_point);
}

void __process_handler_changes(embedded_event_group_t group)
{
    // Lock the registration
    macrothread_mutex_lock(group->registration_mutex);

    // Check for additions
    embedded_event_registration_t reg = group->add_queue;
    embedded_event_registration_t next = NULL;
    embedded_event_registration_t existing = NULL;
    macrothread_condition_t signal = NULL;
    while(reg != NULL) {

        // Store the next one
        next = reg->next;

        // Look for an existing registration
        existing = embedded_event_registration_find(&group->handlers, reg->event);

        // Record the signal
        signal = reg->signal;

        if(existing == NULL) {
            reg->next = NULL;
            reg->signal = NULL;
            embedded_event_registration_append(&group->handlers, reg);
        } else {
            embedded_event_callback_append(&existing->callbacks, reg->callbacks);
            embedded_event_registration_destroy(reg);
        }

        // Give the signal
        if(signal != NULL)
            macrothread_condition_signal(signal);

        // Move to the next
        reg = next;
    }

    // Mark the queue as processed
    group->add_queue = NULL;

    // Check for removals
    reg = group->remove_queue;
    while(reg != NULL) {

        // Store the next one
        next = reg->next;

        // Record the signal
        signal = reg->signal;

        // Look for an existing registration
        existing = embedded_event_registration_find(&group->handlers, reg->event);

        if(existing != NULL) {
            embedded_event_callback_remove(&existing->callbacks, reg->callbacks);
            embedded_event_callback_destroy(reg->callbacks);

            // Check for an empty registration
            if(existing->callbacks == NULL) {

                // Remove the registration
                embedded_event_registration_remove(&group->handlers, existing);
                embedded_event_registration_destroy(existing);
            }
        }
        embedded_event_registration_destroy(reg);

        // Give the signal
        if(signal != NULL)
            macrothread_condition_signal(signal);

        // Move to the next one
        reg = next;
    }

    group->remove_queue = NULL;

    // Unlock the mutex
    macrothread_mutex_unlock(group->registration_mutex);
}

void __dispatch_events(embedded_event_group_t group)
{
    // Lock the mutex
    macrothread_mutex_lock(group->event_mutex);

    // Loop until all events are handled
    embedded_event_t evt = embedded_event_pop(&group->event_queue);
    embedded_event_registration_t reg = NULL;
    while(evt != NULL) {
        
        // Unlock the mutex
        macrothread_mutex_unlock(group->event_mutex);

        // Find the handler
        reg = embedded_event_registration_find(&group->handlers, evt->event);

        // Verify a handler exists
        if(reg != NULL) {

            // Execute the callbacks
            embedded_event_callback_t cb = reg->callbacks;
            embedded_event_callback_t next = NULL;
            while(cb != NULL) {

                // Execute the callback
                if(cb->handler != NULL)
                    cb->handler(evt->event, evt->data, cb->context);

                // Move to the next callback
                next = cb->next;
                cb = next;
            }
        }

        // Call the callback
        if(evt->callback != NULL)
            evt->callback(evt->event, evt->data);

        // Delete the event
        free(evt);

        // Lock the mutex
        macrothread_mutex_lock(group->event_mutex);

        // Move to the next event
        evt = embedded_event_pop(&group->event_queue);
    }

    // Unlock the mutex
    macrothread_mutex_unlock(group->event_mutex);
}

void __group_thread(void *arg)
{
    embedded_event_group_t group = (embedded_event_group_t)arg;

    while(!group->cancel_token) {

        // Check for handler changes
        __process_handler_changes(group);

        // Execute events
        __dispatch_events(group);

        // Check for handler changes again
        __process_handler_changes(group);

        // Wait for a signal
        macrothread_condition_wait(group->sync_point);
    }
}

void embedded_event_group_start(embedded_event_group_t group)
{
    group->thread = macrothread_handle_init();
    macrothread_start_thread(group->thread, __group_thread, group);
}

void embedded_event_group_stop(embedded_event_group_t group)
{
    group->cancel_token = true;
    macrothread_condition_signal(group->sync_point);
    macrothread_join(group->thread);
}

void embedded_event_group_destroy(embedded_event_group_t group)
{
    // Clear the queues
    embedded_event_registration_clear(&group->add_queue);
    embedded_event_registration_clear(&group->remove_queue);
    embedded_event_registration_clear(&group->handlers);
    embedded_event_t evt = embedded_event_pop(&group->event_queue);
    while(evt != NULL) {
        free(evt);
        evt = embedded_event_pop(&group->event_queue);
    }
    macrothread_mutex_destroy(group->registration_mutex);
    macrothread_mutex_destroy(group->event_mutex);
    macrothread_condition_destroy(group->sync_point);
    free(group);
}