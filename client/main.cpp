/*
 * login_server.cpp
 *
 *  Created on: 2013-6-21
 *      Author:
 */
#include <regex>
#include <iostream>
#include <string>
#include "netlib.h"
#include "ConfigFileReader.h"
#include "version.h"
#include "ipparser.h"
#include "ServConn.h"
#include "Thread.h"
#include "Common.h"
//#include "zookeeper.h"
//#include "IM.Monitor.pb.h"
#include "commandlist.h"
#include "im.pub.pb.h"
#include "userInfoMgr.h"

using namespace im;
using std::cout;
using std::endl;
using std::string;


#define MAX_LINE_LEN	1024
string g_cmd_string[10];
int g_cmd_num;
//CClient* g_pClient = NULL;
void split_cmd(char* buf)
{
	int len = strlen(buf);
	string element;

	g_cmd_num = 0;
	for (int i = 0; i < len; i++) {
		if (buf[i] == ' ' || buf[i] == '\t') {
			if (!element.empty()) {
				g_cmd_string[g_cmd_num++] = element;
				element.clear();
			}
		}
		else {
			element += buf[i];
		}
	}

	// put the last one
	if (!element.empty()) {
		g_cmd_string[g_cmd_num++] = element;
	}
}

void print_help()
{
	printf("Usage:\n");
	printf("login\t\t--to cmSvr \n");
	printf("loginout\t--to cmSvr \n");
//	printf("addbuddy\n");
//	printf("dbalogin\n");
//	printf("dbalogout\n");
	printf("chat\t\t--to msgScr send chat request\n");
//	printf("modusrstatus\n");
//	printf("getusrstatus\n");
	printf("addfriend\t--to msgScr send addfriend request\n");
	printf("register\t-- to allsvr(msgSvr, grpSvr and so on)\n");

	printf("summary\t\t--to msgSvr get offline summary\n");
	printf("total\t\t--to msgSvr get offline total\n");
	printf("offmsg\t\t--to msgSvr get offlineMsgs\n");
	printf("grpchat\t\t--to grpSvr send Msg\n");
	printf("grpnotify\t--to msgSvr send grpNotify\n");
	printf("joingrp\t\t--to msgScr send (invite)join grp\n");
	printf("xkey\t\t--to msgScr send exchange Key request\n");
//	printf("close\n");
	printf("quit\n");
//	printf("reg\n");
//	printf("getservlist\n");
}

void exec_cmd()
{
	if (g_cmd_num == 0) {
		return;
	}

	if (g_cmd_string[0] == "reg")
	{
		if (g_cmd_num == 1)
		{
			//    send_cmd(CID_MONITOR_SERVER_REG_REQ);
		}
		else
		{
			print_help();
		}
	}

	//else if (strcmp(g_cmd_string[0].c_str(), "getservlist") == 0) {
	//        send_cmd(CID_MONITOR_SERVER_LIST_GET_REQ);
	//}
	else if (strcmp(g_cmd_string[0].c_str(), "login") == 0) 
	{
		send_cmd(CM_LOGIN);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "register") == 0) 
	{
		send_cmd(SYSTEM_ASSOCSVR_REGIST);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "loginout") == 0) 
	{
		send_cmd(CM_LOGOUT);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "addbuddy") == 0) 
	{
		send_cmd(MES_ADDFRIEND);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "delfnd") == 0) 
	{
		send_cmd(MES_DELFRIEND);
	}
	else if (strncmp(g_cmd_string[0].c_str(), "chat",strlen("chat")) == 0) 
	{
		unsigned long chatNum = 0;
		sscanf(g_cmd_string[0].c_str(),"%*5s%lu", &chatNum); 
//		printf("chat num = %lu\r\n", chatNum);
		chatNum = chatNum <= 20000 ? chatNum : 20000;
		send_cmd(MES_CHAT,(void*)chatNum);
	}
	else if (strncmp(g_cmd_string[0].c_str(), "custchat",strlen("custchat")) == 0) 
	{
		unsigned long chatNum = 10;
		send_cmd(MES_CUSTOM_CHAT,(void*)chatNum);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "chatcancel") == 0) 
	{
		send_cmd(MES_CHATCANCEL);
	}	 
	else if (strcmp(g_cmd_string[0].c_str(), "summary") == 0) 
	{
		send_cmd(MES_OFFLINESUMMARY);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "total") == 0) 
	{
		send_cmd(MES_OFFLINETOTAL);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "offmsg") == 0) 
	{
		send_cmd(MES_OFFLINEMSG);
	}
	else if (strncmp(g_cmd_string[0].c_str(), "grpchat",strlen("grpchat")) == 0) 
	{
		unsigned long chatNum = 0;
		sscanf(g_cmd_string[0].c_str(),"%*8s%lu", &chatNum); 
		printf("chat num = %lu\r\n", chatNum);
		chatNum = chatNum <= 20000 ? chatNum : 20000;
		send_cmd(GROUP_CHAT,(void*)chatNum);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "grpchatcancel") == 0) 
	{
		send_cmd(GROUP_CHATCANCEL);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "grpnotify") == 0) 
	{
		send_cmd(MES_GRPINTERNOTIFY);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "joingrp") == 0) 
	{
		send_cmd(MES_JOINGRP);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "xkey") == 0) 
	{
		send_cmd(MES_EXCHANGE_KEY);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "grpcreate") == 0) {
            send_cmd(GROUP_CREATE);
    }
	else if (strcmp(g_cmd_string[0].c_str(), "grpapply") == 0) {
			send_cmd(GROUP_APPLY);
	}		
    else if (strcmp(g_cmd_string[0].c_str(), "grpinvite") == 0) {
            send_cmd(GROUP_INVITE); 
    }
	else if (strcmp(g_cmd_string[0].c_str(), "grpquit") == 0) {
			send_cmd(GROUP_QUIT);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "grpmodify") == 0) {
			send_cmd(GROUP_MODIFY);
    } 
	else if (strcmp(g_cmd_string[0].c_str(), "call") == 0) {
			send_cmd(SIG_SPONSORP2PCALL);
    } 
	else if (strcmp(g_cmd_string[0].c_str(), "callans") == 0) {
			send_cmd(SIG_SPONSORP2PCALL_ANS);
    } 
	else if (strcmp(g_cmd_string[0].c_str(), "xnat") == 0) {
			send_cmd(SIG_P2PCALL_EXCHANGE_NATINFO);
    } 

	
	//else if (strcmp(g_cmd_string[0].c_str(), "dbalogin") == 0) {
	//        send_cmd(CID_DBA_LOGIN);
	//}   
	//else if (strcmp(g_cmd_string[0].c_str(), "dbalogout") == 0) {
	//        send_cmd(CID_MESSAGE_LOGIN_OUT_REQ);
	//}   
	//else if (strcmp(g_cmd_string[0].c_str(), "modusrstatus") == 0) {
	//        send_cmd(CID_DBA_MOD_USER_STATUS);
	//}   
	//else if (strcmp(g_cmd_string[0].c_str(), "getusrstatus") == 0) {
	//        send_cmd(CID_DBA_GET_USER_STATUS);
	//}  
	else if (strcmp(g_cmd_string[0].c_str(), "quit") == 0) 
	{
		exit(0);
	}
	else if (strcmp(g_cmd_string[0].c_str(), "list") == 0)
	{
		printf("+---------------------+\n");
		printf("|        用户名        |\n");
		printf("+---------------------+\n");
	}
	else if (!strcmp(g_cmd_string[0].c_str(), "q") || !strcmp(g_cmd_string[0].c_str(), "quit") || !strcmp(g_cmd_string[0].c_str(), "exit"))
	{
//		printf("\n");
		exit(0);
	}
	else {
		print_help();
	}
}


