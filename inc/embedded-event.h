#ifndef EMBEDDED_EVENT_H
#define EMBEDDED_EVENT_H

#if defined ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#elif defined EMBEDDED_EVENT_PTHREADS
#include <pthread.h>
#elif defined EMBEDDED_EVENT_CPP11
#include <thread>
#include <mutex>
#include <condition_variable>
#elif defined EMBEDDED_EVENT_OMP
#include <omp.h>
#elif defined EMBEDDED_EVENT_NO_THREADING
// Do nothing
#else
#warning Threading approach not set, defaulting to NO_THREADING
#define EMBEDDED_EVENT_NO_THREADING
#endif

#define EMBEDDED_EVENT_HANDLER(function, context, group, event, data) void function (void* context, const char* group, const int32_t event, const void* data)

#include <stdlib.h>
#include <cstdint>
#include <deque>
#include <vector>
#include "embedded-event-mutex.h"
#include "embedded-event-registration.h"
#include "embedded-event-barrier.h"
#include "embedded-event-link.h"

/*! \brief Namespace for embedded events */
namespace event
{
    class container
    {
    public:
        container(const int32_t event, const void *data, const size_t data_length);
        virtual ~container();
        const int32_t event_id;
        void *data;
    };

    /*! \brief Group of events */
    class group
    {
    public:
        /*! \brief Constructor
         */
        group(const char* name);
        virtual ~group();
        void add(const registration reg);
        registration add(int32_t event, handler_fun handler, void* context);
        void remove(registration reg);
        void post(int32_t event, const void* data, const size_t data_length);
        void wait_for(int32_t event);
        void wait_any(const int32_t *events, const size_t num_events);
        void wait_all(const int32_t *events, const size_t num_events);
        void dispatch();
        void run();
        void stop();
        void clear_events();
        void join();

        #if defined ESP_PLATFORM
        void set_task_priority(const UBaseType_t priority);
        void set_stack_depth(const uint32_t depth);
        void set_task_core(const BaseType_t core);
        #endif
    private:
        const char* name;
        std::deque<registration> add_queue;
        std::deque<registration> remove_queue;
        event::mutex registration_mutex;

        std::deque<container*> event_queue;
        event::mutex event_mutex;
        
        std::vector<link*> handlers;
        void process_handler_changes();

        link* find_link(int32_t event_id);

        #if defined ESP_PLATFORM
        static void run_task(void *arg);
        #elif defined EMBEDDED_EVENT_PTHREADS
        static void *run_task(void *arg);
        #else
        void run_task();
        #endif

        bool cancellation_requested;
        bool cancellation_acknowledged;
        barrier sync_point;

        #if defined ESP_PLATFORM
        TaskHandle_t task_handle;
        UBaseType_t p_priority;
        uint32_t p_depth;
        BaseType_t p_core;
        #elif defined EMBEDDED_EVENT_PTHREADS
        pthread_t task_handle;
        #elif defined EMBEDDED_EVENT_CPP11
        std::thread *task_handle;
        #endif
    };
}

#endif