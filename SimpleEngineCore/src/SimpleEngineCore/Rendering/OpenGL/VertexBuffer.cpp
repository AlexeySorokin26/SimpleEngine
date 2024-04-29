#include "VertexBuffer.hpp"

#include "SimpleEngineCore/Log.hpp"

#include <glad/glad.h>

namespace SimpleEngine {

    constexpr GLenum usage_to_GLenum(const VertexBuffer::EUsage usage)
    {
        switch (usage)
        {
        case VertexBuffer::EUsage::Static:  return GL_STATIC_DRAW; // one time create and use many time to read
        case VertexBuffer::EUsage::Dynamic: return GL_DYNAMIC_DRAW;  // one time create and change many times
        case VertexBuffer::EUsage::Stream:  return GL_STREAM_DRAW; // one time create and one time use (not oftern)
        }

        LOG_ERROR("Unknown VertexBuffer usage");
        return GL_STREAM_DRAW;
    }

    VertexBuffer::VertexBuffer(const void* data, const size_t size, const EUsage usage)
    {
        glGenBuffers(1, &m_id); // (how many buffers we can create array for example, address there to)
        glBindBuffer(GL_ARRAY_BUFFER, m_id); // make current buffer current. current can be only one. (type, id)
        glBufferData(GL_ARRAY_BUFFER, size, data, usage_to_GLenum(usage)); // now we can fill our buffer on gpu
		//(type, size in bytes, pointer to data, GL_STATIC_DRAW we use because we don't change points. DYNAMIC in opposite can change data)
    }

    VertexBuffer::~VertexBuffer()
    {
        glDeleteBuffers(1, &m_id); // clean up gpu memory
    }

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& vertexBuffer) noexcept
    {
        m_id = vertexBuffer.m_id;
        vertexBuffer.m_id = 0;
        return *this;
    }

    VertexBuffer::VertexBuffer(VertexBuffer&& vertexBuffer) noexcept
    {
        m_id = vertexBuffer.m_id;
        vertexBuffer.m_id = 0;
    }

    void VertexBuffer::bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_id); // again make current vbo for points
    }

    void VertexBuffer::unbind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}