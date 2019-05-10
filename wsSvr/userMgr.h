/******************************************************************************
Filename: userMgr.h
autho:bob
date:2018-12-18
description:维护用户状态以及链路的管理
******************************************************************************/
#ifndef _USERMGR_H__
#define _USERMGR_H__

#include <string>
#include <map>
#include "singleton.h"
#include "wsBase.h"

using namespace std;


typedef map<string,websocketpp::connection_hdl> _UserMap_t;
typedef map<string,websocketpp::connection_hdl> _SessionMap_t;
//typedef map<string,websocketpp::connection_hdl> _IPPortMap_t;

class CUserMgr:public Singleton<CUserMgr>
{
private:
    //CUserMgr(){};
    //virtual ~CUserMgr(){};

public:
	void userIn(const string& userId,const string& session,websocketpp::connection_hdl hdl);
	void userOut(const string& userId);
	void userKickOut(const string& userId);
	void addUserConn(string userId,websocketpp::connection_hdl hdl);
	websocketpp::connection_hdl getUserConn(string userId);
	websocketpp::connection_hdl delUserConn(string userId);
	string findUserByConn(websocketpp::connection_hdl hdl);
	string findUserBySession(const string& session);
	void addSessionConn(string session,websocketpp::connection_hdl hdl);
	websocketpp::connection_hdl getSessionConn(string session);
	void delSessionConn(string session);


private:
	_UserMap_t m_UserConnMap;
	CLock	m_UserConnLock;
	_SessionMap_t m_SessionConnMap;
	CLock	m_SessionConnLock;
	//_IPPortMap_t m_IpPortConnMap;

};

#endif //_USERMGR_H__