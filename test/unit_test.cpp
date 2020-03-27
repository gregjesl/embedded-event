#include "embedded-event.h"
#include "test.h"
#include <string>
#if WIN32
#include <chrono>
#include <thread>
#define usleep(a) std::this_thread::sleep_for(std::chrono::microseconds(a));
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
unsigned int handler_count_2 = 0;

// Shutdown signal
bool shutdown_signal = false;

void handler(void* context, const char* name, const int32_t event, const void* data)
{
    TEST_STRING_EQUAL((const char*)context, test_context);
    TEST_STRING_EQUAL(name, "TEST");
    TEST_STRING_EQUAL(data_string, (const char*)data);
    TEST_EQUAL(test_event, event);
    handler_count++;
    if(handler_count < 100) {
        test.post(test_event, (void*)data_string, 13 * sizeof(char));
    } else {
        test.stop();
    }
}

void handler2(void* context, const char* name, const int32_t event, const void* data)
{
    TEST_STRING_EQUAL((const char*)context, test_context);
    TEST_STRING_EQUAL(name, "TEST");
    TEST_STRING_EQUAL(data_string, (const char*)data);
    TEST_EQUAL(test_event + 1, event);
    handler_count_2++;
}

event::registration reg(test_event, handler, (void*)test_context);
event::registration reg2(test_event + 1, handler2, (void*)test_context);

#if defined ESP_PLATFORM
void event_pump()
#elif defined EMBEDDED_EVENT_PTHREADS
void *event_pump(void*)
#else
void event_pump()
#endif
{
    while(!shutdown_signal) {
        test.post(test_event + 1, (void*)data_string, 13 * sizeof(char));
        test.post(test_event + 2, (void*)data_string, 13 * sizeof(char));
        usleep(10);
    }

    #if EMBEDDED_EVENT_PTHREADS
    return NULL;
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
    TEST_EQUAL(handler_count_2, 0);

    // Subscribe to an event
    test.add(reg);
    test.add(reg2);

    // Publish the first event
    test.post(test_event, (void*)data_string, 13 * sizeof(char));

    // Run the dispatcher
    test.run();

    // Wait for the conclusion
    test.join();

    // Verify the result
    TEST_TRUE(handler_count >= 100);
    TEST_EQUAL(handler_count_2, 0);

    // Clear any pending events
    test.clear_events();

    // This concludes the test for no threading and OpenMP
    #if defined EMBEDDED_EVENT_NO_THREADING || defined EMBEDDED_EVENT_OMP
    exit(0);
    #else

    // Start a message pump
    #if defined ESP_PLATFORM
    xTaskCreatePinnedToCore(
        event_pump, 
        "EVENT-PUMP", 
        2048, 
        NULL,
        5, 
        NULL, 
        0
    );
    #elif defined EMBEDDED_EVENT_PTHREADS
    pthread_t task_handle;
    pthread_create(&task_handle, NULL, event_pump, NULL);
    #elif defined EMBEDDED_EVENT_CPP11
    std::thread task_handle(event_pump);
    #endif

    // Start processing messages
    test.run();

    // Test the wait_any function
    {
        const int32_t waiters[2] = { test_event, test_event + 1};
        for(size_t i = 0; i < 10; i++) {
            test.wait_any(waiters, 2);
        }
    }
    

    // Test the wait_all function
    {
        const int32_t waiters[2] = { test_event + 1, test_event + 2};
        for(size_t i = 0; i < 10; i++) {
            test.wait_all(waiters, 2);
        }
    }

    // Stop the message pump
    shutdown_signal = true;

    // Stop the event group
    test.stop();

    // Wait for the test group to finish
    test.join();

    // Verify the result
    TEST_TRUE(handler_count_2 >= 10);

    // Shut down
    #if defined EMBEDDED_EVENT_PTHREADS
    pthread_join(task_handle, NULL);
    #elif defined EMBEDDED_EVENT_CPP11
    task_handle.join();
    #endif

    #endif
}