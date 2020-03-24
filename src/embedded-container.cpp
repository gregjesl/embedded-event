#include "embedded-event.h"
#include <string.h>

event::container::container(const int32_t event, const void *data, const size_t data_length)
:   event_id(event)
{
    this->data = malloc(data_length);
    memcpy(this->data, data, data_length);
}

event::container::~container()
{
    free(this->data);
}
