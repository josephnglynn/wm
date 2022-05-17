//
// Created by joseph on 10/05/22.
//

#include "handlers.hpp"
#include "../deserialize/deserialize.hpp"
#include <wm/product_info/product_info.hpp>

namespace flow::handlers
{
	/*
	 * DOESN'T need mutex yet? watch
	 */
	lib_wm::WindowManager* wm;

	flow::server::flow_wm_server_t* server;
	std::array<request_handler, messages::_number_of_request_types> request_handlers;
	std::array<response_handler, messages::_number_of_response_types> response_handlers;

	void sync_wm_servers_request_handler(Poco::Net::WebSocket& ws, flow::buffers::server_buffer_t& buffer)
	{

		messages::message_sync_wm_servers_response_t response;

		server->get_lock().lock();
		response.server_data = server->get_server_data();
		server->get_lock().unlock();

		auto data = buffer.write(response);
		ws.sendFrame(data.data, data.size, Poco::Net::WebSocket::SendFlags::FRAME_BINARY);
	}

	void debug_message_request_handler(Poco::Net::WebSocket& ws, flow::buffers::server_buffer_t& buffer)
	{
		auto* request = reinterpret_cast<messages::message_debug_message_request_t*>(buffer.get_data());

		messages::message_debug_message_response_t response;
		switch (request->requested_information)
		{
			case ServerVersion:
			{
				response.contents = lib_wm::product_info::version;
			}
			default: break;
		}

		auto data = buffer.write(response);
		ws.sendFrame(data.data, data.size, Poco::Net::WebSocket::SendFlags::FRAME_BINARY);
	}

	void debug_message_response_handler(server::WebSocketClient& client, flow::buffers::server_buffer_t& buffer)
	{
#ifdef SERVER_DEBUG
		auto response = serialization::deserialize<messages::message_debug_message_response_t>(buffer);
		if (client.debug_handler) client.debug_handler(client, response);
#endif
	}

	void sync_wm_servers_response_handler(server::WebSocketClient& client, flow::buffers::server_buffer_t& buffer)
	{
		messages::message_sync_wm_servers_response_t response = serialization::deserialize<messages::message_sync_wm_servers_response_t>(buffer);

		std::lock_guard lock(client.mutex);
		client.server_data = response.server_data;
		std::lock_guard server_lock(server->get_lock());

		auto& clients = server->get_clients();
		auto count = 0;
		for (auto i = 0; i < clients.size(); ++i)
		{
			if (clients[i]->server_data.uid != client.server_data.uid) continue;

			if (count == 1)
			{
				clients.erase(clients.begin() + i);
				--i;
				continue;
			}

			++count;
		}
	}

#define INIT_REQ_HANDLER(name, ...) request_handlers[messages::name##_request] = name##_request_handler;
#define INIT_RES_HANDLER(name, ...) response_handlers[messages::name##_response] = name##_response_handler;

	void init_handlers(lib_wm::WindowManager& p_wm, flow::server::flow_wm_server_t& p_server)
	{
		static bool inited = false;
		if (inited) return;
		else
			inited = true;

		wm = &p_wm;
		server = &p_server;
		request_handlers = {};
		response_handlers = {};

		MESSAGE_TYPES_REQ(INIT_REQ_HANDLER)
		MESSAGE_TYPES_RES(INIT_RES_HANDLER)
	}
#undef INIT_REQ_HANDLER
} // namespace flow::handlers