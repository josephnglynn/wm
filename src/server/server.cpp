//
// Created by joseph on 05/05/22.
//

#include "server.hpp"
#include "../buffer/buffer.hpp"
#include "../config/config.hpp"
#include "../deserialize/deserialize.hpp"
#include "../handlers/handlers.hpp"
#include "../messages/messages.hpp"
#include "../uid/uid.hpp"
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
#include <websocketpp/close.hpp>
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/common/memory.hpp>
#include <websocketpp/common/system_error.hpp>
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
		server.set_open_handler(std::bind(&host_server_t::on_open, this, std::placeholders::_1));
		server.set_close_handler(std::bind(&host_server_t::on_close, this, std::placeholders::_1));
	}

	host_server_t::~host_server_t()
	{
		// TODO DESTRUCTOR
	}

	void host_server_t::run()
	{
		internal_server_thread = std::thread([&] {
			server.listen(SERVER_PORT);
			server.start_accept();
			server.run();
		});
	}

	uid::uid_generator::uid_t host_server_t::add_server(websocketpp::connection_hdl hdl, config::server_config& config)
	{
		ws_client_t client;
		client.hdl = hdl;
		client.config = config;
		client.uid = uid_gen.get_next_uid();

		websocket_clients[client.uid] = client;

		return client.uid;
	}

	guest_client_t::guest_client_t()
	{
		client.clear_access_channels(websocketpp::log::alevel::all);
		client.clear_error_channels(websocketpp::log::elevel::all);

		client.init_asio();
		client.start_perpetual();

		thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&web_client_t::run, &client);
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
		web_client_t::connection_ptr con_ptr = client.get_connection(url, ec);

		if (ec)
		{
			logger::error(ec.message());
			std::exit(-1); //TODO FIX GRACEFULLY
		}

		websocketpp::lib::shared_ptr<connection_metadata> ptr = websocketpp::lib::make_shared<connection_metadata>(con_ptr->get_handle(), url);

		connection = ptr;

		con_ptr->set_open_handler(std::bind(&connection_metadata::on_open, ptr, &client, websocketpp::lib::placeholders::_1));
		con_ptr->set_fail_handler(std::bind(&connection_metadata::on_fail, ptr, &client, websocketpp::lib::placeholders::_1));
		con_ptr->set_close_handler(std::bind(&connection_metadata::on_close, ptr, &client, websocketpp::lib::placeholders::_1));
		con_ptr->set_message_handler(std::bind(&connection_metadata::on_message, ptr, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));

		client.connect(con_ptr);

		do {
			std::this_thread::sleep_for(100ms);
		} while (connection->get_status() == ConnectionStatus::Connecting);

		if (connection->get_status() != ConnectionStatus::Open)
		{
			logger::error("MASSIVE ERROR WE CANT FIX NOW");
			std::exit(-1);
		}

		messages::message_host_connect_test_request_t msg;
		client.send(ptr->get_hdl(), &msg, sizeof(msg), websocketpp::frame::opcode::BINARY, ec);
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
		client.stop_perpetual();

		websocketpp::lib::error_code ec;
		client.close(connection->get_hdl(), websocketpp::close::status::going_away, "", ec);

		thread->join();
	}
} // namespace flow::server
