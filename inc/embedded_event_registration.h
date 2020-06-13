#ifndef EMBEDDED_EVENT_REGISTRATION_H
#define EMBEDDED_EVENT_REGISTRATION_H

#include <stdlib.h>
#include "embedded_event_callback.h"
#include "macrothreading_condition.h"

typedef struct embedded_event_registration_struct
{
    int32_t event;
    embedded_event_callback_t callbacks;
    macrothread_condition_t signal;
    struct embedded_event_registration_struct *next;
} *embedded_event_registration_t;

embedded_event_registration_t embedded_event_registration_init(int32_t event);
void embedded_event_registration_append(embedded_event_registration_t *first, embedded_event_registration_t addition);
void embedded_event_registration_remove(embedded_event_registration_t *first, embedded_event_registration_t removal);
embedded_event_registration_t embedded_event_registration_find(const embedded_event_registration_t *first, int32_t event);
void embedded_event_registration_clear(embedded_event_registration_t *first);
embedded_event_registration_t embedded_event_registration_destroy(embedded_event_registration_t reg);

#endif