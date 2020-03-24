#ifndef EMBEDDED_EVENT_MUTEX_H
#define EMBEDDED_EVENT_MUTEX_H

#if defined ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#elif defined EMBEDDED_EVENT_PTHREADS
#include <pthread.h>
#elif defined EMBEDDED_EVENT_CPP11
#include <mutex>
#elif defined EMBEDDED_EVENT_OMP
#include <omp.h>
#elif defined EMBEDDED_EVENT_NO_THREADING
#include <stdexcept>
#include <assert.h>
#else
#warning Threading approach not set, defaulting to NO_THREADING
#define EMBEDDED_EVENT_NO_THREADING
#endif

namespace event
{
    #ifdef EMBEDDED_EVENT_CPP11
    typedef std::mutex mutex;
    #else
    class mutex
    {
    public:
        mutex();
        virtual ~mutex();
        void lock();
        void unlock();
    private:
        #if defined ESP_PLATFORM
        SemaphoreHandle_t p_mutex;
        #elif defined EMBEDDED_EVENT_PTHREADS
        pthread_mutex_t p_mutex;
        #elif defined EMBEDDED_EVENT_OMP
        omp_lock_t p_mutex;
        #elif defined EMBEDDED_EVENT_NO_THREADING
        bool is_locked;
        #else
        #error Threading approach not defined
        #endif
    };
    #endif
}

#endif