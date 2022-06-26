//
// Created by joseph on 05/05/22.
//

#ifndef WM_SERVER_HPP
#define WM_SERVER_HPP

#include "../buffer/buffer.hpp"
#include "../config/config.hpp"
#include "../uid/uid.hpp"
#include "Poco/Format.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"
#include "src/config/config.hpp"
#include <functional>
#include <logger/logger.hpp>
#include <mutex>
#include <thread>
#include <vector>
#include <wm/flow_wm.hpp>

using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPMessage;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

namespace flow::server
{

	const int SERVER_PORT = 16542;

	class PageRequestHandler : public HTTPRequestHandler
	{
	public:
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;
	};

	class WebSocketRequestHandler : public HTTPRequestHandler
	{
	public:
		WebSocketRequestHandler();
		void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;

	private:
		buffers::server_buffer_t buffer;
	};

	class RequestHandlerFactory : public HTTPRequestHandlerFactory
	{
	public:
		HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) override;
	};

	struct ws_client_t
	{
		WebSocket* ws;
		config::server_config config;
		uid::uid_generator::uid_t uid;
	};

	/*
     * HOST SERVER, ONLY ONE AT TIME
     */
	class host_server_t
	{
	public:
		host_server_t();
		void run();

		uid::uid_generator::uid_t add_server(WebSocket& ws, config::server_config& config);

	private:
		HTTPServer server;
		uid::uid_generator uid_gen;
		std::vector<ws_client_t> websocket_clients;
	};

	class guest_client_t
	{
	public:
		guest_client_t();
		~guest_client_t();

		void connect();

		template <typename T>
		void send_msg(const T& t, buffers::server_buffer_t& buffer)
		{
			auto result = buffer.write(t);
			std::lock_guard<std::mutex> lk(m);
			ws_ptr->sendFrame(result.data, result.size, WebSocket::FRAME_BINARY);
		}

	private:
		std::string scan();
		void internal_connect(const std::string& url);

		/*
		 * hs STORES HOST SERVER PTR ON THIS OBJECT, so can be freed
		 */
		std::mutex m;
		WebSocket* ws_ptr = nullptr;
		host_server_t* hs = nullptr;
		uid::uid_generator::uid_t uid;
		std::thread guest_server_thread;
	};

} // namespace flow::server

#endif //WM_SERVER_HPP
