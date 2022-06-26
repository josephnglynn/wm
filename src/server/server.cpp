//
// Created by joseph on 05/05/22.
//

#include "server.hpp"
#include "../handlers/handlers.hpp"
#include "src/buffer/buffer.hpp"
#include "src/config/config.hpp"
#include "src/deserialize/deserialize.hpp"
#include "src/messages/messages.hpp"
#include "src/uid/uid.hpp"
#include "wm/flow_wm.hpp"
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <ifaddrs.h>
#include <logger/logger.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/common/memory.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/frame.hpp>
#include <websocketpp/logger/levels.hpp>

using namespace std::chrono_literals;

namespace flow::server
{

	host_server_t::host_server_t()
	{
		server.set_error_channels(websocketpp::log::elevel::fatal);
		server.set_access_channels(websocketpp::log::alevel::fail);

		server.init_asio();
		server.set_message_handler(std::bind(&host_server_t::on_msg, this, std::placeholders::_1, std::placeholders::_2));
	}

	void host_server_t::run()
	{
		internal_server_thread = std::thread([&] {
			server.listen(SERVER_PORT);
			server.start_accept();
			server.run();
		});
	}

	void on_msg(websocketpp::connection_hdl hdl, host_server_t::server_t::message_ptr msg)
	{
		const auto* data = dynamic_cast<messages::message_base_request_t*>(msg->get_payload().data());
		if (data->type > messages::message_type::_number_of_message_types) return;
		if (data->uid > 0)
		{

			return;
		}
		handlers::request_handlers[data->type]();
	}

	uid::uid_generator::uid_t host_server_t::add_server(websocketpp::connection_hdl hdl, config::server_config& config)
	{
		ws_client_t client;
		client.hdl = hdl;
		client.config = config;
		client.uid = uid_gen.get_next_uid();

		websocket_clients.push_back(client);

		return client.uid;
	}

