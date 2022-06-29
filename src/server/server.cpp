//
// Created by joseph on 05/05/22.
//

#include "server.hpp"
#include <chrono>
#include <cpr/cpr.h>
#include <cstdlib>
#include <exception>
#include <logger/logger.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <websocketpp/close.hpp>
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/common/memory.hpp>
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
#ifdef DEBUG
		server.set_reuse_addr(true);
#endif
		server.set_message_handler(std::bind(&host_server_t::on_msg, this, std::placeholders::_1, std::placeholders::_2));
		server.set_open_handler(std::bind(&host_server_t::on_open, this, std::placeholders::_1));
		server.set_close_handler(std::bind(&host_server_t::on_close, this, std::placeholders::_1));
		server.set_user_agent(server::AUTH_STRING);
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
		client.hdl = std::move(hdl);
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
		struct ip_t
		{
			ip_t() = default;
			ip_t(int a, int b, int c, int d, int port)
				: a(a), b(b), c(c), d(d), port(port) {}
			static inline ip_t zero() { return ip_t(0, 0, 0, 0, 0); }

			int a;
			int b;
			int c;
			int d;
			int port;

			inline std::string to_str() const { return std::to_string(a) + "." + std::to_string(b) + "." + std::to_string(c) + "." + std::to_string(d) + ":" + std::to_string(port); }
		};
	} // namespace internal

	inline internal::ip_t scan_range(internal::ip_t begin, internal::ip_t end, unsigned int thread_count)
	{
		const int take_amount = 50;
		const int timeout = 100; //ms

		std::mutex m;
		std::vector<internal::ip_t> ips = {};
		ips.reserve((begin.a * (end.a + 1)) * (begin.b * (end.b + 1)) * (begin.c * (end.c + 1)) * (begin.d * (end.d + 1)));
		internal::ip_t begin_backup = begin;
		for (begin.a = begin_backup.a; begin.a <= end.a; ++begin.a)
		{
			for (begin.b = begin_backup.b; begin.b <= end.b; ++begin.b)
			{
				for (begin.c = begin_backup.c; begin.c <= end.c; ++begin.c)
				{
					for (begin.d = begin_backup.d; begin.d <= end.d; ++begin.d)
					{
						ips.emplace_back(begin);
					}
				}
			}
		}

		std::mutex m2;
		bool quit = false;
		std::vector<std::thread> threads;
		internal::ip_t server_ip = internal::ip_t::zero();
		auto should_quit = [&]() { std::lock_guard<std::mutex> lk(m2); return quit; };
		auto func = [&]() {
			std::vector<internal::ip_t> local_ips;
			std::vector<cpr::AsyncResponse> responses;
			while (!should_quit())
			{
				responses.reserve(take_amount);
				local_ips.reserve(take_amount);
				{
					std::lock_guard<std::mutex> lk(m);
					if (ips.size() > take_amount)
					{
						local_ips.assign(ips.end() - take_amount, ips.end());
						ips.erase(ips.end() - take_amount, ips.end());
					}
					else if (!ips.empty())
					{
						local_ips.assign(ips.begin(), ips.end());
						ips.erase(ips.begin(), ips.end());
					}
					else
					{
						break;
					}
				}

				for (const auto& item : local_ips)
				{

					responses.emplace_back(cpr::GetAsync(cpr::Url {item.to_str()}, cpr::Timeout {timeout}));
				}

				for (int i = 0; i < take_amount; ++i)
				{
					responses[i].wait();
					cpr::Response resp = responses[i].get();
					auto s_head = resp.header.find("Server");
					if (s_head != resp.header.end() && s_head->second == server::AUTH_STRING)
					{
						std::lock_guard<std::mutex> lk(m2);
						server_ip = local_ips[i];
						quit = true;
					}
				}

				local_ips.erase(local_ips.begin(), local_ips.end());
				responses.erase(responses.begin(), responses.end());
			}
		};

		for (int i = 0; i < thread_count; ++i)
		{
			threads.emplace_back(func);
		}

		for (int i = 0; i < thread_count; ++i)
		{
			if (threads[i].joinable()) threads[i].join();
		}

		return server_ip;
	}

	std::string guest_client_t::scan()
	{
		const int thread_multiplier = 1;
		auto thread_count = std::thread::hardware_concurrency() * thread_multiplier; // Total thread count *= 3
		auto is_ip_zero = [](internal::ip_t ip) { return ip.a == 0 && ip.b == 0 && ip.c == 0 && ip.d == 0 && ip.port == 0; };

		auto result = scan_range(internal::ip_t(192, 168, 0, 0, SERVER_PORT), internal::ip_t(192, 168, 255, 255, SERVER_PORT), thread_count);
		if (!is_ip_zero(result)) return result.to_str();

		result = scan_range(internal::ip_t(172, 16, 0, 0, SERVER_PORT), internal::ip_t(172, 31, 255, 255, SERVER_PORT), thread_count);
		if (!is_ip_zero(result)) return result.to_str();

		result = scan_range(internal::ip_t(10, 0, 0, 0, SERVER_PORT), internal::ip_t(10, 255, 255, 255, SERVER_PORT), thread_count);
		if (!is_ip_zero(result)) return result.to_str();

		return "";
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

	void guest_client_t::connect(lib_wm::WindowManager& wm)
	{
		auto url = scan();
		if (url.empty())
		{
			hs = new host_server_t();
			handlers::init_handlers(wm, ( void* ) this, ( void* ) hs);
			hs->run();
			url = "ws://127.0.0.1:" + std::to_string(SERVER_PORT);
		}
		handlers::init_handlers(wm, ( void* ) this, ( void* ) hs);
		internal_connect("ws://" + url);
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
