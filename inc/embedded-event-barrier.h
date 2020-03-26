#ifndef EMBEDDED_EVENT_BARRIER_H
#define EMBEDDED_EVENT_BARRIER_H

#if defined ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#elif defined EMBEDDED_EVENT_PTHREADS
#include <pthread.h>
#elif defined EMBEDDED_EVENT_CPP11
#include <mutex>
#include <condition_variable>
#endif

namespace event
{
    class barrier
    {
    public:
        barrier();
        virtual ~barrier();
        void wait();
        void signal();
    private:
        bool is_released;
        #if defined ESP_PLATFORM
        TaskHandle_t task_handle;
        #elif defined EMBEDDED_EVENT_PTHREADS
        pthread_mutex_t p_mutex;
        pthread_cond_t p_condition;
        #elif defined EMBEDDED_EVENT_CPP11
        std::mutex p_mutex;
        std::condition_variable p_condition;
        #endif
    };
}

#endif