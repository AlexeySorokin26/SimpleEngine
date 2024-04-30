#include "SimpleEngineCore/Window.hpp"
#include "SimpleEngineCore/Log.hpp"
#include "SimpleEngineCore/Rendering/OpenGL/ShaderProgram.hpp"
#include "SimpleEngineCore/Rendering/OpenGL/VertexBuffer.hpp"
#include "SimpleEngineCore/Rendering/OpenGL/VertexArray.hpp"
#include "SimpleEngineCore/Rendering/OpenGL/IndexBuffer.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define IMGUI_IMPL_OPENGL_SHADER_VERSION "#version 130"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include <glm/trigonometric.hpp>

namespace  SimpleEngine {

	static bool GLFW_initialized = false;

	GLfloat positions_colors2[] = {
	  -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 0.0f,
	   0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 1.0f,
	  -0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 1.0f,
	   0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f
	};

	GLuint indices[] = {
		0, 1, 2, 3, 2, 1
	};

	// we run it for every vertex (every point)
	// "data type": in/out vec3 
	// what is layout? shader needs to find out input data 
	// which could be points (vertex_position) or colors(vertex_color). and as args in function input data 
	// has a position. so first "arg" witll be vertex_position and the second one will vertex_color
	// this arg will be one for all vertecies 
	// gl_Position is special opengl vertex position which 
	// has to be normalized 
	// model_matrix contains all translate * rotate * scale matrices
	const char* vertex_shader =
		R"(#version 460
		layout(location = 0) in vec3 vertex_position; 
		layout(location = 1) in vec3 vertex_color; 
		uniform mat4 model_matrix;
		out vec3 color;
		void main() {
		   color = vertex_color;
		   gl_Position = model_matrix * vec4(vertex_position, 1.0); 
		}
		)";

	// for every fragment (every pixel we have in out triangle (or whatever we have)) 
	// our interpolated color
	const char* fragment_shader =
		R"(#version 460
		in vec3 color; 
		out vec4 frag_color;
		void main() {
		   frag_color = vec4(color, 1.0);
		}
		)";

	std::unique_ptr<ShaderProgram> p_shader_program;
	std::unique_ptr<VertexBuffer> p_positions_colors_vbo;
	std::unique_ptr<IndexBuffer> p_index_buffer;
	std::unique_ptr<VertexArray> p_vao;

	float scale[3] = { 1.f, 1.f, 1.f };
	float rotate = 0.0f;
	float translate[3] = { 0.f, 0.f, 0.f };

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

		// first: we create "programm" like code on c++
		p_shader_program = std::make_unique<ShaderProgram>(vertex_shader, fragment_shader);
		if (!p_shader_program->isCompiled()) {
			return false;
		}
		// second: now we need to pass gpu our data (which is positions and colors currently) 
		// for that we generate vertex buffer object
		// like in cuda 
		// third: gpu does not know what to do with our buffers yet and how to link them with input shaders args.
		// for that we are using vertex array object. vao can handle several vertex buffer objects. 
		// we need to link it together 
		// we use vertex array object for that 
		BufferLayout buffer_layout_1vec3
		{
			ShaderDataType::Float3
		};

		BufferLayout buffer_layout_2vec3
		{
			ShaderDataType::Float3,
			ShaderDataType::Float3
		};
		p_vao = std::make_unique<VertexArray>();
		p_positions_colors_vbo = std::make_unique<VertexBuffer>(positions_colors2, sizeof(positions_colors2), buffer_layout_2vec3);
		p_index_buffer = std::make_unique<IndexBuffer>(indices, sizeof(indices) / sizeof(GLuint));

		p_vao->add_vertex_buffer(*p_positions_colors_vbo);
		p_vao->set_index_buffer(*p_index_buffer);

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

		//ImGui::ShowDemoWindow(); // create data 

		ImGui::Begin("Background Color Window"); // create window
		ImGui::ColorEdit4("Background Color", backgroundColor);
		ImGui::SliderFloat3("scale", scale, 0.f, 2.f);
		ImGui::SliderFloat3("translate", translate, -1.f, 1.f);
		ImGui::SliderFloat("rotate", &rotate, 0.f, 360.f);

		p_shader_program->bind();

		glm::mat4 scale_matrix(
			scale[0], 0, 0, 0,
			0, scale[1], 0, 0,
			0, 0, scale[2], 0,
			0, 0, 0, 1);

		float rotate_in_rad = glm::radians(rotate);
		glm::mat4 rotate_matrix(
			cos(rotate_in_rad), sin(rotate_in_rad), 0, 0, // we are using column major so matrix is "transposed"
			-sin(rotate_in_rad), cos(rotate_in_rad), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);

		glm::mat4 translate_matrix(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			translate[0], translate[1], translate[2], 1);
		glm::mat4 model_matrix = translate_matrix * rotate_matrix * scale_matrix;

		p_shader_program->setMatrix4("model_matrix", model_matrix);

		p_vao->bind();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(p_vao->get_indices_count()), GL_UNSIGNED_INT, nullptr);


		ImGui::End();

		ImGui::Render(); // draw data using imgui 
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // use our data to draw using opengl for example

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

} // namespace  SimpleEngineCore