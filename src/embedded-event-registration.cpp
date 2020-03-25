#include "embedded-event-registration.h"

event::callback::callback()
:   handler(NULL), context(NULL)
{

}

event::callback::callback(const event::callback &other)
:   handler(other.handler), context(other.context)
{

}

event::callback::callback(event::handler_fun fun, void *context)
:   handler(fun), context(context)
{

}

void event::callback::raise(const char *name, const int32_t event, const void* data)
{
    if(this->handler) {
        this->handler(this->context, name, event, data);
    }
}

event::registration::registration()
:   event::callback()
{

}

event::registration::registration(const event::registration &other)
:   event::callback(other), p_event(other.p_event)
{

}

event::registration::registration(const int32_t event, handler_fun fun, void *context)
:   event::callback(fun, context), p_event(event)
{

}