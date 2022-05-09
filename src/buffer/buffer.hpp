//
// Created by joseph on 09/05/22.
//

#ifndef WM_BUFFER_HPP
#define WM_BUFFER_HPP
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace flow::buffers
{
	template <typename data_type = uintptr_t, typename size_type = size_t>
	class buffer_t
	{
	public:
		explicit buffer_t(size_type size)
			: m_size(size), m_location(0), m_data(static_cast<data_type*>(std::malloc(size)))
		{
		}

		~buffer_t()
		{
			free(m_data);
		}

		void reset()
		{
			m_location = 0;
		}

		void resize(size_type new_size)
		{
			if (m_size > new_size) return;
			m_size = new_size;
			m_data = static_cast<data_type*>(realloc(m_data, new_size));
		}

		void write(const void* data, size_type size)
		{
			data_type new_location = m_location + size;
			if (new_location > m_size)
			{
				m_size = new_location;
				m_data = static_cast<data_type*>(realloc(m_data, m_size));
			}
			std::memcpy(reinterpret_cast<void*>(m_data + m_location), data, size);
		}

		[[nodiscard]] inline size_type get_size() const { return m_size; }
		[[nodiscard]] inline size_type get_location() const { return m_location; }
		inline data_type* get_data() { return m_data; }

	private:
		size_type m_size;
		size_type m_location;
		data_type* m_data;
	};

	/*
	 * EXAMPLE OF CHAR BUFFER
	 * using char_buffer_t = buffer_t<char, size_t>;
	 */

} // namespace flow::buffers

#endif //WM_BUFFER_HPP
