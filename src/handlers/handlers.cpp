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

#define INIT_REQ_HANDLER(host, uid, name, ...) request_handlers[messages::host##_##name##_request] = host##_##name##_request_handler;
#define INIT_RES_HANDLER(host, uid, name, ...) response_handlers[messages::host##_##name##_response] = host##_##name##_response_handler;

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

	void host_initial_connect_request_handler(flow::buffers::server_buffer_t& buffer, Poco::Net::WebSocket& ws)
	{
		auto msg = messages::message_host_initial_connect_response_t();
		auto res = buffer.write(msg);
		ws.sendFrame(res.data, res.size, WebSocket::FRAME_BINARY);
	}

	void host_initial_connect_response_handler(flow::buffers::server_buffer_t& buffer)
	{
	}

	void host_connect_test_request_handler(flow::buffers::server_buffer_t& buffer, Poco::Net::WebSocket& ws) {}
	void host_connect_test_response_handler(flow::buffers::server_buffer_t& buffer) {}

} // namespace flow::handlers
