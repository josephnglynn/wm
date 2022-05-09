//
// Created by joseph on 05/05/22.
//

#ifndef WM_SERVER_HPP
#define WM_SERVER_HPP

#include <libwebsockets.h>

namespace flow::server
{

	struct session_data_t
	{
		session_data_t* next;
		lws* wsi;
		int last;
	};

	class flow_wm_server_t
	{
	public:
		flow_wm_server_t();
		~flow_wm_server_t();

        friend int callback(lws *wsi, lws_callback_reasons reson, void* user, void *in, size_t len);

	private:
		lws_context* context;
	};

} // namespace flow::server

#endif //WM_SERVER_HPP
