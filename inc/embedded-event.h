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

#include <stdlib.h>
#include <cstdint>
#include <deque>
#include <vector>
#include "embedded-event-mutex.h"

/*! \brief Namespace for embedded events */
namespace event
{
    typedef void(handler_fun)(void*, const char*, int32_t, void*);

    struct callback
    {
        handler_fun *handler;
        void* context;
    };

    struct registration : callback
    {
        int32_t event;
    };

    struct event_map
    {
        int32_t event_id;
        std::vector<callback> callbacks;
        event::mutex mutex;
    };

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
        registration add(int32_t event, handler_fun handler, void* context);
        void remove(registration reg);
        void post(int32_t event, const void* data, const size_t data_length);
        void dispatch();
        void run();
        void stop();

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
        
        std::vector<event_map> handlers;
        void process_handler_changes();

        event_map* find_map(int32_t event_id);

        #if defined ESP_PLATFORM || EMBEDDED_EVENT_PTHREADS
        static void *run_task(void *arg);
        #else
        void run_task();
        #endif

        bool cancellation_requested;
        bool cancellation_acknowledged;
        void signal();

        #if defined ESP_PLATFORM
        TaskHandle_t task_handle;
        UBaseType_t p_priority;
        uint32_t p_depth;
        BaseType_t p_core;
        #elif defined EMBEDDED_EVENT_PTHREADS
        pthread_t task_handle;
        pthread_mutex_t p_condition_mutex;
        pthread_cond_t p_condition;
        #elif defined EMBEDDED_EVENT_CPP11
        std::thread *task_handle;
        std::mutex p_condition_mutex;
        std::condition_variable p_condition;
        #endif
    };
}

#endif