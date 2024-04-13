#pragma once

#include "SimpleEngineCore/Event.hpp"

#include <memory>

namespace SimpleEngine{

class Window;

    class Application{
    public:
        Application();

        virtual ~Application();
        
        Application(const Application&) = delete;
        Application(const Application&&) = delete;
        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&) = delete;

        virtual int start(unsigned int windowWidth, unsigned int windowHeight, const char* title);

        virtual void on_update();
    private:
        std::unique_ptr<Window> window;

        EventDispatcher event_dispatcher;
        bool closedWindow = false;
    };

}