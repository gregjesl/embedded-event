#include "embedded-event-barrier.h"
#if WIN32
#include <chrono>
#include <thread>
#define usleep(a) std::this_thread::sleep_for(std::chrono::microseconds(a));
#else
#include <unistd.h>
#endif

event::barrier::barrier()
:   is_released(false)
{
    #if defined EMBEDDED_EVENT_PTHREADS
    pthread_mutex_init(&this->p_mutex, NULL);
    pthread_cond_init(&this->p_condition, NULL);
    #endif
}

event::barrier::~barrier()
{
    #if defined EMBEDDED_EVENT_PTHREADS
    pthread_mutex_destroy(&this->p_mutex);
    pthread_cond_destroy(&this->p_condition);
    #endif
}

void event::barrier::wait()
{
    #if defined ESP_PLATFORM
    TaskStatus_t xTaskDetails;
    vTaskGetInfo( /* The handle of the task being queried. */
                  NULL,
                  /* The TaskStatus_t structure to complete with information
                  on xTask. */
                  &xTaskDetails,
                  /* Include the stack high water mark value in the
                  TaskStatus_t structure. */
                  pdFALSE,
                  /* Include the task state in the TaskStatus_t structure. */
                  eRunning );

    this->task_handle = xTaskDetails.xHandle;
    while(!this->is_released)
        ulTaskNotifyTake(pdTRUE,         /* Clear the notification value before
                                            exiting. */
                        portMAX_DELAY ); /* Block indefinitely */
    this->is_released = false;
    #elif defined EMBEDDED_EVENT_PTHREADS
    pthread_mutex_lock(&this->p_mutex);
    while(!this->is_released)
        pthread_cond_wait(&this->p_condition, &this->p_mutex);
    this->is_released = false;
    pthread_mutex_unlock(&this->p_mutex);
    #elif defined EMBEDDED_EVENT_CPP11
    std::unique_lock<std::mutex> lk(this->p_mutex);
    while(!this->is_released)
        this->p_condition.wait(lk);
    this->is_released = false;
    lk.unlock();
    #else
    while(!this->is_released)
        usleep(1);
    this->is_released = false;
    #endif
}

#ifdef EMBEDDED_EVENT_OMP
void event::barrier::wait(bool *flag)
{
    while(!this->is_released && !(*flag))
        usleep(1);
    this->is_released = !(*flag);
}
#endif

void event::barrier::signal()
{
    #if defined ESP_PLATFORM
    this->is_released = true;
    xTaskNotifyGive(this->task_handle);
    #elif defined EMBEDDED_EVENT_PTHREADS
    pthread_mutex_lock(&this->p_mutex);
    this->is_released = true;
    pthread_cond_signal(&this->p_condition);
    pthread_mutex_unlock(&this->p_mutex);
    #elif defined EMBEDDED_EVENT_CPP11
    std::unique_lock<std::mutex> lk(this->p_mutex);
    this->is_released = true;
    this->p_condition.notify_all();
    #else
    this->is_released = true;
    #endif
}