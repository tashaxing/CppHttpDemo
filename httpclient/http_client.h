#pragma once

#include <string>
#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "../common/mongoose.h"
#include "../common/winini.h"

// ����http����callback
typedef void OnRspCallback(mg_connection *c, std::string);
// ����http����handler
using ReqHandler = std::function<bool (std::string, std::string, mg_connection *c, OnRspCallback)>;

class HttpServer
{
public:
	HttpServer() {
		CMyINI* ini = new CMyINI();
		ini->ReadINI("D:/C++/Projects/Network/Server/x64/Debug/regedit.ini");
		s_web_dir = ini->GetValue("dir", "web");}
	~HttpServer() {}
	void Init(const std::string &port); // ��ʼ������
	bool Start(); // ���httpserver
	bool Close(); // �ر�
	void AddHandler(const std::string &url, ReqHandler req_handler); // ע���¼��������
	void RemoveHandler(const std::string &url); // �Ƴ�ʱ�䴦�����
	//static std::string s_web_dir; // ��ҳ��Ŀ¼ 
	
	static mg_serve_http_opts s_server_option; // web������ѡ��
	static std::unordered_map<std::string, ReqHandler> s_handler_map; // �ص�����ӳ���
private:
	// ��̬�¼���Ӧ����
	std::string s_web_dir;

	static void OnHttpWebsocketEvent(mg_connection *connection, int event_type, void *event_data);

	static void HandleHttpEvent(mg_connection *connection, http_message *http_req);
	static void SendHttpRsp(mg_connection *connection, std::string rsp);

	static int isWebsocket(const mg_connection *connection); // �ж��Ƿ���websoket��������
	static void HandleWebsocketMessage(mg_connection *connection, int event_type, websocket_message *ws_msg); 
	static void SendWebsocketMsg(mg_connection *connection, std::string msg); // ������Ϣ��ָ������
	static void BroadcastWebsocketMsg(std::string msg); // ���������ӹ㲥��Ϣ
	static std::unordered_set<mg_connection *> s_websocket_session_set; // ����websocket����

	std::string m_port;    // �˿�
	mg_mgr m_mgr;          // ���ӹ�����
};

