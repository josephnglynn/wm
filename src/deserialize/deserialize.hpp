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
	inline server::server_location_t deserialize(buffers::server_buffer_t& buffer)
	{
		return *(( server::server_location_t* ) buffer.read(sizeof(server::server_location_t)));
	}

	template <>
	inline server::server_data_t deserialize(buffers::server_buffer_t& buffer)
	{
		server::server_data_t data;
		data.uid = *reinterpret_cast<uint64_t*>(buffer.read(sizeof(data.uid)));
		data.machine_name = deserialize<std::string>(buffer);
		data.location = deserialize<server::server_location_t>(buffer);
		return data;
	}

	template <>
	inline messages::message_sync_wm_servers_response_t deserialize(buffers::server_buffer_t& buffer)
	{
		messages::message_sync_wm_servers_response_t resp;
		buffer.read(sizeof(resp.type));
		resp.server_data = deserialize<server::server_data_t>(buffer);
		return resp;
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
