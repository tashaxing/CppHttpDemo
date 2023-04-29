CMyINI *p = new CMyINI();
	p->ReadINI("Setting.ini");
	cout << "\n原始INI文件内容：" << std::endl;
	p->Travel();
	p->SetValue("setting", "hehe", "eheh");
	cout << "\n增加节点之后的内容：" << std::endl;
	p->Travel();
	cout << "\n修改节点之后的内容：" << std::endl;
	p->SetValue("kk", "kk", "2");
	p->Travel();
	p->WriteINI("Setting.ini");
//https://blog.csdn.net/m_buddy/article/details/54097131