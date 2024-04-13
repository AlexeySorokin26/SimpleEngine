#include "SimpleEngineCore/Window.hpp"
#include "SimpleEngineCore/Log.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace  SimpleEngine {
    
    static bool GLFW_initialized = false;

    Window::Window(std::string title, const unsigned int width, const unsigned int height) 
        : data({std::move(title), width, height}){
        int resCode = init();
    }

    Window::~Window(){
        shutdown();
    }

    int Window::init(){

        LOG_INFO("Creating window");
        
        /* Initialize the library */
        if(!GLFW_initialized){
            if (!glfwInit()){
                LOG_CRITICAL("Can't initialized glfw.");
                return -1;
            }
            GLFW_initialized = true;
        }

        /* Create a windowed mode window and its OpenGL context */
        window = glfwCreateWindow(data.width, data.height, data.name.c_str(), nullptr, nullptr);
        if(!window)
        {
            glfwTerminate();
            LOG_CRITICAL("Can't create window.");
            return -2;
        }
        

        /* Make the window's context current */
        glfwMakeContextCurrent(window);

        // Link all possible opengl functions
        // GLADloadproc is a loader. We can use it from glad or from glfw. Here we are using glfw one.
        // GLADloadproc takes name of function and returns address of this function
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
            LOG_CRITICAL("Failed to initialize GLAD");
            return -1;
        }

        glfwSetWindowUserPointer(window, &data); // give handler pointer to that ever data we want to use inside callback function
        glfwSetWindowSizeCallback(window, 
            [](GLFWwindow* pwindow, int width, int height)
            {
                WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(pwindow));
                data.width = width;
                data.height = height;

                EventWindowResize event(width, height); 
                data.eventCallbackFn(event);
            }
        );

        glfwSetCursorPosCallback(window, 
            [](GLFWwindow* pwindow, double x, double y)
            {
                WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(pwindow));

                EventMouseMoved event(x, y); 
                data.eventCallbackFn(event);
            }
        );

        glfwSetWindowCloseCallback(window, 
            [](GLFWwindow* pwindow)
            {
                WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(pwindow));

                EventWindowClose event; 
                data.eventCallbackFn(event);
            }
        );

        return 0;
    }

    void Window::shutdown(){
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::on_update(){
        glClearColor(1,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

} // namespace  SimpleEngineCore