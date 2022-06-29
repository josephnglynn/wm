//
// Created by joseph on 09/05/22.
//

#ifndef WM_BUFFER_HPP
#define WM_BUFFER_HPP
#include "../config/config.hpp"
#include "../messages/messages.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <logger/logger.hpp>

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
#ifdef DEBUG
			if (size <= 0) throw "Buffer can't have a size of 0";
#endif
		}

		explicit buffer_t(const std::string& str)
			: m_size(str.size()), m_location(0), m_data(( data_type* ) (( void* ) str.data()))
		{
		}

		~buffer_t()
		{
			free(m_data);
		}

		void reset() const
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
			m_location = new_location;
			return write_location_ptr;
		}

		[[nodiscard]] inline data_type* read(size_type amount)
		{
			auto result = m_data + m_location;
			m_location += amount;
			return result;
		}

		[[nodiscard]] inline data_type* read(size_type amount) const
		{
			auto result = m_data + m_location;
			auto* ptr = const_cast<buffer_t<data_type, size_type>*>(this);
			ptr->m_location += amount;
			return result;
		}

		template <typename T>
		inline buffer_write_result_t<data_type, size_type> write(T& t);

		[[nodiscard]] inline size_type get_size() const
		{
			return m_size;
		}
		[[nodiscard]] inline size_type get_location() const
		{
			return m_location;
		}
		[[nodiscard]] inline data_type* get_data() const
		{
			return m_data;
		}

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
#define GET_START_INFO()                          \
	buffer_write_result_t<char, int> result = {}; \
	auto start_location = m_location;             \
	result.data = m_data + m_location

#define WRITE_START_INFO()                     \
	result.size = m_location - start_location; \
	return result

#define WRITE_DEFAULT_INFO() \
	WRITE_MEMBER(type);      \
	WRITE_MEMBER(uid)

	template <>
	template <>
	inline buffer_write_result_t<char, int> buffer_t<char, int>::write(config::vec3<int>& t)
	{
		WRITE_MEMBER(x);
		WRITE_MEMBER(y);
		WRITE_MEMBER(z);
		return {};
	}

	template <>
	template <>
	inline buffer_write_result_t<char, int> buffer_t<char, int>::write(config::server_config& t)
	{
		write(t.server_location);
		//TODO FINISH AFTER SERVER CONFIG
		return {};
	}

	template <>
	template <>
	inline buffer_write_result_t<char, int> buffer_t<char, int>::write(messages::message_host_initial_connect_request_t& t)
	{
		GET_START_INFO();
		WRITE_DEFAULT_INFO();
		WRITE_MEMBER_COMPLEX(server_config);
		WRITE_START_INFO();
	}

#undef WRITE_MEMBER
#undef WRITE_MEMBER_COMPLEX

} // namespace flow::buffers

#endif //WM_BUFFER_HPP
