#ifndef EMBEDDED_EVENT_LINK_H
#define EMBEDDED_EVENT_LINK_H

#include "embedded-event-mutex.h"
#include "embedded-event-registration.h"
#include "embedded-event-barrier.h"
#include <vector>
#include <stdint.h>

namespace event
{
    class link
    {
    public:
        link(const int32_t event);
        const int32_t event;
        void add(event::callback handler);
        void add(event::barrier *checkpoint);
        void remove(event::callback cb);
        void process(const char* name, void* data);
        inline bool is_empty() const { return this->handlers.empty() && this->triggers.empty(); }
        inline void lock() { this->p_mutex.lock(); }
        inline void unlock() { this->p_mutex.unlock(); }
    private:
        std::vector<event::callback> handlers;
        std::vector<event::barrier*> triggers;
        event::mutex p_mutex;
    };
}

#endif