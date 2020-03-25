#include "embedded-event.h"
#include "test.h"
#include <string>
#if WIN32
#include <windows.h>
#define usleep(a) Sleep(a)
#else
#include <unistd.h>
#endif

#if defined EMBEDDED_EVENT_PTHREADS || defined EMBEDDED_EVENT_CPP11
#define TEST_USE_SEPERATE_THREAD
#endif

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

void handler(void* context, const char* name, const int32_t event, const void* data)
{
    TEST_STRING_EQUAL((const char*)context, test_context);
    TEST_STRING_EQUAL(name, "TEST");
    TEST_STRING_EQUAL(data_string, (const char*)data);
    TEST_EQUAL(test_event, event);
    handler_count++;
    #ifdef EMBEDDED_EVENT_OMP
    test.stop();
    #endif
}

#ifdef ESP_PLATFORM
int app_main(void)
#else
int main(void)
#endif
{
    // Verify the starting condition
    TEST_EQUAL(handler_count, 0);

    #ifdef TEST_USE_SEPERATE_THREAD
    // Start the task
    test.run();
    #endif

    // Subscribe to an event
    event::registration reg = test.add(test_event, handler, (void*)test_context);

    // Publish the event
    test.post(test_event, (void*)data_string, 13 * sizeof(char));

    #ifdef TEST_USE_SEPERATE_THREAD
    // Wait for the event to be processed
    usleep(100);
    #elif defined EMBEDDED_EVENT_OMP
    test.run();
    #else
    test.dispatch();
    #endif

    // Verify the event was processed
    TEST_EQUAL(handler_count, 1);

    #ifdef EMBEDDED_EVENT_OMP
    exit(0);
    #endif

    // Publish a different event
    test.post(test_event + 1, NULL, 0);

    #ifdef TEST_USE_SEPERATE_THREAD
    // Wait for the event to be processed
    usleep(100);
    #else
    test.dispatch();
    #endif

    // Verify the handler is not called
    TEST_EQUAL(handler_count, 1);

    // Post the event twice
    test.post(test_event, (void*)data_string, 13 * sizeof(char));
    test.post(test_event, (void*)data_string, 13 * sizeof(char));

    #ifdef TEST_USE_SEPERATE_THREAD
    // Wait for the event to be processed
    usleep(100);
    #else
    test.dispatch();
    #endif

    // Verify the handler was called
    TEST_EQUAL(handler_count, 3);

    // Unsubscribe from the event
    test.remove(reg);

    // Post an event
    test.post(test_event, (void*)data_string, 13 * sizeof(char));

    #ifdef TEST_USE_SEPERATE_THREAD
    // Wait for the event to be processed
    usleep(100);
    #else
    test.dispatch();
    #endif

    // Verify the handler was not called
    TEST_EQUAL(handler_count, 3);

    #ifdef TEST_USE_SEPERATE_THREAD
    // Stop the task
    test.stop();
    #endif
}