//
// Created by joseph on 05/05/22.
//

#ifndef WM_SERVER_HPP
#define WM_SERVER_HPP

#include "../buffer/buffer.hpp"
#include "../config/config.hpp"
#include "../handlers/handlers.hpp"
#include "../messages/messages.hpp"
#include "../uid/uid.hpp"
#include <functional>
#include <istream>
#include <logger/logger.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <websocketpp/client.hpp>
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/common/memory.hpp>
#include <websocketpp/common/system_error.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/frame.hpp>
#include <websocketpp/roles/server_endpoint.hpp>
#include <websocketpp/server.hpp>
#include <wm/flow_wm.hpp>

namespace flow::server
{
	using web_client_t = websocketpp::client<websocketpp::config::asio_client>;

	const int SERVER_PORT = 16544;
	const std::string AUTH_STRING = "THIS_IS_A_LEGIT_FLOW_OS_SERVER_WINK_WINK"; // TODO FIX AUTH

	struct ws_client_t
	{
		ws_client_t() = default;
		ws_client_t(websocketpp::connection_hdl hdl)
			: hdl(std::move(hdl)) {}

		config::server_config config;
		uid::uid_generator::uid_t uid = -1;
		websocketpp::connection_hdl hdl;
	};

	/*
     * HOST SERVER, ONLY ONE AT TIME
     */
	class host_server_t
	{

	public:
		host_server_t();
		~host_server_t();

		using server_t = websocketpp::server<websocketpp::config::asio>;

		void run();
		uid::uid_generator::uid_t add_server(websocketpp::connection_hdl hdl, config::server_config& config);
		inline server_t& get_server(){return server;};

	private:
		inline void on_open(websocketpp::connection_hdl hdl)
		{
			websocket_clients[uid_gen.get_next_uid()] = ws_client_t(std::move(hdl));
		};

		inline void on_close(const websocketpp::connection_hdl& hdl)
		{
			for (auto it = websocket_clients.begin(); it != websocket_clients.end(); it++)
			{
				std::shared_ptr ptr1(it->second.hdl);
				std::shared_ptr ptr2(hdl);

				if (ptr1 == ptr2)
				{
					websocket_clients.erase(it);
					break;
				}
			}
		}

		inline void forward_msg_on(messages::message_base_t* msg, std::string::size_type size)
		{
			auto location = websocket_clients.find(msg->uid);
			if (location != std::end(websocket_clients))
			{
				websocketpp::lib::error_code ec;
				server.send(location->second.hdl, msg, size, websocketpp::frame::opcode::BINARY, ec);
				if (ec) goto error_forward;
			}
			else
			{
				goto error_forward;
			}

		error_forward:
#ifdef DEBUG
			logger::warn("Error occured forwarding msg");
#endif
		}

		inline void on_msg(websocketpp::connection_hdl hdl, const server_t::message_ptr& msg)
		{
			auto& payload = msg->get_payload();
			const buffers::server_buffer_str_t buf(payload);
			auto* data = ( messages::message_base_t* ) buf.get_data();
			if (data->uid > 0) return forward_msg_on(data, payload.size());

			handlers::handlers[data->type](std::move(hdl), *(const buffers::server_buffer_t*)&buf);
		}



		uid::uid_generator uid_gen = {};
		std::thread internal_server_thread;
		std::unordered_map<uid::uid_generator::uid_t, ws_client_t> websocket_clients;
		server_t server;
	};

	enum ConnectionStatus
	{
		Open,
		Connecting,
		Failed,
		Closed
	};

	class connection_metadata
	{
	public:
		using ptr = websocketpp::lib::shared_ptr<connection_metadata>;

		connection_metadata(websocketpp::connection_hdl hdl, std::string uri)
			: uri(std::move(uri)), server_name("N/A"), status(Connecting), uid(-1), hdl(std::move(hdl)) {}

		[[nodiscard]] inline int get_uid() const { return uid; }
		[[nodiscard]] inline ConnectionStatus get_status() const { return status; }
		[[nodiscard]] inline websocketpp::connection_hdl get_hdl() const { return hdl; }

		inline void on_open(web_client_t* c, websocketpp::connection_hdl p_hdl)
		{
			status = Open;
			web_client_t::connection_ptr con = c->get_con_from_hdl(std::move(p_hdl));
			server_name = con->get_response_header("Server");
		}

		inline void on_fail(web_client_t* c, websocketpp::connection_hdl p_hdl)
		{
			status = Failed;
			web_client_t::connection_ptr con = c->get_con_from_hdl(std::move(p_hdl));
			server_name = con->get_response_header("Server");
			error_reason = con->get_ec().message();
		}

		inline void on_close(web_client_t* c, websocketpp::connection_hdl p_hdl)
		{
			status = Closed;
			web_client_t::connection_ptr con = c->get_con_from_hdl(std::move(p_hdl));
			std::stringstream s;
			s << "close code: " << con->get_remote_close_code() << " ("
			  << websocketpp::close::status::get_string(con->get_remote_close_code())
			  << "), close reason: " << con->get_remote_close_reason();
			error_reason = s.str();
		}

		inline void on_message(websocketpp::connection_hdl hdl, web_client_t::message_ptr msg)
		{
			auto& payload = msg->get_payload();
			const buffers::server_buffer_str_t buf(payload);

			messages::message_base_t* data = (( messages::message_base_t* ) buf.get_data());
			if (data->type > messages::message_type::_number_of_message_types) return;

			handlers::handlers[data->type](std::move(hdl),  *(const buffers::server_buffer_t*)&buf);
		}

	private:
		std::string uri;
		std::string server_name;
		ConnectionStatus status;
		std::string error_reason;
		uid::uid_generator::uid_t uid;
		websocketpp::connection_hdl hdl;
	};

	class guest_client_t
	{
	public:
		guest_client_t();
		~guest_client_t();

		void connect(lib_wm::WindowManager& wm);

		template <typename T>
		inline void send_msg(const T& t, buffers::server_buffer_t& buffer)
		{
			websocketpp::lib::error_code ec;

			auto result = buffer.write<T>(t);
			client.send(connection->get_hdl(), result.data, result.size, websocketpp::frame::opcode::BINARY, ec);

#ifdef DEBUG
			if (ec)
			{

				logger::notify("Damn, an error occurred sending a msg: ");
			}
#endif
		}

		inline web_client_t& get_client()
		{
			return client;
		}

		inline config::server_config& get_config()
		{
			return server_config;
		}

		inline uid::uid_generator::uid_t get_uid() const { return uid; }
		inline void set_uid(uid::uid_generator::uid_t n_uid) { uid = n_uid;}

	private:
		std::string scan();
		void internal_connect(const std::string& url);

		/*
		 * hs STORES HOST SERVER PTR ON THIS OBJECT, so can be freed
		 */
		host_server_t* hs = nullptr;
		uid::uid_generator::uid_t uid;
		web_client_t client;
		config::server_config server_config;
		connection_metadata::ptr connection;
		websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread;
	};

} // namespace flow::server

#endif //WM_SERVER_HPP
