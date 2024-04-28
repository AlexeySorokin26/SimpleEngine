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

	GLfloat points[] = {
		0.0f,  0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
	   -0.5f, -0.5f, 0.0f
	};

	GLfloat colors[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	// we run it for every vertex (every point)
	const char* vertex_shader =
		"#version 460\n"
		"layout(location = 0) in vec3 vertex_position;" // "data type": in/out vec3 
		"layout(location = 1) in vec3 vertex_color;" // what is layout? shader needs to find out input data 
		// which could be points (vertex_position) or colors(vertex_color). and as args in function input data 
		// has a position. so first "arg" witll be vertex_position and the second one will vertex_color
		"out vec3 color;"
		"void main() {"
		"   color = vertex_color;"
		"   gl_Position = vec4(vertex_position, 1.0);" // gl_Position is special opengl vertex position which 
		// has to be normalized 
		"}";

	// for every fragment (every pixel we have in out triangle (or whatever we have)) 
	const char* fragment_shader =
		"#version 460\n"
		"in vec3 color;"  // our interpolated color
		"out vec4 frag_color;"
		"void main() {"
		"   frag_color = vec4(color, 1.0);"
		"}";

	GLuint shader_program;
	GLuint vao;

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

		// set size of window to draw 
		glfwSetFramebufferSizeCallback(window,
			[](GLFWwindow* pWindow, int width, int height)
			{
				glViewport(0, 0, width, height);
			}
		);

		// first we create "programm" like code on c++
		GLuint vs = glCreateShader(GL_VERTEX_SHADER); // create vertex shader and get its id 
		glShaderSource(vs, 1, &vertex_shader, nullptr); // pass code (shader id, amount of string, pointer to code, array of string)
		glCompileShader(vs); // compile shader code 

		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &fragment_shader, nullptr);
		glCompileShader(fs);

		shader_program = glCreateProgram(); // create program as in c++
		glAttachShader(shader_program, vs); // link vertex shader to program
		glAttachShader(shader_program, fs);
		glLinkProgram(shader_program); // create final program and "link all together"

		glDeleteShader(vs); // we don't need anymore so can delete shaders 
		glDeleteShader(fs);

		// second now we need to pass gpu our data (which is positions and colors currently) 
		// for that we generate vertex buffer object
		// like in cuda 
		GLuint points_vbo = 0;
		glGenBuffers(1, &points_vbo); // (how many buffers we can create array for example, address there to)
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo); // make current buffer current. current can be only one. (type, id)
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW); // now we can fill our buffer on gpu
		//(type, size in bytes, pointer to data, GL_STATIC_DRAW we use because we don't change points. DYNAMIC in opposite can change data)
		GLuint colors_vbo = 0;
		glGenBuffers(1, &colors_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

		// gpu does not know what to do with our buffers yet and how to link them with input shaders args.
		// for that we are using vertex array object. vao can handle several vertex buffer objects. 
		// we need to link it together 
		// we use vertex array object for that 
		glGenVertexArrays(1, &vao); // generate it first  (how many, address)
		glBindVertexArray(vao); // make it current as for vbo

		// link vbo with their position (location) in shaders 
		glEnableVertexAttribArray(0); // first we have to TURN on this position (location -> 0) 
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo); // again make current vbo for points
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr); // link data 
		// (location, how many numbers we have (x,y,z), data type, if normalized, stride, shift)

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		return 0;
	}

	void Window::shutdown() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::on_update() {
		glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
		glClear(GL_COLOR_BUFFER_BIT);

		// first we need to inlude shader we wanted to use 
		glUseProgram(shader_program);
		// second we need to include vao for data
		glBindVertexArray(vao);
		// finally draw (how, index_start in our data, how many vertex we want to draw)
		glDrawArrays(GL_TRIANGLES, 0, 3);

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