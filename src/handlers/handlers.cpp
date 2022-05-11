//
// Created by joseph on 10/05/22.
//

#include "handlers.hpp"
#include "../deserialize/deserialize.hpp"

namespace flow::handlers
{
	lib_wm::window_manager_t* wm;
	flow::server::flow_wm_server_t* server;
	std::array<request_handler, messages::_number_of_request_types> request_handlers;


	void sync_wm_servers_request_handler(Poco::Net::WebSocket& ws, flow::buffers::server_buffer_t& buffer)
	{

		messages::message_sync_wm_servers_response_t response;
		response.server_data = server->get_server_data();

		auto data = buffer.write(response);
		ws.sendFrame(data.data, data.size, Poco::Net::WebSocket::SendFlags::FRAME_BINARY);
	}

	void  hello_world_request_handler(Poco::Net::WebSocket& ws, flow::buffers::server_buffer_t& buffer)
	{
		static const char str[] = "Hello world!";
		ws.sendFrame(str, sizeof(str) - 1, Poco::Net::WebSocket::SendFlags::FRAME_TEXT);
	}

#define INIT_REQ_HANDLER(name, ...) request_handlers[messages::name##_request] = name##_request_handler;

	void init_handlers(lib_wm::window_manager_t& p_wm, flow::server::flow_wm_server_t& p_server)
	{
		static bool inited = false;
		if (inited) return;
		else
			inited = true;

		wm = &p_wm;
		server = &p_server;
		request_handlers = {};
		MESSAGE_TYPES_REQ(INIT_REQ_HANDLER)
	}
#undef INIT_REQ_HANDLER
} // namespace flow::handlers