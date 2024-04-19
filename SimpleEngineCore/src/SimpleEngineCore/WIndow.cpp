#include "SimpleEngineCore/Window.hpp"
#include "SimpleEngineCore/Log.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define IMGUI_IMPL_OPENGL_SHADER_VERSION "#version 130"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

namespace  SimpleEngine {

	static bool GLFW_initialized = false;

	Window::Window(std::string title, const unsigned int width, const unsigned int height)
		: data({ std::move(title), width, height }) {
		int resCode = init();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplOpenGL3_Init();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
	}

	Window::~Window() {
		shutdown();
	}

	int Window::init() {

		LOG_INFO("Creating window");

		/* Initialize the library */
		if (!GLFW_initialized) {
			if (!glfwInit()) {
				LOG_CRITICAL("Can't initialized glfw.");
				return -1;
			}
			GLFW_initialized = true;
		}

		/* Create a windowed mode window and its OpenGL context */
		window = glfwCreateWindow(data.width, data.height, data.name.c_str(), nullptr, nullptr);
		if (!window)
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
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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

	void Window::shutdown() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::on_update() {
		glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = static_cast<float>(get_width());
		io.DisplaySize.y = static_cast<float>(get_height());

		ImGui_ImplOpenGL3_NewFrame(); // create fram where we will draw
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow(); // create data 

		ImGui::Begin("Background Color Window"); // create window
		ImGui::ColorEdit4("Background Color", backgroundColor);
		ImGui::End();

		ImGui::Render(); // draw data using imgui 
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // use our data to draw using opengl for example

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

} // namespace  SimpleEngineCore