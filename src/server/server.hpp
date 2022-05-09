//
// Created by joseph on 05/05/22.
//

#ifndef WM_SERVER_HPP
#define WM_SERVER_HPP

#include <libwebsockets.h>

namespace flow::server
{


	class flow_wm_server_t
	{
	public:
		flow_wm_server_t();
		~flow_wm_server_t();

	private:
		lws_context* context;
	};

} // namespace flow::server

#endif //WM_SERVER_HPP
