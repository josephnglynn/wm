//
// Created by joseph on 05/05/22.
//

#ifndef WM_SERVER_HPP
#define WM_SERVER_HPP

#include "../buffer/buffer.hpp"
#include "Poco/Format.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
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
#include <logger/logger.hpp>
#include <wm/flow_wm.hpp>

using Poco::ThreadPool;
using Poco::Timestamp;
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

	class flow_wm_server_t
	{
	public:
		flow_wm_server_t(lib_wm::window_manager_t& wm, int port = 9600);
		~flow_wm_server_t();

		void run();
		void stop();

		inline const auto& get_server_data() { return server_data; }

	private:
		HTTPServer server;
		server_data_t server_data;
		ServerSocket server_socket;
	};

} // namespace flow::server

#endif //WM_SERVER_HPP
