#include <utility>
#include "http_server.h"
#include <iostream>
#include <thread>
#include <mutex>
#include "time.h"
#include "../common/functions.h"
#define cookielen 8

using namespace std;

int counting_req = 0;

struct requestMessage {
	string keywords[20] = {};
	string Fn = "", url, httpVersion;
	string reqinfo[40] = {};
	char body[];

	string findInfo(string Name_info) {
		
	}

	void readIn(string req) {
		int i = 0;
		while (req[i] != ' ') {
			this->Fn += req[i];
			i++;
		}
		cout << this->Fn;
		i+=url.length()+2;
		while (req[i] != '\r') {
			this->httpVersion += req[i];
			i++;
		}
		int flag = 0, black = 0;
		for (; i < req.length(); i++) {
			if (req[i] == '\r') {
				if (black == 0) {
					break;
				}
				black = 0;
				flag++;
				i++;
				continue;
			}
			black++;
			this->reqinfo[flag] += req[i];
		}
		if (this->Fn == "POST") {
			
		}
	}
};


long long time_now_sec()
{
	time_t tv;
	tv = time(NULL);//time(&tv); get current time;
	//std::cout << tv << std::endl;//距离1970-01-01 00:00:00经历的秒数
	return tv;
}

string time_now_str() {
	time_t tv;
	tv = time(NULL);//time(&tv); get current time;
	return ctime(&tv);
}

struct user {
	/*
	*level: 0==guest,1==user,2==admin,3==root
	* User can not use this website if tis able == false;
	*/
	string name = "";
	string password = "";
	int ID = 0;
	string cookie = "";
	string time_str_creation_cookie = "";
	long long time_num_creation_cookie = 0;
	bool able = false;
	int level = -1;
	string level_str = "";

	bool logined = false;
	bool time_login = "";
	int login(string username, string password) {
		/*If the funcution returns 0,username or password can't be matched.
		* If -1 is returned ,the user has been disabled.
		* If 1 is returned ,login is allowed.
		* If 2 is returned ,the user is logined.
		*/

		if (username == this->name && password == this->password) {
			if (this->logined == true) {
				return 2;
			}
			if (this->able) {
				this->logined = true;
				return 1;
			}else{
				return -1;
			}
		}else{
			return 0;
		}
	}
	bool login_cookie(string cookie) {
		if (cookie == this->cookie && this->able&&this->time_num_creation_cookie <= time_now_sec()) {
			this->logined = true;
			return true;
		}
		else if(this->time_num_creation_cookie > time_now_sec()){
			this->cookie = "";
		}
		return false;
	}

	void read_in(
		int ID,
		string name,
		int level,
		string password,
		string cookie,
		bool creation_time_cookie
		
	) {
		switch (level) {
		case 0:
			this->level_str += "Guset";
			break;
		case 1:
			this->level_str += "User";
			break;
		case 2:
			this->level_str += "Admin";
			break;
		case 3:
			this->level_str += "Root";
			break;
		default:
			this->level_str += "Error";
		}
		this->cookie += cookie;
		this->time_str_creation_cookie += creation_time_cookie;
		this->ID = ID;
		this->name += name;
		this->password += password;
	}
}users[44];

void readin() {
	int ID;
	string name;
	int level;
	string password;
	string cookie;
	bool creation_time_cookie;
	FILE* stream1;
	freopen_s(&stream1, "userinfo.dba", "r", stdin);
	for (int i = 0; i < 44; i++) {
		cin >> ID >> name >> level >> password >> cookie >> creation_time_cookie;
		users[i].read_in(ID, name, level, password, cookie, creation_time_cookie);
	}
	fclose(stdin);
}




void HttpServer::Init(const std::string& port)
{
	m_port = port;
	s_server_option.enable_directory_listing = "yes";
	s_server_option.document_root = s_web_dir.c_str();

	// 其他http设置

	// 开启 CORS，本项只针对主页加载有效
	// s_server_option.extra_headers = "Access-Control-Allow-Origin: *";
}

bool HttpServer::Start()
{
	mg_mgr_init(&m_mgr, NULL);
	mg_connection* connection = mg_bind(&m_mgr, m_port.c_str(), HttpServer::OnHttpWebsocketEvent);
	if (connection == NULL)
		return false;

	thread stat01(readin);

	// for both http and websocket
	thread stat02(mg_set_protocol_http_websocket,connection);

	printf("starting http server at port: %s\n", m_port.c_str());

	stat01.join();
	stat02.join();

	// loop
	while (true)
		mg_mgr_poll(&m_mgr, 500); // ms

	return true;
}

