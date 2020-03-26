#include "embedded-event.h"

#if defined ESP_PLATFORM
void event::group::run_task(void *arg)
#elif defined EMBEDDED_EVENT_PTHREADS
void *event::group::run_task(void *arg)
#else
void event::group::run_task()
#endif
{
    #if defined ESP_PLATFORM || EMBEDDED_EVENT_PTHREADS
    event::group *parent = (event::group*)arg;
    #else
    event::group *parent = this;
    #endif

    while(!parent->cancellation_requested)
    {
        // Dispatch events
        parent->dispatch();

        parent->sync_point.wait();
    }

    #ifndef EMBEDDED_EVENT_OMP
    parent->cancellation_acknowledged = true;
    #endif

    #if defined EMBEDDED_EVENT_PTHREADS
    return NULL;
    #endif
}

void event::group::run()
{
    this->cancellation_requested = false;
    this->cancellation_acknowledged = false;

    #if defined ESP_PLATFORM
    xTaskCreatePinnedToCore(
        event::group::run_task, 
        "EMBEDDED-EVENT-LOOP", 
        this->p_depth, 
        (void*)this, 
        this->p_priority, 
        &this->task_handle, 
        this->p_core
    );
    #elif defined EMBEDDED_EVENT_PTHREADS
    pthread_create(&this->task_handle, NULL, event::group::run_task, (void*)this);
    #elif defined EMBEDDED_EVENT_CPP11
    this->task_handle = new std::thread(&event::group::run_task, this);
    #elif defined EMBEDDED_EVENT_OMP
    #pragma omp parallel
    {
        this->run_task();
    }
    #elif defined EMBEDDED_EVENT_NO_THREADING
    this->run_task();
    #endif
}

void event::group::stop()
{
    // Set the cancellation flag
    this->cancellation_requested = true;

    // Signal the cancellation
    this->sync_point.signal();
}

void event::group::join()
{
    // Wait for the threads to shut down
    #if defined ESP_PLATFORM
    vTaskDelete(this->task_handle);
    #elif defined EMBEDDED_EVENT_PTHREADS
    pthread_join(this->task_handle, NULL);
    #elif defined EMBEDDED_EVENT_CPP11
    this->task_handle->join();
    delete this->task_handle;
    #elif EMBEDDED_EVENT_OMP
    // Do nothing
    #endif
}