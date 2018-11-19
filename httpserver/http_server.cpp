#include <utility>
#include "http_server.h"

void HttpServer::Init(const std::string &port)
{
	m_port = port;
	s_server_option.enable_directory_listing = "yes";
	s_server_option.document_root = s_web_dir.c_str();

	// TODO：其他http设置
}

bool HttpServer::Start()
{
	mg_mgr_init(&m_mgr, NULL);
	mg_connection *connection = mg_bind(&m_mgr, m_port.c_str(), HttpServer::OnHttpWebsocketEvent);
	if (connection == NULL)
		return false;
	// for both http and websocket
	mg_set_protocol_http_websocket(connection);

	printf("starting http server at port: %s\n", m_port.c_str());
	// loop
	while (true)
		mg_mgr_poll(&m_mgr, 500); // ms

	return true;
}

void HttpServer::OnHttpWebsocketEvent(mg_connection *connection, int event_type, void *event_data)
{
	// 区分http和websocket
	if (event_type == MG_EV_HTTP_REQUEST)
	{
		http_message *http_req = (http_message *)event_data;
		HandleHttpEvent(connection, http_req);
	}
	else if (event_type == MG_EV_WEBSOCKET_HANDSHAKE_DONE ||
		     event_type == MG_EV_WEBSOCKET_FRAME ||
		     event_type == MG_EV_CLOSE)
	{
		websocket_message *ws_message = (struct websocket_message *)event_data;
		HandleWebsocketMessage(connection, event_type, ws_message);
	}
}

// ---- simple http ---- //
static bool route_check(http_message *http_msg, char *route_prefix)
{
	if (mg_vcmp(&http_msg->uri, route_prefix) == 0)
		return true;
	else
		return false;

	// TODO: 还可以判断 GET, POST, PUT, DELTE等方法
	//mg_vcmp(&http_msg->method, "GET");
	//mg_vcmp(&http_msg->method, "POST");
	//mg_vcmp(&http_msg->method, "PUT");
	//mg_vcmp(&http_msg->method, "DELETE");
}

void HttpServer::AddHandler(const std::string &url, ReqHandler req_handler)
{
	if (s_handler_map.find(url) != s_handler_map.end())
		return;

	s_handler_map.insert(std::make_pair(url, req_handler));
}

void HttpServer::RemoveHandler(const std::string &url)
{
	auto it = s_handler_map.find(url);
	if (it != s_handler_map.end())
		s_handler_map.erase(it);
}

void HttpServer::SendHttpRsp(mg_connection *connection, std::string rsp)
{
	// 必须先发送header
	mg_printf(connection, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
	// 以json形式返回
	mg_printf_http_chunk(connection, "{ \"result\": %s }", rsp.c_str());
	// 发送空白字符快，结束当前响应
	mg_send_http_chunk(connection, "", 0);
}

void HttpServer::HandleHttpEvent(mg_connection *connection, http_message *http_req)
{
	std::string req_str = std::string(http_req->message.p, http_req->message.len);
	printf("got request: %s\n", req_str.c_str());

	// 先过滤是否已注册的函数回调
	std::string url = std::string(http_req->uri.p, http_req->uri.len);
	std::string body = std::string(http_req->body.p, http_req->body.len);
	auto it = s_handler_map.find(url);
	if (it != s_handler_map.end())
	{
		ReqHandler handle_func = it->second;
		handle_func(url, body, connection, &HttpServer::SendHttpRsp);
	}

	// 其他请求
	if (route_check(http_req, "/")) // index page
		mg_serve_http(connection, http_req, s_server_option);
	else if (route_check(http_req, "/api/hello")) 
	{
		// 直接回传
		SendHttpRsp(connection, "welcome to httpserver");
	}
	else if (route_check(http_req, "/api/sum"))
	{
		// 简单post请求，加法运算测试
		char n1[100], n2[100];
		double result;

		/* Get form variables */
		mg_get_http_var(&http_req->body, "n1", n1, sizeof(n1));
		mg_get_http_var(&http_req->body, "n2", n2, sizeof(n2));

		/* Compute the result and send it back as a JSON object */
		result = strtod(n1, NULL) + strtod(n2, NULL);
		SendHttpRsp(connection, std::to_string(result));
	}
	else
	{
		mg_printf(
			connection,
			"%s",
			"HTTP/1.1 501 Not Implemented\r\n"
			"Content-Length: 0\r\n\r\n");
	}
}

// ---- websocket ---- //
int HttpServer::isWebsocket(const mg_connection *connection)
{
	return connection->flags & MG_F_IS_WEBSOCKET;
}

void HttpServer::HandleWebsocketMessage(mg_connection *connection, int event_type, websocket_message *ws_msg)
{
	if (event_type == MG_EV_WEBSOCKET_HANDSHAKE_DONE)
	{
		printf("client websocket connected\n");
		// 获取连接客户端的IP和端口
		char addr[32];
		mg_sock_addr_to_str(&connection->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
		printf("client addr: %s\n", addr);

		// 添加 session
		s_websocket_session_set.insert(connection);

		SendWebsocketMsg(connection, "client websocket connected");
	}
	else if (event_type == MG_EV_WEBSOCKET_FRAME)
	{
		mg_str received_msg = {
			(char *)ws_msg->data, ws_msg->size
		};

		char buff[1024] = {0};
		strncpy(buff, received_msg.p, received_msg.len); // must use strncpy, specifiy memory pointer and length

		// do sth to process request
		printf("received msg: %s\n", buff);
		SendWebsocketMsg(connection, "send your msg back: " + std::string(buff));
		//BroadcastWebsocketMsg("broadcast msg: " + std::string(buff));
	}
	else if (event_type == MG_EV_CLOSE)
	{
		if (isWebsocket(connection))
		{
			printf("client websocket closed\n");
			// 移除session
			if (s_websocket_session_set.find(connection) != s_websocket_session_set.end())
				s_websocket_session_set.erase(connection);
		}
	}
}

void HttpServer::SendWebsocketMsg(mg_connection *connection, std::string msg)
{
	mg_send_websocket_frame(connection, WEBSOCKET_OP_TEXT, msg.c_str(), strlen(msg.c_str()));
}

void HttpServer::BroadcastWebsocketMsg(std::string msg)
{
	for (mg_connection *connection : s_websocket_session_set)
		mg_send_websocket_frame(connection, WEBSOCKET_OP_TEXT, msg.c_str(), strlen(msg.c_str()));
}

bool HttpServer::Close()
{
	mg_mgr_free(&m_mgr);
	return true;
}