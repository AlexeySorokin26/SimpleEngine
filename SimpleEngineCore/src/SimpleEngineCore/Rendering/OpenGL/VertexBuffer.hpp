#pragma once

#include <vector>

namespace SimpleEngine {

	enum class ShaderDataType
	{
		Float,
		Float2,
		Float3,
		Float4,
		Int,
		Int2,
		Int3,
		Int4,
	};

	struct BufferElement
	{
		ShaderDataType type; // Float, Float2, ...
		uint32_t component_type; // OpenGL type
		size_t components_count; // FLoat->1, Float2->2, ...
		size_t size; // size in bytes
		size_t offset; // how many bytes from begining 

		BufferElement(const ShaderDataType type);
	};

	class BufferLayout
	{
	public:
		BufferLayout(std::initializer_list<BufferElement> elements)
			: m_elements(std::move(elements))
		{
			size_t offset = 0;
			m_stride = 0;
			for (auto& element : m_elements)
			{
				element.offset = offset;
				offset += element.size;
				m_stride += element.size;
			}
		}

		const std::vector<BufferElement>& get_elements() const { return m_elements; }
		size_t get_stride() const { return m_stride; }

	private:
		std::vector<BufferElement> m_elements;
		size_t m_stride = 0; // in how many bytes we have the next element 
		// 1.0f 1.0f 1.0f, 1.0f 1.0f 1.0f -> stride will be 3*4bytes * 2 = 24
		// 1.0f 1.0f 1.0f, 1.0f 1.0f 1.0f
		// 1.0f 1.0f 1.0f, 1.0f 1.0f 1.0f
		// pos, color
		// 1.0f 1.0f 1.0f,  -> 12 bytes
		// ... 
	};

	class VertexBuffer {
	public:

		enum class EUsage
		{
			Static,
			Dynamic,
			Stream
		};

		VertexBuffer(const void* data, const size_t size, BufferLayout buffer_layout, const EUsage usage = VertexBuffer::EUsage::Static);
		~VertexBuffer();

		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&& vertexBuffer) noexcept;
		VertexBuffer(VertexBuffer&& vertexBuffer) noexcept;

		void bind() const;
		static void unbind();

		const BufferLayout& get_layout() const { return m_buffer_layout; }

	private:
		unsigned int m_id = 0;
		BufferLayout m_buffer_layout; // indicate how data packaged in buffer 
	};

}