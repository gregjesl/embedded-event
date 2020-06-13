#include "embedded-event.h"
#include "test.h"

// Create the event group
embedded_event_group_t test;

// Set the context
char* test_context = "Context";

// Set the condition variables
macrothread_condition_t sig1;
macrothread_condition_t sig2;
macrothread_condition_t donesig1;
macrothread_condition_t donesig2;

int32_t test_data = 1;
int32_t expected_event = 1;
int32_t max_event = 10;
int32_t message_count = 0;

macrothread_condition_t add_signal;
macrothread_condition_t remove_signal;
macrothread_condition_t data_signal;

void donehandler(int32_t event, void *data_ptr)
{
    int32_t data = *((int32_t*)data_ptr);
    TEST_EQUAL(event, expected_event);
    TEST_EQUAL(data, expected_event);
    TEST_EQUAL(event, message_count);
    message_count = 0;
    if(event == max_event)
        macrothread_condition_signal(data_signal);
    else {
        expected_event++;
        test_data = expected_event;
        embedded_event_group_post(test, expected_event, &test_data, donehandler);
    }
}

EMBEDDED_EVENT_HANDLER(handler, event, data_ptr, context)
{
    int32_t data = *((int32_t*)data_ptr);
    TEST_EQUAL(event, expected_event);
    TEST_EQUAL(data, expected_event);
    TEST_STRING_EQUAL((const char*)context, test_context);

    message_count++;
}

int main(void)
{
    // Create the event group
    test = embedded_event_group_init();

    // Initialize the signals
    add_signal = macrothread_condition_init();
    remove_signal = macrothread_condition_init();
    data_signal = macrothread_condition_init();

    // Start the event group
    embedded_event_group_start(test);

    // Store the callbacks
    embedded_event_callback_t *callbacks = (embedded_event_callback_t*)malloc(max_event * max_event * sizeof(struct embedded_event_callback_struct));
    embedded_event_callback_t *write_index = callbacks;

    // Register the callback
    for(int32_t i = 1; i <= max_event; i++) {
        for(int32_t j = 1; j <= i; j++) {
            if(i == max_event && j == max_event)
                *write_index = embedded_event_group_add(test, i, handler, test_context, add_signal);
            else
                *write_index = embedded_event_group_add(test, i, handler, test_context, NULL);
            
            write_index++;
        }
    }

    // Wait for the add
    macrothread_condition_wait(add_signal);

    // Triger the events
    embedded_event_group_post(test, 1, &test_data, donehandler);

    // Wait for completion
    macrothread_condition_wait(data_signal);

    // Remove the handlers
    embedded_event_callback_t *read_index = callbacks;
    for(int32_t i = 1; i <= max_event; i++) {
        for(int32_t j = 1; j <= i; j++) {
            if(i == max_event && j == max_event)
                embedded_event_group_remove(test, i, *read_index, remove_signal);
            else
                embedded_event_group_remove(test, i, *read_index, NULL);
            
            read_index++;
        }
    }
    free(callbacks);

    // Wait for removal
    macrothread_condition_wait(remove_signal);

    // Stop the event group
    embedded_event_group_stop(test);

    // Destroy the event group
    embedded_event_group_destroy(test);

    // Destory the signals
    macrothread_condition_destroy(add_signal);
    macrothread_condition_destroy(remove_signal);
    macrothread_condition_destroy(data_signal);
}