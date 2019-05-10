/******************************************************************************
Filename: redisMgr.h
autho:bob
date:2018-12-18
description:定义了WS server相关的redis操作
******************************************************************************/
#ifndef REDISMGR_H_
#define REDISMGR_H_

#include "common.h"
#include "singleton.h"
#include <string>
#include <list>
#include <map>


using namespace std;
using namespace CUST;

class CRedisMgr:public Singleton<CRedisMgr>
{
public:
	CRedisMgr();
	~CRedisMgr();
	bool init(uint32_t num);
	bool getCurrentCustInfoByUser(const string userId,USER_PROCESSING_INFO& info);
	//bool userToCust(const string &userId,const string& custId,SESSION_TYPE type,uint32_t sessionNum); //分配客服给用户
	bool userToCustCurrent(const string userId,const string& custId); //分配用户给客服当前会话
	bool usersToCustCurrent(list<string>&users,const string& custId);
	//bool getCustCurrentUsers(const string& custId,list<SESSION_INFO>& usersInfo);
	bool getCustCurrentUsers(const string custId,list<string>& users);
	bool delUserFromCustCurrent(const string custId,const string& userId,string& queueUser);
	bool userToCustQueue(const string userId,const string& custId); //分配用户给客服排队会话
	bool usersToCustQueue(list<string>& users,const string& custId);
	//bool getCustQueueUsers(const string& custId,list<CUST::SESSION_INFO>& usersInfo);
	bool getCustQueueUsers(const string custId,list<string>& users);
	bool userToCustHistory(const string userId,const string& custId); //分配用户给客服历史会话
	//bool getCustHistoryUsers(const string& custId,list<CUST::SESSION_INFO>& usersInfo);
	
	bool getCustInfo(list<string>& custList,map<string,map<string,string>>& custInfo);
	bool getCustInfo(const string custId,CUST::CUST_INFO& custInfo);
	bool getCustStatus(const string custId,USER_STATUS& status); //获取客服状态
	bool setCustStatus(const string custId,USER_STATUS status);  //设置客服状态
	uint32_t getSessionListNum(){ return m_sessionListNum; };
	bool custSessionIsExist(list<string> keys,list<string>& existKeys);
	bool delCustSession(list<string> keys);
	bool getUserInfo(const string userId,CUST::USER_INFO & userInfo);
	bool insertUserInfo(const string userId,const CUST::USER_INFO & userInfo);
	
	
private:
	uint32_t m_sessionListNum;
	
};

#endif //REDISMGR_H_