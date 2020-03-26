#ifndef EMBEDDED_EVENT_BARRIER_H
#define EMBEDDED_EVENT_BARRIER_H

#if defined ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#define BARRIER_RELEASED (1 << 0)
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
        #ifdef EMBEDDED_EVENT_OMP
        void wait(bool *flag);
        #endif
        void signal();
    private:
        bool is_released;
        #if defined ESP_PLATFORM
        EventGroupHandle_t p_event_group;
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