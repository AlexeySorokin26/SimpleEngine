#pragma once

#include "SimpleEngineCore/Event.hpp"

#include <string> 
#include <functional>

struct GLFWwindow; 

namespace SimpleEngine{

    class Window{

    public:
        using EventCallBackFn = std::function<void(BaseEvent&)>;

        Window(std::string title, const unsigned int width, const unsigned int height);

        ~Window();
        
        Window(const Window&) = delete;
        Window(const Window&&) = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = delete;

        void on_update();

        unsigned int get_width() const { return data.width; }
        unsigned int get_height() const { return data.height; }

        void set_event_callback(const EventCallBackFn& callback) { data.eventCallbackFn = callback; }

    private:
        struct WindowData{
            std::string name;
            unsigned int width;
            unsigned int height;
            EventCallBackFn eventCallbackFn;
        };
        int init();
        void shutdown();

        GLFWwindow* window = nullptr;
        WindowData data;
    };

}