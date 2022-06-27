//
// Created by joseph on 05/05/22.
//

#ifndef WM_SERVER_HPP
#define WM_SERVER_HPP

#include "../buffer/buffer.hpp"
#include "../config/config.hpp"
#include "../uid/uid.hpp"
#include "src/config/config.hpp"
#include "src/handlers/handlers.hpp"
#include "src/messages/messages.hpp"
#include <functional>
#include <logger/logger.hpp>
#include <mutex>
#include <thread>
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

	const int SERVER_PORT = 16542;

	struct ws_client_t
	{
		config::server_config config;
		uid::uid_generator::uid_t uid;
		websocketpp::connection_hdl hdl;
	};

	/*
     * HOST SERVER, ONLY ONE AT TIME
     */
	class host_server_t
	{

	public:
		host_server_t();

		using server_t = websocketpp::server<websocketpp::config::asio>;

		void run();
		uid::uid_generator::uid_t add_server(websocketpp::connection_hdl hdl, config::server_config& config);

	private:
		void on_msg(websocketpp::connection_hdl hdl, server_t::message_ptr msg);

		uid::uid_generator uid_gen;
		std::thread internal_server_thread;
		std::vector<ws_client_t> websocket_clients;
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
		using client = websocketpp::client<websocketpp::config::asio_client>;

		connection_metadata(websocketpp::connection_hdl hdl, std::string uri)
			: uri(uri), server_name("N/A"), status(Connecting), uid(-1), hdl(hdl) {}

		inline int get_uid() const { return uid; }
		inline ConnectionStatus get_status() const { return status; }
		inline websocketpp::connection_hdl get_hdl() const { return hdl; }

		inline void on_open(client* c, websocketpp::connection_hdl hdl)
		{
			status = Open;
			client::connection_ptr con = c->get_con_from_hdl(hdl);
			server_name = con->get_response_header("Server");
		}

		inline void on_fail(client* c, websocketpp::connection_hdl hdl)
		{
			status = Failed;
			client::connection_ptr con = c->get_con_from_hdl(hdl);
			server_name = con->get_response_header("Server");
			error_reason = con->get_ec().message();
		}

		inline void on_close(client* c, websocketpp::connection_hdl hdl)
		{
			status = Closed;
			client::connection_ptr con = c->get_con_from_hdl(hdl);
			std::stringstream s;
			s << "close code: " << con->get_remote_close_code() << " ("
			  << websocketpp::close::status::get_string(con->get_remote_close_code())
			  << "), close reason: " << con->get_remote_close_reason();
			error_reason = s.str();
		}

		inline void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg)
		{
			auto* data = ( messages::message_base_t* ) msg->get_payload().data();
			if (data->type > messages::message_type::_number_of_message_types || data->uid > 0) return;
			handlers::handlers[data->type](msg, data);
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

		void connect();

		template <typename T>
		inline void send_msg(const T& t, buffers::server_buffer_t& buffer)
		{
			websocketpp::lib::error_code ec;

			auto result = buffer.write<T>(t);
			client.send(connection->get_hdl(), result.data, result.size, websocketpp::frame::opcode::BINARY, ec);

#ifdef DEBUG
			if (ec)
			{

				logger::notify("Damn, an error occured sending a msg: ");
			}
#endif
		}

		inline connection_metadata::client& get_client() { return client; }
		inline config::server_config& get_config() { return server_config; }

	private:
		std::string scan();
		void internal_connect(const std::string& url);

		/*
		 * hs STORES HOST SERVER PTR ON THIS OBJECT, so can be freed
		 */
		host_server_t* hs = nullptr;
		uid::uid_generator::uid_t uid;
		connection_metadata::client client;
		config::server_config server_config;
		connection_metadata::ptr connection;
		websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread;
	};

} // namespace flow::server

#endif //WM_SERVER_HPP