void HttpServer::OnHttpWebsocketEvent(mg_connection* connection, int event_type, void* event_data)
{
	// 区分http和websocket
	if (event_type == MG_EV_HTTP_REQUEST)
	{
		http_message* http_req = (http_message*)event_data;
		std::thread HandleHttpEvent(HttpServer::HandleHttpEvent, connection, http_req);
		HandleHttpEvent.join();
		//HandleHttpEvent(connection, http_req);
	}
	else if (event_type == MG_EV_WEBSOCKET_HANDSHAKE_DONE ||
		event_type == MG_EV_WEBSOCKET_FRAME ||
		event_type == MG_EV_CLOSE)
	{
		websocket_message* ws_message = (struct websocket_message*)event_data;
		HandleWebsocketMessage(connection, event_type, ws_message);
	}

}

// ---- simple http ---- //
static bool route_check(http_message* http_msg, char* route_prefix)
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

void HttpServer::AddHandler(const std::string& url, ReqHandler req_handler)
{
	if (s_handler_map.find(url) != s_handler_map.end())
		return;

	s_handler_map.insert(std::make_pair(url, req_handler));
}

void HttpServer::RemoveHandler(const std::string& url)
{
	auto it = s_handler_map.find(url);
	if (it != s_handler_map.end())
		s_handler_map.erase(it);
}

void HttpServer::SendHttpRsp(mg_connection* connection, std::string rsp)
{
	// --- 未开启CORS
	// 必须先发送header, 暂时还不能用HTTP/2.0
	mg_printf(connection, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
	// 以json形式返回
	mg_printf_http_chunk(connection, "{ \"result\": %s }", rsp.c_str());
	// 发送空白字符快，结束当前响应
	mg_send_http_chunk(connection, "", 0);

	// --- 开启CORS
	/*mg_printf(connection, "HTTP/1.1 200 OK\r\n"
			  "Content-Type: text/plain\n"
			  "Cache-Control: no-cache\n"
			  "Content-Length: %d\n"
			  "Access-Control-Allow-Origin: *\n\n"
			  "%s\n", rsp.length(), rsp.c_str()); */
}

void HttpServer::HandleHttpEvent(mg_connection* connection, http_message* http_req)
{
	std::string req_str = std::string(http_req->message.p, http_req->message.len);
	counting_req++;
	cout << "Time:" << time_now_sec()<<" counting:"<<counting_req<< endl;
	printf("%s\n", req_str.c_str());

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

	requestMessage req;

	req.url.resize(url.length());
	req.url = url;
	req.readIn(req_str);

	switch (hash_(url.c_str()))
	{
	case "/a"_hash:
		SendHttpRsp(connection, "AAAAA");
		break;
	case "/api/hello"_hash:
		SendHttpRsp(connection, "welcome to httpserver");
		break;
	case "/api/sum"_hash:
		// 简单post请求，加法运算测试
		char n1[100], n2[100];
		double result;
		/* Get form variables */
		mg_get_http_var(&http_req->body, "n1", n1, sizeof(n1));
		mg_get_http_var(&http_req->body, "n2", n2, sizeof(n2));
		/* Compute the result and send it back as a JSOt object */
		result = strtod(n1, NULL) + strtod(n2, NULL);
		SendHttpRsp(connection, std::to_string(result));
		break;
	case "/server.html"_hash:
		mg_serve_http(connection, http_req, s_server_option);
		break;
	default:
		mg_serve_http(connection, http_req, s_server_option);
	}

	//mg_printf(
		//connection,
		//:"%s",
		//"HTTP/1.1 501 Not Implemented\r\n"
		//"Content-Length: 0\r\n\r\n");
}


// ---- websocket ---- //
int HttpServer::isWebsocket(const mg_connection* connection)
{
	return connection->flags & MG_F_IS_WEBSOCKET;
}

void HttpServer::HandleWebsocketMessage(mg_connection* connection, int event_type, websocket_message* ws_msg)
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
			(char*)ws_msg->data, ws_msg->size
		};

		char buff[1024] = { 0 };
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

void HttpServer::SendWebsocketMsg(mg_connection* connection, std::string msg)
{
	mg_send_websocket_frame(connection, WEBSOCKET_OP_TEXT, msg.c_str(), strlen(msg.c_str()));
}

void HttpServer::BroadcastWebsocketMsg(std::string msg)
{
	for (mg_connection* connection : s_websocket_session_set)
		mg_send_websocket_frame(connection, WEBSOCKET_OP_TEXT, msg.c_str(), strlen(msg.c_str()));
}

bool HttpServer::Close()
{
	mg_mgr_free(&m_mgr);
	return true;
}
