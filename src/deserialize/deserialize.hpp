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
	inline T deserialize(const buffers::server_buffer_t& buffer);

	template <>
	inline std::string deserialize(const buffers::server_buffer_t& buffer)
	{
		auto size = *reinterpret_cast<std::string::size_type*>(buffer.read(sizeof(std::string::size_type)));
		auto data = buffer.read(size);
		return {data, size};
	}

	template <>
	inline config::server_config deserialize(const buffers::server_buffer_t& buffer)
	{
		config::server_config config;
		config.server_location = *( config::server_location_t* ) buffer.read(sizeof(config.server_location));
		return config;
	}

	template <>
	inline messages::message_host_initial_connect_request_t deserialize(const buffers::server_buffer_t& buffer)
	{
		messages::message_host_initial_connect_request_t msg;
		buffer.read(sizeof(msg.type));
		msg.to = *( uid::uid_generator::uid_t* ) buffer.read(sizeof(msg.to));
		msg.from = *( uid::uid_generator::uid_t* ) buffer.read(sizeof(msg.from));
		msg.server_config = deserialize<config::server_config>(buffer);
		return msg;
	}

} // namespace flow::serialization

#endif //WM_DESERIALIZE_HPP
