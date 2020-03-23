#include "embedded-event.h"
#include "test.h"
#include <string>

// Create the event group
event::group test("TEST");

// Set the data string
const char* data_string = "Hello world!\0";

// Set the context
const char* test_context = "Context";

// Set the event id
int32_t test_event = 11;

// Handler count
unsigned int handler_count = 0;

void handler(void* context, const char* name, int32_t event, void* data)
{
    TEST_STRING_EQUAL((const char*)context, test_context);
    TEST_STRING_EQUAL(name, "TEST");
    TEST_STRING_EQUAL(data_string, (const char*)data);
    TEST_EQUAL(test_event, event);
    handler_count++;
}

int main(void)
{
    // Verify the starting condition
    TEST_EQUAL(handler_count, 0);

    // Subscribe to an event
    event::registration reg = test.add(test_event, handler, (void*)test_context);

    // Publish the event
    test.post(test_event, (void*)data_string, 13 * sizeof(char));

    // Run the dispatcher
    test.dispatch();

    // Verify the event was processed
    TEST_EQUAL(handler_count, 1);

    // Run the dispatcher again
    test.dispatch();

    // Verify the handler was not called
    TEST_EQUAL(handler_count, 1);

    // Publish a different event
    test.post(test_event + 1, NULL, 0);

    // Run the dispatcher
    test.dispatch();

    // Verify the handler is not called
    TEST_EQUAL(handler_count, 1);

    // Post the event twice
    test.post(test_event, (void*)data_string, 13 * sizeof(char));
    test.post(test_event, (void*)data_string, 13 * sizeof(char));

    // Run the dispatcher
    test.dispatch();

    // Verify the handler was called
    TEST_EQUAL(handler_count, 3);

    // Unsubscribe from the event
    test.remove(reg);

    // Post an event
    test.post(test_event, (void*)data_string, 13 * sizeof(char));

    // Run the dispatcher
    test.dispatch();

    // Verify the handler was not called
    TEST_EQUAL(handler_count, 3);
}