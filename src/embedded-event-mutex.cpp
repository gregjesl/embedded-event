#include "embedded-event-mutex.h"

#ifndef EMBEDDED_EVENT_CPP11

event::mutex::mutex()
{
    #if defined EMBEDDED_EVENT_ESP32
    this->p_mutex = xSemaphoreCreateMutex();
    #elif defined EMBEDDED_EVENT_PTHREADS
    pthread_mutex_init(&this->p_mutex, NULL);
    #elif defined EMBEDDED_EVENT_OMP
    omp_init_lock(&this->p_mutex);
    #elif defined EMBEDDED_EVENT_NO_THREADING
    this->is_locked = false;
    #else
    #error Threading approach not defined
    #endif
}

event::mutex::~mutex()
{
    #if defined EMBEDDED_EVENT_ESP32
    vSemaphoreDelete(this->p_mutex);
    #elif defined EMBEDDED_EVENT_PTHREADS
    pthread_mutex_destroy(&this->p_mutex);
    #elif defined EMBEDDED_EVENT_OMP
    omp_destroy_lock(&this->p_mutex);
    #elif defined EMBEDDED_EVENT_NO_THREADING
    // Do nothing
    #else
    #error Threading approach not defined
    #endif
}

void event::mutex::lock()
{
    #if defined EMBEDDED_EVENT_ESP32
    xSemaphoreTake(this->p_mutex, portMAX_DELAY);
    #elif defined EMBEDDED_EVENT_PTHREADS
    pthread_mutex_lock(&this->p_mutex);
    #elif defined EMBEDDED_EVENT_OMP
    omp_set_lock(&this->p_mutex);
    #elif defined EMBEDDED_EVENT_NO_THREADING
    if(this->is_locked) throw std::runtime_error("Deadlock");
    this->is_locked = true;
    #else
    #error Threading approach not defined
    #endif
}

void event::mutex::unlock()
{
    #if defined EMBEDDED_EVENT_ESP32
    xSemaphoreGive(this->p_mutex);
    #elif defined EMBEDDED_EVENT_PTHREADS
    pthread_mutex_unlock(&this->p_mutex);
    #elif defined EMBEDDED_EVENT_OMP
    omp_unset_lock(&this->p_mutex);
    #elif defined EMBEDDED_EVENT_NO_THREADING
    if(!this->is_locked) throw std::runtime_error("Mutex not locked");
    this->is_locked = false;
    #else
    #error Threading approach not defined
    #endif
}

#endif