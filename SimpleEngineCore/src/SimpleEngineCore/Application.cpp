#include "SimpleEngineCore/Application.hpp"
#include "SimpleEngineCore/Log.hpp"
#include "SimpleEngineCore/Window.hpp"

#include <iostream>

namespace SimpleEngine{

    Application::Application(){
        LOG_INFO("Starting Application");
    }

    Application::~Application(){
        LOG_INFO("Closing Application");
    }
        
    int Application::start(unsigned int windowWidth, unsigned int windowHeight, const char* title){
        window = std::make_unique<Window>(title, windowWidth, windowHeight);

        event_dispatcher.add_event_listener<EventMouseMoved>(
            [](EventMouseMoved& event){
                LOG_INFO("[MouseMoved] Mouse moved to {0}x{1}", event.x, event.y);
            }
        );
        event_dispatcher.add_event_listener<EventWindowResize>(
            [](EventWindowResize& event){
                LOG_INFO("[WindowResize] Window resized to {0}x{1}", event.height, event.width);
            }
        );
        event_dispatcher.add_event_listener<EventWindowClose>(
            [&](EventWindowClose& event){
                LOG_INFO("[WindowClose] Window closed");
                closedWindow = true;
            }
        );
        window->set_event_callback(
            [&](BaseEvent& event){
                event_dispatcher.dispatch(event);
            }
        );

        while(!closedWindow){
            window->on_update();
            on_update();
        }
    }

    void Application::on_update(){

    }

}