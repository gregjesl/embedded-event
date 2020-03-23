#include "embedded-event.h"
#include <string.h>

#ifndef ESP_PLATFORM

event::wrapper::wrapper(const int32_t event, const void *data, const size_t data_length)
:   event_id(event)
{
    this->data = malloc(data_length);
    memcpy(this->data, data, data_length);
}

event::wrapper::~wrapper()
{
    free(this->data);
}

#endif