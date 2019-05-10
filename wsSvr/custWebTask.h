/****************************************
author:bob
date:2018-12-12
description:customer websocket 处理类
*****************************************/

#ifndef _CUSTWEBTASK_H_
#define _CUSTWEBTASK_H_

#include "task.h"
#include "wsBase.h"
#include <json/json.h>
#include "custDataMgr.h"

typedef struct{
	wsServer* s; 
	websocketpp::connection_hdl hdl;
	Json::Value msg;
	websocketpp::frame::opcode::value opcode;
}msg_context;


class  CCustWebTask : public CTask
{
public:
	CCustWebTask(msg_context& msg);
	virtual ~CCustWebTask(){};
	virtual void run() override;

private:
	void loginHandler();
	void heartBeatHandler();
	void sessionCloseHandler();
	void updateCustStatus();
	void sessionTransferHandler();
	void sessionPullHandler();
	void sessionActiveHander();
	bool send(const string jsonMsg);
	bool sendToCust(const string custId,const string jsonMsg);
	
	bool getCustCurrentSessions(const string& custId,const string& serviceId,list<CUST::SESSION_INFO>& usersInfo);
	bool dispatchCurrentSessions(const string& custId,const string& serviceId,uint32_t num,list<CUST::SESSION_INFO>& usersInfo);
	bool getCustQueueSessions(const string& custId,const string& serviceId,list<CUST::SESSION_INFO>& usersInfo);
	bool dispatchQueueSessions(const string& custId,const string& serviceId,uint32_t num,list<CUST::SESSION_INFO>& usersInfo);
	void sessionClose(const string& custId,const string& serviceId,const string& userId);
	bool getUserInfo(const string& userId,CUST::USER_INFO& userInfo);
private:
	msg_context m_msg;
	CCustMsgMgr m_custMsgMgr;
	CCustProcessedMgr m_custProcessedMgr;
	CCustWaitMgr m_custWaitMgr;
	CServiceWaitMgr m_serviceWaitMgr;
};

#endif //_CUSTWEBTASK_H_