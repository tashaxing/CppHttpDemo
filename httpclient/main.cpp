#include <iostream>
#include "http_client.h"

void handle_func(std::string rsp)
{
	// do sth according to rsp
	std::cout << "http rsp1: " << rsp << std::endl;
}

int main()
{
	// 拼完整url，带参数
	std::string url1 = "http://127.0.0.1:7999/api/hello";
	HttpClient::SendReq(url1, handle_func);
	
	std::string url2 = "http://127.0.0.1:7999/api/fun2";
	HttpClient::SendReq(url2, [](std::string rsp) { 
		std::cout << "http rsp2: " << rsp << std::endl; 
	});

	system("pause");

	return 0;
}