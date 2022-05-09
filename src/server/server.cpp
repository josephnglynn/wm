//
// Created by joseph on 05/05/22.
//

#include "server.hpp"

namespace flow::server
{


	const lws_protocols protocols[] = {
		LWS_PLUGIN_PROTOCOL_MINIMAL_SERVER_ECHO,
		LWS_PROTOCOL_LIST_TERM,
	};

	const lws_retry_bo retry = {
		nullptr,
		0,
		0,
		3,
		10,
		0
	};


	flow_wm_server_t::flow_wm_server_t()
	{
#ifdef DEBUG
		lws_set_log_level(LLL_DEBUG, nullptr);
#else
		lws_set_log_level(LLL_ERR, nullptr);
#endif

		lwsl_user("LWS minimal ws server | visit http://localhost:7681 (-s = use TLS / https)\n");

		lws_context_creation_info info = {};
		info.port = 9600;
		info.mounts = nullptr;
		info.protocols = protocols;
		info.vhost_name = "localhost";
		info.options = 0;
		info.retry_and_idle_policy = &retry;
		info.ssl_cert_filepath = nullptr;
		info.ssl_private_key_filepath = nullptr;
		info.pvo = nullptr;


		context = lws_create_context(&info);
		if (!context)
		{
			lwsl_err("lws init failed!");
			std::exit(1);
		}

		auto n = 0;
		while (n >= 0 ) {
			n = lws_service(context, 0);
		}
	}

	flow_wm_server_t::~flow_wm_server_t()
	{
		lws_context_destroy(context);
	}

} // namespace flow::server