#include "ShaderProgram.hpp"

#include "SimpleEngineCore/Log.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace SimpleEngine
{
	bool create_shader(const char* source, const GLenum shader_type, GLuint& shader_id)
	{
		shader_id = glCreateShader(shader_type); // create vertex shader and get its id 
		glShaderSource(shader_id, 1, &source, nullptr); // pass code (shader id, amount of string, pointer to code, array of string)
		glCompileShader(shader_id);

		GLint success; // here we ask if shader was successfully compiled using glGetShaderiv by GL_COMPLIE_STATUS
		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			char info_log[1024];
			glGetShaderInfoLog(shader_id, 1024, nullptr, info_log);

			LOG_CRITICAL("Shader compilation error:\n{}", info_log);
			return false;
		}
		return true;
	}


	ShaderProgram::ShaderProgram(const char* vertex_shader_src, const char* fragment_shader_src)
	{
		GLuint vertex_shader_id = 0;
		if (!create_shader(vertex_shader_src, GL_VERTEX_SHADER, vertex_shader_id))
		{
			LOG_CRITICAL("VERTEX SHADER: compile-time error!");
			glDeleteShader(vertex_shader_id);
			return;
		}

		GLuint fragment_shader_id = 0;
		if (!create_shader(fragment_shader_src, GL_FRAGMENT_SHADER, fragment_shader_id))
		{
			LOG_CRITICAL("FRAGMENT SHADER: compile-time error!");
			glDeleteShader(vertex_shader_id);
			glDeleteShader(fragment_shader_id);
			return;
		}

		m_id = glCreateProgram(); // create program as in c++
		glAttachShader(m_id, vertex_shader_id); // link vertex shader to program
		glAttachShader(m_id, fragment_shader_id);
		glLinkProgram(m_id);  // create final program and "link all together"

		GLint success;
		glGetProgramiv(m_id, GL_LINK_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLchar info_log[1024];
			glGetProgramInfoLog(m_id, 1024, nullptr, info_log);
			LOG_CRITICAL("SHADER PROGRAM: Link-time error:\n{0}", info_log);
			glDeleteProgram(m_id);
			m_id = 0;
			glDeleteShader(vertex_shader_id);
			glDeleteShader(fragment_shader_id);
			return;
		}
		else
		{
			m_isCompiled = true;
		}

		glDetachShader(m_id, vertex_shader_id);
		glDetachShader(m_id, fragment_shader_id);
		glDeleteShader(vertex_shader_id);
		glDeleteShader(fragment_shader_id);
	}

	ShaderProgram::~ShaderProgram()
	{
		glDeleteProgram(m_id);
	}

	void ShaderProgram::bind() const
	{
		glUseProgram(m_id); // make shader current 
	}

	void ShaderProgram::unbind()
	{
		glUseProgram(0);
	}

	void ShaderProgram::setMatrix4(const char* name, const glm::mat4& matrix) const
	{
		// we pass uniform matrix in shader
		glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, glm::value_ptr(matrix));
		// (location, count (how many matrices), transpose or not, pointer to data)
	}

	ShaderProgram& ShaderProgram::operator=(ShaderProgram&& shaderProgram)
	{
		glDeleteProgram(m_id);
		m_id = shaderProgram.m_id;
		m_isCompiled = shaderProgram.m_isCompiled;

		shaderProgram.m_id = 0;
		shaderProgram.m_isCompiled = false;
		return *this;
	}

	ShaderProgram::ShaderProgram(ShaderProgram&& shaderProgram)
	{
		m_id = shaderProgram.m_id;
		m_isCompiled = shaderProgram.m_isCompiled;

		shaderProgram.m_id = 0;
		shaderProgram.m_isCompiled = false;
	}
}