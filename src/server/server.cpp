//
// Created by joseph on 05/05/22.
//

#include "server.hpp"

namespace flow::server
{

	int callback(lws* wsi, lws_callback_reasons reson, void* user, void* in, size_t len)
	{
		flow_wm_server_t* server = reinterpret_cast<flow_wm_server_t*>(user);

		return 0;
	}

	flow_wm_server_t::flow_wm_server_t()
	{
#ifdef DEBUG
		lws_set_log_level(LLL_DEBUG, nullptr);
#else
		lws_set_log_level(LLL_ERR, nullptr);
#endif

		lwsl_user("LWS minimal ws server | visit http://localhost:7681 (-s = use TLS / https)\n");

		const lws_protocols protocol = {"websocket_minimal", callback, sizeof(session_data_t), 128, 0, this, 0};
		const lws_protocols protocols[] = {
            protocol,
			LWS_PROTOCOL_LIST_TERM,
		};

		const lws_retry_bo retry = {
			nullptr,
			0,
			0,
			3,
			10,
			0,
        };

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
    }

	flow_wm_server_t::~flow_wm_server_t()
	{
		lws_context_destroy(context);
	}

    flow_wm_server_t::run() 
    { 
		auto n = 0;
		while (n >= 0)
		{
			n = lws_service(context, 0);
		}
	}
} // namespace flow::server
