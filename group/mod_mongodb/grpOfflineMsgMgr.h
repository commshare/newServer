/******************************************************************************
Filename: grpOfflineMsgMgr.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/04
Description: 数据类，保存离线消息的信息,主要用在mongodb中离线消息的存储和获取
******************************************************************************/
#ifndef __GRPOFFLINEMSGMGR_H__
#define __GRPOFFLINEMSGMGR_H__

#include <bsoncxx/builder/basic/document.hpp>
#include <string>
#include <map>
#include <vector>
#include "mongoDbColl.h"
#include "ostype.h"
#include "grpOfflineMsg.h"

#define MONGO_ERROR_CONN_UNVALID  -1

#define GRPOFFLINEMSG_MONGO_DB_NAME			"msgSvr"
#define GRPOFFLINEMSG_MONGO_COLL_NAME		"offlineGrpMsg"


using std::string;

class CGrpOfflineMsgMgr : public CMongoDbColl
{
public:
	CGrpOfflineMsgMgr(const string& DbName = GRPOFFLINEMSG_MONGO_DB_NAME, const string& CollName = GRPOFFLINEMSG_MONGO_COLL_NAME);
	/* Insert an offlineMsg to mongoSvr:dbName:collection right now*/
	bool InsertGrpOfflineMsg(const CGrpOfflineMsg& offlineMsg);
	/* 返回值表示生成任务是否成功*/
	bool InsertGrpOfflineMsg(const CGrpOfflineMsg& offlineMsg, mongoInsertCallBack callBack, void* para);	//执行后回调

	bool DelOfflineMsg(const string& grpId, const string& msgId);
	bool DelOfflineMsg(const string& grpId, const string& msgId, mongoDelCallBack callBack, void* para);

	//bool InsertGrpNoMongoOperTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, mongoNoOperCallBack callBack, void* para);			//添加非mongo的任务，主要是复用线程

	CGrpOfflineMsg GetGrpOfflineMsg(const string& grpId, const string& msgId);
	string GetGrpOfflineMsgData(const string& grpId, const string& msgId);

	std::map<string, CGrpOfflineMsg> GetGrpOfflineMsg(const string& grpId, const std::vector<string>& msgIds);
	std::map<string, string> GetGrpOfflineMsgData(const string& grpId, const std::vector<string>& msgIds);
};


#endif // __GRPOFFLINEMSGMGR_H__