class CmdThread : public CThread
{
public:
	void OnThreadRun()
	{
		while (true)
		{
			fprintf(stderr, "%s", PROMPT);	// print to error will not buffer the printed message

			if (fgets(m_buf, MAX_LINE_LEN - 1, stdin) == NULL)
			{
				fprintf(stderr, "fgets failed: %d\n", errno);
				continue;
			}

			m_buf[strlen(m_buf) - 1] = '\0';	// remove newline character

			split_cmd(m_buf);

			exec_cmd();
		}
	}
private:
	char	m_buf[MAX_LINE_LEN];
};

CmdThread g_cmd_thread;

IpParser* pIpParser = NULL;
string strMsfsUrl;
string strDiscovery;//发现获取地址

string GetConfigFileName(const string& processName)
{
	static string configFileName("client.conf");
	if (!processName.empty())
	{
		int lastIndex = processName.find_last_of('/');
		if (lastIndex != -1)
		{
			configFileName = processName.substr(lastIndex + 1) + ".conf";
		}
	}
	return configFileName;
}

void testReg()
{
	regex reg1("\\w+day");
    string s1 = "saturday";
    string s2 = "saturday and sunday";
    smatch r1;
    smatch r2;
//    cout << boolalpha << regex_match(s1, r1, reg1) << std::endl;　　//true
//    cout << boolalpha << regex_match(s2, r2, reg1) << std::endl;　　//false
//    std::cout << "s1匹配结果：" << r1.str() << std::endl;　　　　　　　　　　//saturday
//    cout << "s2匹配结果：" << r2.str() << std::endl;　　　　　　　　　　//空
    cout << endl;
}


int main(int argc, char* argv[])
{
//	testReg();	

	if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
		printf("Server Version: EchoServer/%s\n", VERSION);
		printf("Server Build: %s %s\n", __DATE__, __TIME__);
		return 0;
	}

	srand(time(NULL));
	init_cmd_map();
	g_cmd_thread.StartThread();

	UserInfoMgr::setFileName("loginData.txt");

	signal(SIGPIPE, SIG_IGN);

	CConfigFileReader config_file(GetConfigFileName(argv[0]).c_str()/*"client.conf"*/);

	char* ip_addr1 = config_file.GetConfigName("IpAddr1");	// 电信IP
	char* ip_addr2 = config_file.GetConfigName("IpAddr2");	// 网通IP
	char* str_max_conn_cnt = config_file.GetConfigName("MaxConnCnt");

	// 没有IP2，就用第一个IP
	if (!ip_addr2) 
	{
		ip_addr2 = ip_addr1;
	}

	uint32_t max_conn_cnt = atoi(str_max_conn_cnt);


	uint32_t server_count = 0;
	serv_info_t* server_list = read_server_config(&config_file, "ServerIP", "ServerPort", server_count);

	pIpParser = new IpParser();

	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;

	init_serv_conn(server_list, server_count, ip_addr1, ip_addr2, 1800, max_conn_cnt);

	printf("now enter the event loop...\n");

	writePid();

	netlib_eventloop();

	return 0;
}
