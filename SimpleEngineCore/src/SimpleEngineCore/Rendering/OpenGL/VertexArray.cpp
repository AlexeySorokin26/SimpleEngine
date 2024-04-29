#include "VertexArray.hpp"

#include "SimpleEngineCore/Log.hpp"

#include <glad/glad.h>

namespace SimpleEngine {

	VertexArray::VertexArray()
	{
		glGenVertexArrays(1, &m_id); // generate it first  (how many, address)
	}


	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &m_id);
	}


	VertexArray& VertexArray::operator=(VertexArray&& vertex_array) noexcept
	{
		m_id = vertex_array.m_id;
		m_elements_count = vertex_array.m_id;
		vertex_array.m_id = 0;
		vertex_array.m_elements_count = 0;
		return *this;
	}


	VertexArray::VertexArray(VertexArray&& vertex_array) noexcept
		: m_id(vertex_array.m_id)
		, m_elements_count(vertex_array.m_elements_count)
	{
		vertex_array.m_id = 0;
		vertex_array.m_elements_count = 0;
	}


	void VertexArray::bind() const
	{
		glBindVertexArray(m_id); // make it current as for vbo
	}


	void VertexArray::unbind()
	{
		glBindVertexArray(0);
	}


	void VertexArray::add_buffer(const VertexBuffer& vertex_buffer)
	{
		bind();
		vertex_buffer.bind();

		// link vbo with their position (location) in shaders 
		glEnableVertexAttribArray(m_elements_count); // first we have to TURN on this position (location -> 0) 
		glVertexAttribPointer(m_elements_count, 3, GL_FLOAT, GL_FALSE, 0, nullptr); // link data 
		// (location, how many numbers we have (x,y,z), data type, if normalized, stride, shift)

		++m_elements_count;
	}
}