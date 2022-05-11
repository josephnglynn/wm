//
// Created by joseph on 09/05/22.
//

#ifndef WM_BUFFER_HPP
#define WM_BUFFER_HPP
#include "../messages/messages.hpp"
#include "src/server_data/server_data.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace flow::buffers
{
	template <typename data_type = uintptr_t, typename size_type = size_t>
	struct buffer_write_result_t
	{
		buffer_write_result_t() = default;
		buffer_write_result_t(data_type* data, size_type size)
			: size(size), data(data) {}

		size_type size;
		data_type* data;
	};

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

		data_type* write(const void* data, size_type size)
		{
			size_type new_location = m_location + size;
			if (new_location > m_size)
			{
				m_size = new_location;
				m_data = static_cast<data_type*>(realloc(m_data, m_size));
			}
			data_type* write_location_ptr = m_data + m_location;
			std::memcpy(reinterpret_cast<void*>(write_location_ptr), data, size);
			return write_location_ptr;
		}

		inline data_type* read(size_type amount)
		{
			auto result = m_data + m_location;
			m_location += amount;
			return result;
		}

		template <typename T>
		inline buffer_write_result_t<data_type, size_type> write(T& t);

		[[nodiscard]] inline size_type get_size() const { return m_size; }
		[[nodiscard]] inline size_type get_location() const { return m_location; }
		inline data_type* get_data() { return m_data; }

	private:
		size_type m_size;
		size_type m_location;
		data_type* m_data;
	};

	/*
	 * TEMPLATE SPECIFICATION
	 */

	/*
	 * EXAMPLE OF CHAR BUFFER
	 * using char_buffer_t = buffer_t<char, size_t>;
	 */
	using server_buffer_t = buffers::buffer_t<char, int>;
#define WRITE_MEMBER(name) write(&t.name, sizeof(t.name))
#define WRITE_MEMBER_COMPLEX(name) write(t.name)

	template <>
	template <>
	inline buffer_write_result_t<char, int> buffer_t<char, int>::write(std::string_view& t)
	{
		auto size = t.size();
		write(&size, sizeof(size));
		write(t.data(), size);
		/*
		 * RETURN NOT IMPORTANT
		 */
		return {};
	}

    template<>
    template<>
    inline buffer_write_result_t<char, int> buffer_t<char, int>::write(server::server_location_t& t) 
    {
        write(&t, sizeof(t));
        /*
         * RETURN NOT IMPORTANT
         */
        return {};
    }

	template <>
	template <>
	inline buffer_write_result_t<char, int> buffer_t<char, int>::write(server::server_data_t& t)
	{
		WRITE_MEMBER(uid);
		WRITE_MEMBER_COMPLEX(machine_name);
	    WRITE_MEMBER_COMPLEX(location);	
        /*
		 * RETURN NOT IMPORTANT
		 */
		return {};
	}

	template <>
	template <>
	inline buffer_write_result_t<char, int> buffer_t<char, int>::write(messages::message_sync_wm_servers_response_t& t)
	{
		buffer_write_result_t<char, int> result = {};
		auto start_location = m_location;
		result.data = m_data + m_location;
		WRITE_MEMBER(type);
		WRITE_MEMBER_COMPLEX(server_data);

		result.size = m_location - start_location;
		return result;
	}

#undef WRITE_MEMBER
#undef WRITE_MEMBER_COMPLEX

} // namespace flow::buffers

#endif //WM_BUFFER_HPP