	guest_client_t::guest_client_t()
	{
		client.clear_access_channels(websocketpp::log::alevel::all);
		client.clear_error_channels(websocketpp::log::elevel::all);

		client.init_asio();
		client.start_perpetual();

		thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&connection_metadata::client::run, &client);
	}

	namespace internal
	{
		/*
		 * INCLUSIVE
		 */
		struct range
		{
			range() = default;
			range(int both)
				: begin(both), end(both) {}
			range(int b, int e)
				: begin(b), end(e) {}

			int begin;
			int end;
		};
	} // namespace internal

	inline bool test_ip(const std::string& url, buffers::server_buffer_t& buf)
	{
		return false;
	}

	class str_con
	{
	public:
		inline void set_value(const std::string& new_value)
		{
			{
				std::lock_guard<std::mutex> lock(m);
				value = new_value;
			}
		}

		inline std::mutex& get_mutex() { return m; }
		inline const std::string& get_value() { return value; }

	private:
		std::mutex m;
		std::string value;
	};

	inline void scan_range(str_con* str, internal::range ar, internal::range br, internal::range cr, internal::range dr)
	{
		buffers::server_buffer_t buf(2048);
		for (int a = ar.begin; a <= ar.end; ++a)
		{
			for (int b = br.begin; b <= br.end; ++b)
			{
				for (int c = cr.begin; c <= cr.end; ++c)
				{
					for (int d = dr.begin; d <= dr.end; ++d)
					{
						const std::string url = std::to_string(a) + "." + std::to_string(b) + "." + std::to_string(c) + "." + std::to_string(d) + ":" + std::to_string(SERVER_PORT);
						if (test_ip(url, buf))
						{
							str->set_value(url);
							return;
						}
					}
				}
			}
		}
	}

	inline int get_number_of_threads_alive(std::vector<std::vector<std::thread>>& t)
	{
		int count = 0;
		for (const auto& a : t)
		{
			for (const auto& b : a)
			{
				count += b.joinable();
			}
		}

		return count;
	}

	std::string guest_client_t::scan()
	{
		const int thread_multiplier = 1;
		auto thread_count = std::thread::hardware_concurrency() * thread_multiplier; // Total thread count *= 3

		str_con str;

		std::vector<std::vector<std::thread>> thread_containter;

		{
			thread_containter.emplace_back();
			std::vector<std::thread>& threads = thread_containter[0];
			threads.reserve(thread_count);

			const auto interval = 255 / thread_count;
			for (auto i = 0; i < thread_count; ++i)
			{
				threads.emplace_back(scan_range, &str, internal::range(192), internal::range(168), internal::range(i * interval, (i + 1) * interval), internal::range(0, 255));
			}

			threads.emplace_back(scan_range, &str, internal::range(192), internal::range(168), internal::range(thread_count * interval, 255), internal::range(0, 255));
		}

		{
			thread_containter.emplace_back();
			std::vector<std::thread>& threads = thread_containter[1];
			threads.reserve(thread_count);

			const auto interval = 15 / thread_count;
			for (auto i = 0; i < thread_count; ++i)
			{
				threads.emplace_back(scan_range, &str, internal::range(172), internal::range(16 + i * interval, (i + 1) * interval), internal::range(0, 255), internal::range(0, 255));
			}

			threads.emplace_back(scan_range, &str, internal::range(172), internal::range(16 + thread_count * interval, 31), internal::range(0, 255), internal::range(0, 255));
		}

		{
			thread_containter.emplace_back();
			std::vector<std::thread>& threads = thread_containter[0];
			threads.reserve(thread_count);

			const auto interval = 255 / thread_count;
			for (auto i = 0; i < thread_count; ++i)
			{
				threads.emplace_back(scan_range, &str, internal::range(10), internal::range(i * interval, (i + 1) * interval), internal::range(0, 255), internal::range(0, 255));
			}

			threads.emplace_back(scan_range, &str, internal::range(10), internal::range(thread_count * interval, 255), internal::range(0, 255), internal::range(0, 255));
		}

		do {
			std::this_thread::sleep_for(100ms);
		} while (str.get_value().empty() && get_number_of_threads_alive(thread_containter) != 0);

		return str.get_value();
	}

	void guest_client_t::internal_connect(const std::string& url)
	{

		websocketpp::lib::error_code ec;
		connection_metadata::client::connection_ptr con = client.get_connection(url, ec);

		if (ec)
		{
			logger::error(ec.message());
			std::exit(-1); //TODO FIX GRACEFULLY
		}

		// https://github.com/zaphoyd/websocketpp/blob/master/tutorials/utility_client/step6.cpp
		// TODO FINISH BASIC CLIENT

		HTTPClientSession cs(url, SERVER_PORT);
		HTTPRequest request(HTTPRequest::HTTP_GET, "/?encoding=text", HTTPMessage::HTTP_1_1);
		HTTPResponse response;

		WebSocket ws(cs, request, response);
		ws_ptr = &ws;

		buffers::server_buffer_t internal_buffer(2048);
		/*
         * CHECK IF REAL SERVER
         */
		{
			messages::message_host_connect_test_request_t msg(0);
			auto res = internal_buffer.write(msg);
			ws.sendFrame(res.data, res.size, WebSocket::FRAME_BINARY);
		}

		int n, flags;

		/*
         * WAIT FOR RESPONSE TO OUR TEST REQUEST
         */
		{
			int count = 0;
			do {
				std::lock_guard<std::mutex> lk(m);
				n = receive_frame(ws, internal_buffer, flags);
				auto msg = reinterpret_cast<messages::message_base_response_t*>(internal_buffer.get_data());

				if (msg->type > messages::_number_of_request_types)
				{
					continue;
				}
				if (msg->type == messages::message_type_response::host_connect_test_response) { break; }
				++count;
			} while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE && count < 10); // TODO: 10 is arbitary, change

			if (count == 10)
			{
				logger::error("FATAL ERROR, server url is incorrect");
				std::exit(-1);
			}

			messages::message_host_initial_connect_request_t msg;
			auto res = internal_buffer.write(msg);

			std::lock_guard<std::mutex> lk(m);
			ws.sendFrame(res.data, res.size, WebSocket::FRAME_BINARY);
		}

		do {
			std::lock_guard<std::mutex> lk(m);
			n = receive_frame(ws, internal_buffer, flags);
			auto msg = reinterpret_cast<messages::message_base_response_t*>(internal_buffer.get_data());

			if (msg->type > messages::_number_of_request_types)
			{
				continue;
			}

			handlers::response_handlers[msg->type](internal_buffer);

		} while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
	}

	void guest_client_t::connect()
	{
		auto url = scan();
		if (url.empty())
		{
			hs = new host_server_t();
			hs->run();
			url = "127.0.0.1:" + std::to_string(SERVER_PORT);
		}
	}

	guest_client_t::~guest_client_t()
	{
		delete hs;
	}
} // namespace flow::server
