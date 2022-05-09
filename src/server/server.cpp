//
// Created by joseph on 05/05/22.
//

#include "server.hpp"
#include "../macro/macro.hpp"
#include "../messages/messages.hpp"
#define class struct
#define private public
#define protected public
#include <Poco/Net/WebSocketImpl.h>
#undef class
#undef private
#undef protected
#include <logger/logger.hpp>
#include <thread>

namespace flow::server
{

	flow_wm_server_t::flow_wm_server_t(int port)
		: server_socket(port), server(new RequestHandlerFactory(), server_socket, new HTTPServerParams)
	{
	}

	flow_wm_server_t::~flow_wm_server_t()
	{
		stop();
	}

	void flow_wm_server_t::run()
	{
		server.start();
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(1000min);
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
		ostr << "        ws.send(\"Hello, world!\");";
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


	inline int receive_bytes(Poco::Net::WebSocketImpl* impl, server_buffer_t& buffer, int)
	{
		char mask[4];
		bool use_mask;
		impl->_frameFlags = 0;
		int payload_length = impl->receiveHeader(mask, use_mask);
		if (payload_length <= 0) return payload_length;
		buffer.resize(payload_length);
		return impl->receivePayload(reinterpret_cast<char*>(buffer.get_data()), payload_length, mask, use_mask);
	}

	inline int receive_frame(WebSocket& web_socket, server_buffer_t& buffer, int& flags)
	{
		int n = receive_bytes(dynamic_cast<Poco::Net::WebSocketImpl*>(web_socket.impl()), buffer, 0);;
		flags = dynamic_cast<Poco::Net::WebSocketImpl*>(web_socket.impl())->frameFlags();
		return n;
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
				logger::notify(Poco::format("Frame received (length=%d, flags=0x%x).", n, unsigned(flags)));
				logger::notify("VALUE:", ( char* ) buffer.get_data());
				ws.sendFrame(buffer.get_data(), n, flags);
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
