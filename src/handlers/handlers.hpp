//
// Created by joseph on 10/05/22.
//

#ifndef WM_HANDLERS_HPP
#define WM_HANDLERS_HPP
#include "../buffer/buffer.hpp"
#include "../messages/messages.hpp"
#include <array>
#include <websocketpp/client.hpp>
#include <wm/flow_wm.hpp>

namespace flow::handlers
{
	using handler = void (*)(websocketpp::connection_hdl, messages::message_base_t*);
#define PLACE_HANDLERS_REQ(host, uid, name, ...) void host##_##name##_request_handler(websocketpp::connection_hdl, messages::message_base_t*);
#define PLACE_HANDLERS_RES(host, uid, name, ...) void host##_##name##_response_handler(websocketpp::connection_hdl, messages::message_base_t*);
	MESSAGE_TYPES_REQ(PLACE_HANDLERS_REQ)
	MESSAGE_TYPES_RES(PLACE_HANDLERS_RES)
#undef PLACE_HANDLERS_REQ
#undef PLACE_HANDLERS_RES

	extern std::array<handler, messages::_number_of_message_types> handlers;

	void init_handlers(lib_wm::WindowManager& p_wm, void* gc, void* hs);
} // namespace flow::handlers

#endif //WM_HANDLERS_HPP
