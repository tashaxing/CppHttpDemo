#include <iostream>
#include <memory>
#include "http_server.h"
#define cookielen 8

// ��ʼ��HttpServer��̬���Ա

mg_serve_http_opts HttpServer::s_server_option;
std::unordered_map<std::string, ReqHandler> HttpServer::s_handler_map;

std::unordered_set<mg_connection *> HttpServer::s_websocket_session_set;

bool handle_fun1(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback)
{
	// do sth
	std::cout << "handle fun1" << std::endl;
	std::cout << "url: " << url << std::endl;
	std::cout << "body: " << body << std::endl;

	rsp_callback(c, "rsp1");

	return true;
}

bool handle_fun2(std::string url, std::string body, mg_connection *c, OnRspCallback rsp_callback)
{
	// do sth
	std::cout << "handle fun2" << std::endl;
	std::cout << "url: " << url << std::endl;
	std::cout << "body: " << body << std::endl;

	rsp_callback(c, "rsp2");

	return true;
}

int main(int argc, char *argv[]) 
{
	CMyINI* ini = new CMyINI();
	ini->ReadINI("D:/C++/Projects/Network/Server/x64/Debug/regedit.ini");
	std::string port = ini->GetValue("web", "port");
	auto http_server = std::shared_ptr<HttpServer>(new HttpServer);
	http_server->Init(port);
	// add handler
	http_server->AddHandler("/api/fun1", handle_fun1);
	http_server->AddHandler("/api/fun2", handle_fun2);

	http_server->Start();
	

	return 0;
}
