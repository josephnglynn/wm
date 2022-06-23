//
// Created by joseph on 10/05/22.
//

#ifndef WM_DESERIALIZE_HPP
#define WM_DESERIALIZE_HPP
#include "../buffer/buffer.hpp"
#include "../messages/messages.hpp"

namespace flow::serialization
{
	template <typename T>
	inline T deserialize(buffers::server_buffer_t& buffer);

	template <>
	inline std::string deserialize(buffers::server_buffer_t& buffer)
	{
		auto size = *reinterpret_cast<std::string::size_type*>(buffer.read(sizeof(std::string::size_type)));
		auto data = buffer.read(size);
		return {data, size};
	}

	template <>
	inline messages::message_debug_message_response_t deserialize(buffers::server_buffer_t& buffer)
	{
		messages::message_debug_message_response_t resp;
		buffer.read(sizeof(resp.type));
		resp.contents = deserialize<std::string>(buffer);
		return resp;
	}

} // namespace flow::serialization

#endif //WM_DESERIALIZE_HPP
