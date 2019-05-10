/******************************************************************************
Filename: offlineMsg.h
Author:TongHuaizhi 			Version:1.0.0 		Date:2017/06/05
Description: 数据类，保存离线消息的信息,主要用在mongodb中离线消息的存储和获取
******************************************************************************/
#ifndef __OFFLINEMSGMGR_H__
#define __OFFLINEMSGMGR_H__

#include <bsoncxx/builder/basic/document.hpp>
#include <string>
#include "mongoDbColl.h"
#include "im.mes.pb.h"
#include "ostype.h"
#include "offlineMsg.h"

#define MONGO_ERROR_CONN_UNVALID  -1

#define OFFLINEMSG_MONGO_DB_NAME			"msgSvr"
#define OFFLINEMSG_MONGO_COLL_NAME			"offlineMsg"

using std::string;

class COfflineMsgMgr: public CMongoDbColl
{
public:
	COfflineMsgMgr(const string& DbName = OFFLINEMSG_MONGO_DB_NAME, const string& CollName = OFFLINEMSG_MONGO_COLL_NAME);
	/* Insert an offlineMsg to mongoSvr:dbName:collection right now*/
	bool InsertOfflineMsg(const COfflineMsg& offlineMsg);
    unsigned short InsertOfflineMsg(const COfflineMsg& offlineMsg, int);
	/* 返回值表示生成任务是否成功*/
    bool InsertOfflineMsg(const COfflineMsg& offlineMsg, mongoInsertCallBack callBack, void* para);	//执行后回调
    bool InsertOfflineMsg(const COfflineMsg& offlineMsg, mongoInsertCallBack__ callBack, void* para);	//执行后回调

	/* Insert offlineMsgs to mongoSvr:dbName:collection */
	bool InsertOfflineMsg(const std::vector<COfflineMsg>& offlineMsgs);
	bool InsertOfflineMsg(const std::vector<COfflineMsg>& offlineMsgs, mongoInsertManyCallBack callBack, void* para);	//执行后回调

	//bool InsertOfflineMsg(const std::vector<std::shared_ptr<COfflineMsg> >& offlineMsgs);
	//bool InsertOfflineMsg(const std::vector<std::shared_ptr<COfflineMsg> >& offlineMsgs, mongoInsertManyCallBack callBack, void* para);	//执行后回调

	bool DelOfflineMsg(const string& toId, const int cmdId, const string& msgId);
	bool DelOfflineMsg(const string& toId, const int cmdId, const string& msgId, mongoDelCallBack callBack, void* para);

	bool DelOfflineMsg(const std::vector<string>& toIds, const int cmdId, const string& msgIds);
	bool DelOfflineMsg(const string& toId, const int cmdId, const std::vector<string>& msgIds);


	bool UpdateOfflineMsg(const string& toId, const int cmdId, const string& msgId, const COfflineMsg& offlineMsg);
	/* 返回值表示生成任务是否成功*/
	bool UpdateOfflineMsg(const string& toId, const int cmdId, const string& msgId, const COfflineMsg& offlineMsg, mongoInsertCallBack callBack, void* para);	//执行后回调

	//COfflineMsg GetOfflineMsg(const int cmdId, const string& msgId);

    im::MESOfflineMsgAck GetUserOfflineMsg(const string& toId, const string& fromId = string(""), int maxLen = 65535, int limitNum = 50, int cmdId = 0, int64_t createTime = 0)  throw (int);
    //bool GetUserOfflineMsgSortByCmdID(std::vector<COfflineMsg>& offlineMsgList, string& toId, const string& fromId = string(""),  int limitNum = 0, int cmdId = 0)  throw (int);
    //im::MESOfflineMsgAck GetUserOfflineMsg(const string& toId, const string& fromId = string(""), int maxLen = 65515, int limitNum = 0, int cmdId = 0, std::int64_t createTime = 0)  throw (int);

	int64_t GetUserUnreadPushMsgCount(const string& toId);
	
	// 获取数据库中单条消息
	COfflineMsg* GetOneOffineMsg(const string& toId, const int cmdId, const string& msgId);

	/*获取离线消息汇总，通过发送方id分组*/
	im::MESOfflineSummaryAck GetUserOfflineMsgSummaryGrpByFromId(const string & toId, const string& fromId = "", int maxLen = 65515, int cmdId = 0);
	im::MESOfflineSummaryAck GetUserOfflineTotal(const string & toId, const string& fromId, int maxLen = 65515, int cmdId = 0);
	// 更改 呼叫/视频通话离线消息的状态
    bool updateCallOfflineMsgStatus(const string& toId, const int cmdId, const string&  scallId, int nStatus);
	bool getOfflineMsgPullStatus(const string& toId, const int cmdId, const string& msgId);
    int GetUserOfflineMsgSummaryBytoId(const string &toId, int64_t createTime);
};


#endif // __OFFLINEMSGMGR_H__
