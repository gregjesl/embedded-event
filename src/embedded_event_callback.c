#include "embedded_event_callback.h"
#include <string.h>
#include <assert.h>

embedded_event_callback_t embedded_event_callback_init(embedded_event_handler_t handler, void *context)
{
    embedded_event_callback_t result = (embedded_event_callback_t)malloc(sizeof(struct embedded_event_callback_struct));
    result->handler = handler;
    result->context = context;
    result->next = NULL;
    return result;
}

void embedded_event_callback_append(embedded_event_callback_t *first, embedded_event_callback_t addition)
{
    // Validate the input
    if(addition == NULL && first == NULL) return;

    // Check for an empty list
    if(*first == NULL) {
        *first = addition;
        (*first)->next = NULL;
        return;
    }

    // Iterate through the list
    embedded_event_callback_t current = *first;
    embedded_event_callback_t next = NULL;
    while(current->next != NULL) {
        assert(current->next != current);
        next = current->next;
        current = next;
    }

    // Store the addition
    current->next = addition;
}

void embedded_event_callback_remove(embedded_event_callback_t *first, embedded_event_callback_t removal)
{
    // Validate the input
    if(first == NULL || removal == NULL) return;

    // Check for list of 1
    if(*first == removal) {
        *first = removal->next;
        return;
    }

    embedded_event_callback_t parent = *first;
    embedded_event_callback_t current = (*first)->next;
    embedded_event_callback_t next = NULL;
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

void embedded_event_callback_clear(embedded_event_callback_t *first)
{
    if(first == NULL || *first == NULL) return;

    embedded_event_callback_t current = *first;
    embedded_event_callback_t next = NULL;
    do {
        next = embedded_event_callback_destroy(current);
        current = next;
    } while(current != NULL);

    *first = NULL;
}

embedded_event_callback_t embedded_event_callback_destroy(embedded_event_callback_t reg)
{
    embedded_event_callback_t result = reg->next;
    free(reg);
    return result;
}