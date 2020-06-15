#include "embedded_event_registration.h"
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

embedded_event_registration_t embedded_event_registration_init(int32_t event)
{
    embedded_event_registration_t result = (embedded_event_registration_t)malloc(sizeof(struct embedded_event_registration_struct));
    result->event = event;
    result->callbacks = NULL;
    result->next = NULL;
    return result;
}

void embedded_event_registration_append(embedded_event_registration_t *first, embedded_event_registration_t addition)
{
    // Validate the input
    if(addition == NULL && first == NULL) return;

    // Check for an empty list
    if(*first == NULL) {
        *first = addition;
        return;
    }

    // Iterate through the list
    embedded_event_registration_t current = *first;
    embedded_event_registration_t next = NULL;
    while(current->next != NULL) {
        next = current->next;
        current = next;
    }

    // Store the addition
    current->next = addition;
}

void embedded_event_registration_remove(embedded_event_registration_t *first, embedded_event_registration_t removal)
{
    // Validate the input
    if(first == NULL || removal == NULL) return;

    // Check for list of 1
    if(*first == removal) {
        *first = removal->next;
        return;
    }

    embedded_event_registration_t parent = *first;
    embedded_event_registration_t current = (*first)->next;
    embedded_event_registration_t next = NULL;
    while(current != NULL) {

        // Check for a match
        if(current == removal) {
            parent->next = current->next;
            current->next = NULL;
            return;
        }

        next = current->next;
        parent = current;
        current = next;
    }
}

embedded_event_registration_t embedded_event_registration_find(const embedded_event_registration_t *first, int32_t event)
{
    if(first == NULL || *first == NULL) return NULL;

    embedded_event_registration_t current = *first;
    embedded_event_registration_t next = NULL;
    while(current != NULL) {
        if(current->event == event) {
            return current;
        }

        next = current->next;
        current = next;
    }
    return NULL;
}

void embedded_event_registration_clear(embedded_event_registration_t *first)
{
    if(first == NULL || *first == NULL) return;

    embedded_event_registration_t current = *first;
    embedded_event_registration_t next = NULL;
    do {
        next = embedded_event_registration_destroy(current);
        current = next;
    } while(current != NULL);

    *first = NULL;
}

embedded_event_registration_t embedded_event_registration_destroy(embedded_event_registration_t reg)
{
    embedded_event_registration_t result = reg->next;
    free(reg);
    return result;
}

#ifdef __cplusplus
}
#endif