#ifndef EMBEDDED_EVENT_H
#define EMBEDDED_EVENT_H

#define EMBEDDED_EVENT_HANDLER(function, event, data, context) void function (int32_t event, void* data, void* context)

#include <stdlib.h>
#include "embedded_event_registration.h"
#include "macrothreading_thread.h"
#include "macrothreading_mutex.h"
#include "macrothreading_condition.h"

typedef void(event_posted_callback_t)(int32_t, void *);

typedef struct embedded_event_struct
{
    int32_t event;
    void *data;
    event_posted_callback_t *callback;
    struct embedded_event_struct *next;
} *embedded_event_t;

void embedded_event_push(embedded_event_t *first, int32_t event, void *data, event_posted_callback_t callback);
embedded_event_t embedded_event_pop(embedded_event_t *first);

typedef struct embedded_event_group_struct
{
    embedded_event_registration_t add_queue;
    embedded_event_registration_t remove_queue;
    embedded_event_registration_t handlers;
    macrothread_mutex_t registration_mutex;
    embedded_event_t event_queue;
    macrothread_mutex_t event_mutex;
    macrothread_condition_t sync_point;
    macrothread_handle_t thread;
    bool cancel_token;
} *embedded_event_group_t;

embedded_event_group_t embedded_event_group_init();
embedded_event_callback_t embedded_event_group_add(embedded_event_group_t group, int32_t event, embedded_event_handler_t handler, void *context, macrothread_condition_t signal);
void embedded_event_group_remove(embedded_event_group_t group, int32_t event, embedded_event_callback_t callback, macrothread_condition_t signal);
void embedded_event_group_post(embedded_event_group_t group, int32_t event, void *data, event_posted_callback_t *callback);
void embedded_event_group_start(embedded_event_group_t group);
void embedded_event_group_stop(embedded_event_group_t group);
void embedded_event_group_destroy(embedded_event_group_t group);

#endif