//
// Created by joseph on 10/05/22.
//

#ifndef WM_HANDLERS_HPP
#define WM_HANDLERS_HPP
#include "../buffer/buffer.hpp"
#include "../messages/messages.hpp"
#include <Poco/Net/WebSocket.h>
#include <array>
#include <wm/flow_wm.hpp>
#include "../server/server.hpp"

namespace flow::handlers
{
	using request_handler = void (*)(Poco::Net::WebSocket&, flow::buffers::server_buffer_t& buffer);
#define PLACE_HANDLERS_REQ(name, ...) void name##_request_handler(Poco::Net::WebSocket&, flow::buffers::server_buffer_t& buffer);
	MESSAGE_TYPES_REQ(PLACE_HANDLERS_REQ)
#undef PLACE_HANDLERS_REQ

	extern std::array<request_handler, messages::_number_of_request_types> request_handlers;

	void init_handlers(lib_wm::window_manager_t& p_wm, flow::server::flow_wm_server_t& p_server);
} // namespace flow::handlers

#endif //WM_HANDLERS_HPP
