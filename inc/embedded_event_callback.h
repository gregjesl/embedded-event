#ifndef EMBEDDED_EVENT_CALLBACK_H
#define EMBEDDED_EVENT_CALLBACK_H

#include <stdlib.h>
#include <stdint.h>

typedef void(embedded_event_handler_t)(int32_t, void*, void*);

typedef struct embedded_event_callback_struct
{
    embedded_event_handler_t *handler;
    void *context;
    struct embedded_event_callback_struct *next;
} *embedded_event_callback_t;

embedded_event_callback_t embedded_event_callback_init(embedded_event_handler_t handler, void *context);
void embedded_event_callback_append(embedded_event_callback_t *first, embedded_event_callback_t addition);
void embedded_event_callback_remove(embedded_event_callback_t *first, embedded_event_callback_t removal);
void embedded_event_callback_clear(embedded_event_callback_t *first);
embedded_event_callback_t embedded_event_callback_destroy(embedded_event_callback_t reg);

#endif