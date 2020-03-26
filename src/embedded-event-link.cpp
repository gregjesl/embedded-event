#include "embedded-event-link.h"

event::link::link(const int32_t event)
:   event(event)
{

}

void event::link::add(event::callback handler)
{
    // Check for an existing handler
    for(size_t i = 0; i < this->handlers.size(); i++) {
        
        // Compare the handlers
        if(this->handlers.at(i) == handler) 
            return;
    }

    // Add the handler
    this->handlers.push_back(handler);
}

void event::link::add(event::barrier *trigger)
{
    this->triggers.push_back(trigger);
}

void event::link::remove(event::callback handler)
{
    // Iterate through all handlers
    size_t i = 0;
    while(i < this->handlers.size()) {

        // Compare the handlers
        if(this->handlers.at(i) == handler) {

            // Remove the handler
            this->handlers.erase(this->handlers.begin() + i);
        } else {

            // Move to the next handler
            i++;
        }
    }
}

void event::link::process(const char *name, void *data)
{
    // Iterate through all callbacks
    for(size_t i = 0; i < this->handlers.size(); i++) {
        this->handlers.at(i).raise(name, this->event, data);
    }

    // Iterate through all triggers
    for(size_t i = 0; i < this->triggers.size(); i++) {
        this->triggers.at(i)->signal();
    }

    // Clear the triggers
    this->triggers.clear();
}
