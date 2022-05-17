//
// Created by joseph on 05/05/22.
//

#include "server.hpp"
#include "wm/flow_wm.hpp"
#include <Poco/Net/WebSocket.h>
#include <cstdlib>
#include <exception>
#include <string>
#define class struct
#define private public
#define protected public
#include <Poco/Net/WebSocketImpl.h>
#undef class
#undef private
#undef protected
#include "../handlers/handlers.hpp"
#include <fstream>
#include <ifaddrs.h>
#include <logger/logger.hpp>
#include <thread>

namespace flow::server
{

	flow_wm_server_t::flow_wm_server_t(lib_wm::WindowManager& wm, server_data_t&& server_data, int port)
		: server_port(port),
		  server_socket(port),
		  server(new RequestHandlerFactory, server_socket, new HTTPServerParams),
		  server_data(server_data)
	{
		handlers::init_handlers(wm, *this);
	}

	flow_wm_server_t::~flow_wm_server_t()
	{
		stop();
	}

	std::vector<std::string> get_all_ips()
	{
		ifaddrs* if_address = nullptr;
		getifaddrs(&if_address);

		std::vector<std::string> addresses;

		for (ifaddrs* i = if_address; i != nullptr; i = i->ifa_next)
		{
			if (i->ifa_addr->sa_family == AF_INET)
			{
				char address_buf[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(( sockaddr_in* ) i->ifa_addr)->sin_addr, address_buf, INET_ADDRSTRLEN);
				addresses.emplace_back(address_buf);
			}
			else if (i->ifa_addr->sa_family == AF_INET6)
			{
				char address_buf[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, &(( struct sockaddr_in6* ) i->ifa_addr)->sin6_addr, address_buf, INET6_ADDRSTRLEN);
				logger::warn<logger::Debug>("UMM, why ipv6?");
				addresses.emplace_back(address_buf);
			}
		}

		if (if_address != nullptr) freeifaddrs(if_address);
		return addresses;
	}

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

	void flow_wm_server_t::run()
	{
		server.start();
		logger::notify<logger::Debug>("STARTING SERVER ON PORT:", server_port);
		const auto static_port = server_port;

		const auto home = std::string(std::getenv("HOME"));
		std::ifstream ips_file(home + "/.config/flow_wm/ip_addresses");
		if (!ips_file.good()) return;

		auto current_ip = get_all_ips();
		std::string line;
		while (std::getline(ips_file, line))
		{
			for (const auto& ip : current_ip)
			{
				if (ip == line) goto continue_loop;
			}

			for (const auto& ip : ips)
			{
				if (ip == line) goto continue_loop;
			}


			ips.push_back(line);
			client_threads.emplace_back(new WebSocketClient([line, static_port](WebSocketClient* client) {
				try
				{
					HTTPClientSession cs(line, static_port);
					HTTPRequest request(HTTPRequest::HTTP_GET, "/?encoding=text", HTTPMessage::HTTP_1_1);
					HTTPResponse response;

					{
						std::lock_guard<std::mutex> lock(client->mutex);
						client->socket = new WebSocket(cs, request, response);
						messages::message_sync_wm_servers_request_t msg;
						client->socket->sendFrame(&msg, sizeof(msg), WebSocket::FRAME_BINARY);
					}

					logger::notify<logger::Debug>("WebSocket connection established.");
					buffers::server_buffer_t buffer = buffers::server_buffer_t(1024);
					int n, flags;
					do
					{
						client->mutex.lock();
						n = receive_frame(*client->socket, buffer, flags);
						client->mutex.unlock();

						auto msg = reinterpret_cast<messages::message_base_request_t*>(buffer.get_data());
#ifdef DEBUG
						if (msg->type > messages::_number_of_request_types)
						{
							logger::error("How did we get a message with invalid type?");
							continue;
						}
#endif
						handlers::response_handlers[msg->type](*client, buffer);
					} while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
					logger::notify<logger::Debug>("WebSocket connection closed.");
				}
				catch (std::exception& e)
				{
					logger::error(e.what());
				}

				client->socket = nullptr;
			}));

		continue_loop:;
		}
	}

	void flow_wm_server_t::stop()
	{
		server.stop();
	}

	HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
	{
		if (request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
			return new WebSocketRequestHandler;
		else
			return new PageRequestHandler;
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
				handlers::request_handlers[msg->type](ws, buffer);
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

	/*
	 * CHECK DEFAULT BUFFER SIZE
	 */
	WebSocketRequestHandler::WebSocketRequestHandler()
		: HTTPRequestHandler(), buffer(2048)
	{
	}

} // namespace flow::server
