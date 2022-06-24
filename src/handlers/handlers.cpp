//
// Created by joseph on 10/05/22.
//

#include "handlers.hpp"
#include "../deserialize/deserialize.hpp"
#include "src/messages/messages.hpp"
#include <Poco/Net/WebSocket.h>
#include <wm/product_info/product_info.hpp>

namespace flow::handlers
{
	/*
	 * DOESN'T need mutex yet? watch
	 */
	lib_wm::WindowManager* wm;
	server::host_server_t* host_server;

	std::array<request_handler, messages::_number_of_request_types> request_handlers;
	std::array<response_handler, messages::_number_of_response_types> response_handlers;

#define INIT_REQ_HANDLER(name, ...) request_handlers[messages::name##_request] = name##_request_handler;
#define INIT_RES_HANDLER(name, ...) response_handlers[messages::name##_response] = name##_response_handler;

	void init_handlers(lib_wm::WindowManager& p_wm, server::host_server_t* hs)
	{
		static bool inited = false;
		if (inited) return;
		else
			inited = true;

		wm = &p_wm;
		request_handlers = {};
		response_handlers = {};

		host_server = hs;

		MESSAGE_TYPES_REQ(INIT_REQ_HANDLER)
		MESSAGE_TYPES_RES(INIT_RES_HANDLER)
	}
#undef INIT_REQ_HANDLER

	void initial_connect_request_handler(flow::buffers::server_buffer_t& buffer, Poco::Net::WebSocket& ws)
	{
		messages::message_initial_connect_request_t req = serialization::deserialize<messages::message_initial_connect_request_t>(buffer);
		messages::message_initial_connect_response_t res(host_server->add_server(req.server_config));
		auto data = buffer.write(res);
		ws.sendBytes(data.data, data.size);
	}

	void initial_connect_response_handler(flow::buffers::server_buffer_t& buffer)
	{
		
	}

} // namespace flow::handlers
