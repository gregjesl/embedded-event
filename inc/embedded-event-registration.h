#ifndef EMBEDDED_EVENT_REGISTRATION_H
#define EMBEDDED_EVENT_REGISTRATION_H

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include "embedded-event-mutex.h"

namespace event
{
    typedef void(handler_fun)(void*, const char*, const int32_t, const void*);

    class callback
    {
    public:
        callback();
        callback(const callback &other);
        callback(handler_fun fun, void *context = NULL);
        void raise(const char *name, const int32_t event, const void* data);
        inline bool operator==(const callback &rhs) {
            return this->handler == rhs.handler && this->context == rhs.context;
        }
        inline bool operator!=(const callback &rhs) {
            return !operator==(rhs);
        }
    private:
        handler_fun *handler;
        void* context;
    };

    class registration : public callback
    {
    public:
        registration();
        registration(const registration &other);
        registration(const int32_t event, handler_fun fun, void *context = NULL);
        inline bool operator==(const registration &rhs) {
            return this->p_event == rhs.p_event && callback::operator==(rhs);
        }
        inline bool operator!=(const registration &rhs) {
            return !operator==(rhs);
        }
        inline int32_t event() const { return this->p_event; }
    private:
        int32_t p_event;
    };
}

#endif