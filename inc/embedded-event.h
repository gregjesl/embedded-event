#ifndef EMBEDDED_EVENT_H
#define EMBEDDED_EVENT_H
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
        #ifdef ESP_PLATFORM
        esp_event_handler_instance_t instance;
        #endif
    };

    #ifndef ESP_PLATFORM
    struct event_map
    {
        int32_t event_id;
        std::vector<callback> callbacks;
    };

    class container
    {
    public:
        container(const int32_t event, const void *data, const size_t data_length);
        virtual ~container();
        const int32_t event_id;
        void *data;
    };
    #endif

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
        #ifdef ESP_PLATFORM
        void set_dispatch_timeout(const unsigned int ms_timeout);
        #endif
    private:
        const char* name;
        #ifdef ESP_PLATFORM
        esp_event_loop_handle_t loop_handle;
        unsigned int ms_timetout;
        #else
        std::deque<registration> add_queue;
        std::deque<registration> remove_queue;
        event::mutex registration_mutex;

        std::deque<container*> event_queue;
        event::mutex event_mutex;
        
        std::vector<event_map> handlers;
        void process_handler_changes();

        event_map* find_map(int32_t event_id);
        #endif
    };
}

#endif