#include "embedded_event_registration.h"
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
    embedded_event_registration_t reg = embedded_event_registration_init(11);
    TEST_EQUAL(reg->event, 11);
    TEST_NULL(reg->next);

    embedded_event_registration_t list = NULL;
    embedded_event_registration_append(&list, reg);
    TEST_NOT_NULL(list);

    embedded_event_registration_t other = embedded_event_registration_init(12);
    embedded_event_registration_append(&list, other);
    TEST_EQUAL(list->next, other);

    TEST_EQUAL(embedded_event_registration_find(&list, 11), reg);
    TEST_EQUAL(embedded_event_registration_find(&list, 12), other);

    embedded_event_registration_remove(&list, other);
    TEST_NULL(embedded_event_registration_find(&list, 12));
    TEST_EQUAL(embedded_event_registration_find(&list, 11), reg);

    embedded_event_registration_remove(&list, reg);
    TEST_NULL(embedded_event_registration_find(&list, 11));
    TEST_NULL(embedded_event_registration_find(&list, 12));
    TEST_NULL(list);

    embedded_event_registration_clear(&other);
    TEST_NULL(other);
    embedded_event_registration_destroy(reg);
}