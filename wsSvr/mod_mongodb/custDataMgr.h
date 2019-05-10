/******************************************************************************
Filename: custDataMgr.h
Description: 客服消息数据管理类
******************************************************************************/
#ifndef __CUSTDATAMGR_H__
#define __CUSTDATAMGR_H__

#include <bsoncxx/builder/basic/document.hpp>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <list>
#include "mongoDbColl.h"
#include "ostype.h"
#include "custData.h"
#include "common.h"

using namespace std;

#define MONGO_ERROR_CONN_UNVALID  -1

#define CUSTDATA_MONGO_DB_NAME			"custData"
#define CUSTMSG_MONGO_COLL_NAME		    "custMsg"
#define CUSTSESSION_MONGO_COLL_NAME     "custSession"    //按单次处理记录会话，用于统计
#define CUST_WAIT_MONGO_COLL_NAME       "custWait"       //客服排队会话信息
#define CUST_PROCESSED_MONGO_COLL_NAME  "custProcessed"  //客服已处理会话信息
#define SERVICE_WAIT_MONGO_COLL_NAME    "serviceWait"    //服务号排队会话信息


typedef struct{
	string custId;
	string content;
	uint64_t msgTime;
}CUST_LAST_MSG_INFO;

class CCustMsgMgr : public CMongoDbColl
{
public:
	CCustMsgMgr(const string& DbName = CUSTDATA_MONGO_DB_NAME, const string& CollName = CUSTMSG_MONGO_COLL_NAME);
	bool insertCustMsg(const CCustMsg& custMsg);
	bool updateToCustId(const string& fromId,const string& serviceId, const string& msgId,const string& toId);
	bool updateProcessed(const string&fromId,const string& toId,const string &msgId);
	bool getUserToCustLastMsg(const string& userId,const string& custId,const string& serviceId,CUST_LAST_MSG_INFO& lastMsg);
	bool getCustMsgByPage(const string& custId,const string&userId,const string& serviceId,uint32_t pageIndex,uint32_t pageSize);
	

private:
	
		
};

class CCustSessionMgr : public CMongoDbColl
{
public:
	CCustSessionMgr(const string& DbName = CUSTDATA_MONGO_DB_NAME, const string& CollName = CUSTSESSION_MONGO_COLL_NAME);
	bool insertCustSession(const CCustSession& custSession);
	bool updateCustSessionEndTime(const string& sessionId,uint64_t timeStamp);
	
};

class CCustWaitMgr : public CMongoDbColl
{
public:
	CCustWaitMgr(const string& DbName = CUSTDATA_MONGO_DB_NAME, const string& CollName = CUST_WAIT_MONGO_COLL_NAME);
	bool insertCustWait(const CCustWait& custWaitRec);
	bool insertCustWaits(list<CCustWait>& custWaitRecs);
	bool getCustWait(const string& custId,const string& userId,const string& serviceId,CCustWait& custWaitRec);
	bool getCustWaits(const string& custId,list<string>& users,const string& serviceId,list<CCustWait>& custWaitRecs);
	bool getCustWaits(const string& custId,const string& serviceId,uint32_t num,list<CCustWait>& custWaitRecs);
	bool deleteCustWait(const string& custId,const string& userId,const string& serviceId,CCustWait& custWaitRec);
	bool deleteCustWaits(const string& custId,list<string>& users,const string& serviceId);
	
};

class CCustProcessedMgr : public CMongoDbColl
{
public:
	CCustProcessedMgr(const string& DbName = CUSTDATA_MONGO_DB_NAME, const string& CollName = CUST_PROCESSED_MONGO_COLL_NAME);
	bool insertCustProcessed(const CCustProcessed& custProcessedRec);
	bool insertCustProcessed(CCustBase* custProcessedRec);
	bool insertCustProcesseds(list<CCustProcessed>& custProcessedRecs);
	bool getCustProcessed(const string& custId,const string& userId,const string& serviceId,CCustProcessed& custProcessedRec);
	bool getCustProcesseds(const string& custId,list<string>& users,const string& serviceId,list<CCustProcessed>& custProcessedRecs);
	bool getCustProcessedsByPage(const string& custId,const string& serviceId,uint32_t pageInDex,uint32_t pageSize,list<CCustProcessed>& custProcessedRecs);
	//bool deleteCustProcessed(const string& custId,const string& userId,const string& serviceId,CCustProcessed& custProcessedRec);
	
};

class CServiceWaitMgr : public CMongoDbColl
{
public:
	CServiceWaitMgr(const string& DbName = CUSTDATA_MONGO_DB_NAME, const string& CollName = SERVICE_WAIT_MONGO_COLL_NAME);
	bool insertCustServiceWait(const CServiceWait& serviceWaitRec);
	bool getServiceWaits(const string& serviceId,uint32_t num,list<CServiceWait>& serviceWaitRecs);
	bool deleteServiceWait(const string& userId,const string& serviceId,CServiceWait& serviceWaitRec);
	bool deleteServiceWaits(const string& serviceId,list<string>& users);
	
};

#endif // __CUSTDATAMGR_H__