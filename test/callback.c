#include "embedded_event_callback.h"
#include "test.h"

void callback(int32_t event, void *data, void *context)
{
    TEST_TRUE(event);
    TEST_NOT_NULL(data);
    TEST_NOT_NULL(context);
}

char *context = "Hello world!";

int main(void)
{
    embedded_event_callback_t reg = embedded_event_callback_init(callback, context);
    TEST_EQUAL(reg->handler, callback);
    TEST_EQUAL(reg->context, context);
    TEST_NULL(reg->next);

    embedded_event_callback_t list = NULL;
    embedded_event_callback_append(&list, reg);
    TEST_NOT_NULL(list);

    embedded_event_callback_t other = embedded_event_callback_init(callback, context);
    embedded_event_callback_append(&list, other);
    TEST_EQUAL(list->next, other);

    embedded_event_callback_clear(&other);
    TEST_NULL(other);
    embedded_event_callback_destroy(reg);
}