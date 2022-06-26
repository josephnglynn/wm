//
// Created by joseph on 05/05/22.
//

#include "server.hpp"
#include "src/buffer/buffer.hpp"
#include "src/config/config.hpp"
#include "src/deserialize/deserialize.hpp"
#include "src/messages/messages.hpp"
#include "src/uid/uid.hpp"
#include "wm/flow_wm.hpp"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/WebSocket.h>
#include <condition_variable>
#include <cstdlib>
#include <exception>
#include <mutex>
#include <string>
#define class struct
#define private public
#define protected public
#include <Poco/Net/WebSocketImpl.h>
#undef class
#undef private
#undef protected
#include "../handlers/handlers.hpp"
#include <chrono>
#include <fstream>
#include <ifaddrs.h>
#include <logger/logger.hpp>
#include <thread>

using namespace std::chrono_literals;

namespace flow::server
{

	inline int receive_bytes(Poco::Net::WebSocketImpl* impl, buffers::server_buffer_t& buffer, int)
	{
		char mask[4];
		bool use_mask;
		impl->_frameFlags = 0;
		int payload_length = impl->receiveHeader(mask, use_mask);
		if (payload_length <= 0) return payload_length;
		buffer.resize(payload_length);
		return impl->receivePayload(reinterpret_cast<char*>(buffer.get_data()), payload_length, mask, use_mask);
	}

	inline int receive_frame(WebSocket& web_socket, buffers::server_buffer_t& buffer, int& flags)
	{
		int n = receive_bytes(dynamic_cast<Poco::Net::WebSocketImpl*>(web_socket.impl()), buffer, 0);
		flags = dynamic_cast<Poco::Net::WebSocketImpl*>(web_socket.impl())->frameFlags();
		return n;
	}

	void PageRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		/*
		 * CAN BE USED IN BROWSER TO TEST SERVER IS STARTING => TAKEN FROM OFFICIAL DOCS
		 */
#ifdef DEBUG
		response.setChunkedTransferEncoding(true);
		response.setContentType("text/html");
		std::ostream& ostr = response.send();
		ostr << "<html>";
		ostr << "<head>";
		ostr << "<title>WebSocketServer</title>";
		ostr << "<script type=\"text/javascript\">";
		ostr << "function WebSocketTest()";
		ostr << "{";
		ostr << "  if (\"WebSocket\" in window)";
		ostr << "  {";
		ostr << "    var ws = new WebSocket(\"ws://" << request.serverAddress().toString() << "/ws\");";
		ostr << "    ws.onopen = function()";
		ostr << "      {";
		ostr << "        var buf = new ArrayBuffer(4);";
		ostr << "        var result = new Uint8Array(buf);";
		ostr << "        result[0] = 1;";
		ostr << "        ws.send(result.buffer);";
		ostr << "      };";
		ostr << "    ws.onmessage = function(evt)";
		ostr << "      { ";
		ostr << "        var msg = evt.data;";
		ostr << "        alert(\"Message received: \" + msg);";
		ostr << "        ws.close();";
		ostr << "      };";
		ostr << "    ws.onclose = function()";
		ostr << "      { ";
		ostr << "        alert(\"WebSocket closed.\");";
		ostr << "      };";
		ostr << "  }";
		ostr << "  else";
		ostr << "  {";
		ostr << "     alert(\"This browser does not support WebSockets.\");";
		ostr << "  }";
		ostr << "}";
		ostr << "</script>";
		ostr << "</head>";
		ostr << "<body>";
		ostr << "  <h1>WebSocket Server</h1>";
		ostr << "  <p><a href=\"javascript:WebSocketTest()\">Run WebSocket Script</a></p>";
		ostr << "</body>";
		ostr << "</html>";
#endif
	}

	HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
	{
		if (request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
			return new WebSocketRequestHandler;
		else
			return new PageRequestHandler;
	}

	void WebSocketRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		try
		{
			WebSocket ws(request, response);
			logger::notify<logger::Debug>("WebSocket connection established.");
			int n, flags;
			do
			{
				n = receive_frame(ws, buffer, flags);
				auto msg = reinterpret_cast<messages::message_base_request_t*>(buffer.get_data());
#ifdef DEBUG
				if (msg->type > messages::_number_of_request_types)
				{
					logger::error("How did we get a message with invalid type?");
					continue;
				}
#endif
				handlers::request_handlers[msg->type](buffer, ws);
			} while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
			logger::notify<logger::Debug>("WebSocket connection closed.");
		}
		catch (WebSocketException& exc)
		{
			switch (exc.code())
			{
				case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
					response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
					// fallthrough
				case WebSocket::WS_ERR_NO_HANDSHAKE:
				case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
				case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
					response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
					response.setContentLength(0);
					response.send();
					break;
			}
		}
	}

	WebSocketRequestHandler::WebSocketRequestHandler()
		: buffer(2048)
	{
	}

	host_server_t::host_server_t()
		: server(new RequestHandlerFactory(), SERVER_PORT, new HTTPServerParams())
	{
	}

	void host_server_t::run()
	{
		server.start();
	}

	uid::uid_generator::uid_t host_server_t::add_server(WebSocket& ws, config::server_config& config)
	{
		ws_client_t client;
		client.ws = &ws;
		client.config = config;
		client.uid = uid_gen.get_next_uid();

		websocket_clients.push_back(client);

		return client.uid;
	}

	guest_client_t::guest_client_t()
	{
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
		HTTPClientSession cs(url, SERVER_PORT);
		HTTPRequest request(HTTPRequest::HTTP_GET, "/?encoding=text", HTTPMessage::HTTP_1_1);
		HTTPResponse response;

		try
		{
			WebSocket socket = WebSocket(cs, request, response);
			messages::message_host_connect_test_request_t req;
			auto result = buf.write(req);
			socket.sendFrame(result.data, result.size, WebSocket::FRAME_BINARY);

			return true;
		}
		catch (std::exception& e)
		{
			//logger::notify("ERROR");
			return false;
		}
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

	void guest_client_t::connect()
	{
		auto url = scan();
		if (url.empty())
		{
			hs = new host_server_t();
			hs->run();
			url = "127.0.0.1:" + std::to_string(SERVER_PORT);
		}

		guest_server_thread = std::thread([&, copy_url = url] { this->internal_connect(copy_url); });
	}

	void guest_client_t::internal_connect(const std::string& url)
	{
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

	guest_client_t::~guest_client_t()
	{
		delete hs;
	}
} // namespace flow::server
