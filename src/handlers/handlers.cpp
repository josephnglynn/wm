//
// Created by joseph on 10/05/22.
//

#include "handlers.hpp"
#include "../deserialize/deserialize.hpp"
#include <Poco/Net/WebSocket.h>
#include <wm/product_info/product_info.hpp>

namespace flow::handlers
{
	/*
	 * DOESN'T need mutex yet? watch
	 */
	lib_wm::WindowManager* wm;

	std::array<request_handler, messages::_number_of_request_types> request_handlers;
	std::array<response_handler, messages::_number_of_response_types> response_handlers;



#define INIT_REQ_HANDLER(name, ...) request_handlers[messages::name##_request] = name##_request_handler;
#define INIT_RES_HANDLER(name, ...) response_handlers[messages::name##_response] = name##_response_handler;

	void init_handlers(lib_wm::WindowManager& p_wm)
	{
		static bool inited = false;
		if (inited) return;
		else
			inited = true;

		wm = &p_wm;
		request_handlers = {};
		response_handlers = {};

		MESSAGE_TYPES_REQ(INIT_REQ_HANDLER)
		MESSAGE_TYPES_RES(INIT_RES_HANDLER)
	}
#undef INIT_REQ_HANDLER


	void debug_message_request_handler(flow::buffers::server_buffer_t& buffer, WebSocket& ws)
	{
	}

	void debug_message_response_handler(flow::buffers::server_buffer_t& buffer)
	{
#ifdef SERVER_DEBUG
		auto response = serialization::deserialize<messages::message_debug_message_response_t>(buffer);
		if (client.debug_handler) client.debug_handler(client, response);
#endif
	}

} // namespace flow::handlers
