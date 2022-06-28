//
// Created by joseph on 10/05/22.
//

#include "handlers.hpp"
#include "../buffer/buffer.hpp"
#include "../deserialize/deserialize.hpp"
#include "../messages/messages.hpp"
#include "../server/server.hpp"
#include "logger/logger.hpp"
#include <websocketpp/common/system_error.hpp>
#include <websocketpp/frame.hpp>
#include <wm/product_info/product_info.hpp>

namespace flow::handlers
{
	/*
	 * DOESN'T need mutex yet? watch
	 */
	lib_wm::WindowManager* wm;
	server::host_server_t* host_server;
	server::guest_client_t* guest_client;

	extern std::array<handler, messages::_number_of_message_types> handlers;
	using connection_ptr_t = server::web_client_t::connection_ptr;

#define INIT_REQ_HANDLER(host, uid, name, ...) handlers[messages::host##_##name##_request] = host##_##name##_request_handler;
#define INIT_RES_HANDLER(host, uid, name, ...) handlers[messages::host##_##name##_response] = host##_##name##_response_handler;

	void init_handlers(lib_wm::WindowManager& p_wm, void* gc, void* hs)
	{
		static bool inited = false;
		if (inited)
		{
			logger::error("wtf, we've already inited handlers");
			return;
		}
		else
			inited = true;

		wm = &p_wm;
		guest_client = ( server::guest_client_t* ) gc;
		host_server = ( server::host_server_t* ) hs;

		handlers = {};

		MESSAGE_TYPES_REQ(INIT_REQ_HANDLER)
		MESSAGE_TYPES_RES(INIT_RES_HANDLER)
	}
#undef INIT_REQ_HANDLER

	void host_initial_connect_request_handler(websocketpp::connection_hdl hdl, messages::message_base_t* msg)
	{
		auto* req = ( messages::message_host_initial_connect_request_t* ) msg;
	}

	void host_initial_connect_response_handler(websocketpp::connection_hdl, messages::message_base_t*)
	{
	}

	void host_connect_test_request_handler(websocketpp::connection_hdl hdl, messages::message_base_t*)
	{
		messages::message_host_connect_test_response_t response;
		websocketpp::lib::error_code ec;
		response.good = true;

		server::web_client_t& c = guest_client->get_client();
		c.send(hdl, &response, sizeof(response), websocketpp::frame::opcode::BINARY, ec);
	}

	void host_connect_test_response_handler(websocketpp::connection_hdl hdl, messages::message_base_t* msg)
	{
		auto* req = ( messages::message_host_connect_test_response_t* ) msg;
		if (!req->good)
		{
			logger::error("IDEK HOW??");
			std::exit(-1);
		}

		messages::message_host_initial_connect_request_t resp;
		resp.server_config = &guest_client->get_config();

		buffers::server_buffer_t buf(sizeof(resp.uid) + sizeof(resp.type) + sizeof(*resp.server_config)); //TODO CHECK NO REALLOC
		auto result = buf.write(resp);

		websocketpp::lib::error_code ec;
		server::web_client_t& c = guest_client->get_client();
		c.send(hdl, result.data, result.size, websocketpp::frame::opcode::BINARY, ec);
	}

} // namespace flow::handlers
