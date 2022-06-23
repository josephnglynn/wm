//
// Created by joseph on 10/05/22.
//

#ifndef WM_HANDLERS_HPP
#define WM_HANDLERS_HPP
#include "../buffer/buffer.hpp"
#include "../messages/messages.hpp"
#include "../server/server.hpp"
#include <Poco/Net/WebSocket.h>
#include <array>
#include <wm/flow_wm.hpp>

namespace flow::handlers
{
	using request_handler = void (*)(flow::buffers::server_buffer_t& buffer, Poco::Net::WebSocket& ws);
	using response_handler = void (*)(flow::buffers::server_buffer_t& buffer);
#define PLACE_HANDLERS_REQ(name, ...) void name##_request_handler(flow::buffers::server_buffer_t& buffer, Poco::Net::WebSocket& ws);
#define PLACE_HANDLERS_RES(name, ...) void name##_response_handler(flow::buffers::server_buffer_t& buffer);
	MESSAGE_TYPES_REQ(PLACE_HANDLERS_REQ)
	MESSAGE_TYPES_RES(PLACE_HANDLERS_RES)
#undef PLACE_HANDLERS_REQ
#undef PLACE_HANDLERS_RES

	extern std::array<request_handler, messages::_number_of_request_types> request_handlers;
	extern std::array<response_handler, messages::_number_of_response_types> response_handlers;

	void init_handlers(lib_wm::WindowManager& p_wm);
} // namespace flow::handlers

#endif //WM_HANDLERS_HPP
