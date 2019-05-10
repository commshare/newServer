#include "userMgr.h"
#include "redisMgr.h"
#include "configMgr.h"
#include "custWebProxy.h"
#include <json/json.h>


void CUserMgr::userIn(const string& userId,const string& session,websocketpp::connection_hdl hdl)
{
	DbgLog("user[%s] hdl[%p] session[%s] in sucess",userId.c_str(),hdl.lock().get(),session.c_str());
	//设置用户状态
	CRedisMgr::getInstance()->setCustStatus(userId,CUST::STATUS_ONLINE);

	addUserConn(userId,hdl);
	addSessionConn(session,hdl);
	
}

void CUserMgr::userOut(const string& userId)
{
	DbgLog("user[%s] out",userId.c_str());
	//设置用户状态
	CRedisMgr::getInstance()->setCustStatus(userId,CUST::STATUS_QUIT);

	const websocketpp::connection_hdl hdl = getUserConn(userId);
	delUserConn(userId);
	
	CAutoLock userConn(&m_SessionConnLock);
	_SessionMap_t::iterator it = m_SessionConnMap.begin();
    for (;it != m_SessionConnMap.end(); it++)
    {
        if(hdl.lock().get() == it->second.lock().get())
        {
        	m_SessionConnMap.erase(it);
			break;
        }
    }
	
}

void CUserMgr::userKickOut(const string& userId)
{
	DbgLog("user[%s] kickout check", userId.c_str());
	//获取用户状态，
	CUST::CUST_INFO custInfo;
	if( true == CRedisMgr::getInstance()->getCustInfo(userId,custInfo) )
	{
		if( CUST::STATUS_QUIT  != custInfo.custStatus )
		{
			//kick out
			websocketpp::connection_hdl hdl = delUserConn(userId);
			DbgLog("user[%s] kickouted hdl[%p]", userId.c_str(),hdl.lock().get());

			string custId,serviceId;
			CUST::unpackCustBind(userId, custId,serviceId);
			Json::Value kickot;
			kickot[CUST_JSON_FIELD_COMMAND] = CUST::MES_KICK_OUT;
			kickot[CUST_JSON_FIELD_USERID] = custId;
			kickot[CUST_JSON_FIELD_SERVICEID] = serviceId;

			CCustWebProxy::getInstance()->send(hdl,kickot.toStyledString());
			CCustWebProxy::getInstance()->close(hdl,"kickOut");
		}
	}
	else
	{
		DbgLog("user[%s] no kickout ", userId.c_str());
	}
	
}

void CUserMgr::addUserConn(string userId,websocketpp::connection_hdl hdl)
{
	CAutoLock userConn(&m_UserConnLock);
	//m_UserConnMap.insert(make_pair(userId, hdl));
	m_UserConnMap[userId] = hdl;
}

websocketpp::connection_hdl CUserMgr::getUserConn(string userId)
{	
	websocketpp::connection_hdl hdl;
	
	CAutoLock userConn(&m_UserConnLock);
	_UserMap_t::iterator it = m_UserConnMap.find(userId);
    if (it != m_UserConnMap.end()) {
        hdl = it->second;
    }
	
    return hdl;
}

websocketpp::connection_hdl CUserMgr::delUserConn(string userId)
{	
	websocketpp::connection_hdl hdl;

	CAutoLock userConn(&m_UserConnLock);
	_UserMap_t::iterator itUser = m_UserConnMap.find(userId);
    if (itUser != m_UserConnMap.end()) {
        hdl = itUser->second;
    }
	
	m_UserConnMap.erase(userId);

	//DEL SessionConn
	_SessionMap_t::iterator itSession = m_SessionConnMap.begin();
    for(;itSession != m_SessionConnMap.end();itSession++) 
	{
        if( itSession->second.lock().get() == hdl.lock().get() )
        {
        	m_SessionConnMap.erase(itSession);
			break;
        }
    }
	
	return hdl;
}


string CUserMgr::findUserByConn(websocketpp::connection_hdl hdl)
{
	string userId;
	CAutoLock userConn(&m_UserConnLock);
	_UserMap_t::iterator it = m_UserConnMap.begin();
    for ( ;it != m_UserConnMap.end(); it++ ) 
	{
        if(hdl.lock().get() == it->second.lock().get())
        {
			userId = it->first;
			break;
		}
    }

	return userId;
}


string CUserMgr::findUserBySession(const string& session)
{

	string userId;
	websocketpp::connection_hdl hdl;
	
	CAutoLock userConn(&m_SessionConnLock);
	
	_SessionMap_t::iterator it = m_SessionConnMap.find(session);
    if (it != m_SessionConnMap.end())
	{
        hdl = it->second;
		
		CAutoLock userConn(&m_UserConnLock);
		_UserMap_t::iterator it1 = m_UserConnMap.begin();
    	for ( ;it1!= m_UserConnMap.end(); it1++ ) 
		{
        	if(hdl.lock().get() == it1->second.lock().get())
        	{
				userId = it1->first;
			}
    	}
    }
	
	return userId;
	
}

void CUserMgr::addSessionConn(string session,websocketpp::connection_hdl hdl)
{
	CAutoLock userConn(&m_SessionConnLock);
	//m_SessionConnMap.insert(make_pair(session, hdl));
	m_SessionConnMap[session] = hdl;
}

websocketpp::connection_hdl CUserMgr::getSessionConn(string session)
{	
	websocketpp::connection_hdl hdl;
	
	CAutoLock userConn(&m_SessionConnLock);
	_SessionMap_t::iterator it = m_SessionConnMap.find(session);
    if (it != m_SessionConnMap.end()) {
        hdl = it->second;
    }
	
    return hdl;
}

void CUserMgr::delSessionConn(string session)
{
	CAutoLock userConn(&m_SessionConnLock);
	m_SessionConnMap.erase(session);
}


