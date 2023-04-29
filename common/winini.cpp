#include "winini.h"
 
#define INIDEBUG
 
CMyINI::CMyINI()
{
}
 
 
CMyINI::~CMyINI()
{
}
 
//************************************************************************
// 函数名称:    	TrimString
// 访问权限:    	public 
// 创建日期:		2017/01/05
// 创 建 人:		
// 函数说明:		去除空格
// 函数参数: 	string & str	输入的字符串
// 返 回 值:   	std::string &	结果字符串
//************************************************************************
string &TrimString(string &str)
{
	string::size_type pos = 0;
	while (str.npos != (pos = str.find(" ")))
		str = str.replace(pos, pos + 1, "");
	return str;
}
 
//************************************************************************
// 函数名称:    	ReadINI
// 访问权限:    	public 
// 创建日期:		2017/01/05
// 创 建 人:		
// 函数说明:		读取INI文件，并将其保存到map结构中
// 函数参数: 	string path	INI文件的路径
// 返 回 值:   	int
//************************************************************************
int CMyINI::ReadINI(string path)
{
	ifstream in_conf_file(path.c_str());
	if (!in_conf_file) return 0;
	string str_line = "";
	string str_root = "";
	vector<ININode> vec_ini;
	while (getline(in_conf_file, str_line))
	{
		string::size_type left_pos = 0;
		string::size_type right_pos = 0;
		string::size_type equal_div_pos = 0;
		string str_key = "";
		string str_value = "";
		if ((str_line.npos != (left_pos = str_line.find("["))) && (str_line.npos != (right_pos = str_line.find("]"))))
		{
			//cout << str_line.substr(left_pos+1, right_pos-1) << endl;
			str_root = str_line.substr(left_pos + 1, right_pos - 1);
		}
 
		if (str_line.npos != (equal_div_pos = str_line.find("=")))
		{
			str_key = str_line.substr(0, equal_div_pos);
			str_value = str_line.substr(equal_div_pos + 1, str_line.size() - 1);
			str_key = TrimString(str_key);
			str_value = TrimString(str_value);
			//cout << str_key << "=" << str_value << endl;
		}
 
		if ((!str_root.empty()) && (!str_key.empty()) && (!str_value.empty()))
		{
			ININode ini_node(str_root, str_key, str_value);
			vec_ini.push_back(ini_node);
			//cout << vec_ini.size() << endl;
		}
	}
	in_conf_file.close();
	in_conf_file.clear();
 
	//vector convert to map
	map<string, string> map_tmp;
	for (vector<ININode>::iterator itr = vec_ini.begin(); itr != vec_ini.end(); ++itr)
	{
		map_tmp.insert(pair<string, string>(itr->root, ""));
	}	//提取出根节点
	for (map<string, string>::iterator itr = map_tmp.begin(); itr != map_tmp.end(); ++itr)
	{
#ifdef INIDEBUG
		//cout << "根节点： " << itr->first << endl;
#endif	//INIDEBUG
		SubNode sn;
		for (vector<ININode>::iterator sub_itr = vec_ini.begin(); sub_itr != vec_ini.end(); ++sub_itr)
		{
			if (sub_itr->root == itr->first)
			{
#ifdef INIDEBUG
				//cout << "键值对： " << sub_itr->key << "=" << sub_itr->value << endl;
#endif	//INIDEBUG
				sn.InsertElement(sub_itr->key, sub_itr->value);
			}
		}
		map_ini.insert(pair<string, SubNode>(itr->first, sn));
	}
	return 1;
}
 
//************************************************************************
// 函数名称:    	GetValue
// 访问权限:    	public 
// 创建日期:		2017/01/05
// 创 建 人:		
// 函数说明:		根据给出的根结点和键值查找配置项的值
// 函数参数: 	string root		配置项的根结点
// 函数参数: 	string key		配置项的键
// 返 回 值:   	std::string		配置项的值
//************************************************************************
string CMyINI::GetValue(string root, string key)
{
	map<string, SubNode>::iterator itr = map_ini.find(root);
	map<string, string>::iterator sub_itr = itr->second.sub_node.find(key);
	if (!(sub_itr->second).empty())
		return sub_itr->second;
	return "";
}
 
//************************************************************************
// 函数名称:    	WriteINI
// 访问权限:    	public 
// 创建日期:		2017/01/05
// 创 建 人:		
// 函数说明:		保存XML的信息到文件中
// 函数参数: 	string path	INI文件的保存路径
// 返 回 值:   	int
//************************************************************************
int CMyINI::WriteINI(string path)
{
	ofstream out_conf_file(path.c_str());
	if (!out_conf_file)
		return -1;
	//cout << map_ini.size() << endl;
	for (map<string, SubNode>::iterator itr = map_ini.begin(); itr != map_ini.end(); ++itr)
	{
		//cout << itr->first << endl;
		out_conf_file << "[" << itr->first << "]" << endl;
		for (map<string, string>::iterator sub_itr = itr->second.sub_node.begin(); sub_itr != itr->second.sub_node.end(); ++sub_itr)
		{
			//cout << sub_itr->first << "=" << sub_itr->second << endl;
			out_conf_file << sub_itr->first << "=" << sub_itr->second << endl;
		}
	}
 
	out_conf_file.close();
	out_conf_file.clear();
 
	return 1;
}
 
 
//************************************************************************
// 函数名称:    	SetValue
// 访问权限:    	public 
// 创建日期:		2017/01/05
// 创 建 人:		
// 函数说明:		设置配置项的值
// 函数参数: 	string root		配置项的根节点
// 函数参数: 	string key		配置项的键
// 函数参数: 	string value	配置项的值
// 返 回 值:   	std::vector<ININode>::size_type	
//************************************************************************
vector<ININode>::size_type CMyINI::SetValue(string root, string key, string value)
{
	map<string, SubNode>::iterator itr = map_ini.find(root);	//查找
	if (map_ini.end() != itr)
	{
		//itr->second.sub_node.insert(pair<string, string>(key, value));
		itr->second.sub_node[key] = value;
	}	//根节点已经存在了，更新值
	else
	{
		SubNode sn;
		sn.InsertElement(key, value);
		map_ini.insert(pair<string, SubNode>(root, sn));
	}	//根节点不存在，添加值
 
	return map_ini.size();
}
 
//************************************************************************
// 函数名称:    	Travel
// 访问权限:    	public 
// 创建日期:		2017/01/05
// 函数说明:		遍历打印INI文件
// 返 回 值:   	void
//************************************************************************
void CMyINI::Travel()
{
	for (map<string, SubNode>::iterator itr = this->map_ini.begin(); itr!= this->map_ini.end(); ++itr)
	{
		//root
		cout << "[" << itr->first << "]" << endl;
		for (map<string, string>::iterator itr1=itr->second.sub_node.begin(); itr1!=itr->second.sub_node.end();
			++itr1)
		{
			cout << "    " << itr1->first << " = " << itr1->second << endl;
		}
	}
	
}

//https://blog.csdn.net/m_buddy/article/details/54097131