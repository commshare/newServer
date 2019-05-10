/******************************************************************************
Filename: grpOfflineMsgMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/04
Description:
******************************************************************************/
#include "grpOfflineMsg.h"
#include "grpOfflineMsgMgr.h"
#include "mongoDbManager.h"
#include "mongoTask.h"
#include "util.h"
#include "im_time.h"
#include "im.pub.pb.h"
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/document.hpp>
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::document;
using namespace std;
using namespace im;

#define GRPOFFLINEMSG_TOTAL_COUNT				"totalCount"
#define GRPOFFLINEMSG_CHAT_COUNT				"chatCount"




CGrpOfflineMsgMgr::CGrpOfflineMsgMgr(const string& dbName, const string& collName)
	:CMongoDbColl(dbName, collName)
{

}

bool CGrpOfflineMsgMgr::InsertGrpOfflineMsg(const CGrpOfflineMsg& offlineMsg)
{
	bool bInsertSuccess = CMongoDbColl::InsertOne(offlineMsg);
	if (!bInsertSuccess)
	{
		WarnLog("insert grpOfflineMsg %s (%s-->%s) failed",
			offlineMsg.GetMsgId().c_str(), offlineMsg.GetFromId().c_str(), offlineMsg.GetGrpId().c_str());
	}
	return bInsertSuccess;
}

bool CGrpOfflineMsgMgr::InsertGrpOfflineMsg(const CGrpOfflineMsg& offlineMsg, mongoInsertCallBack callBack, void* para)
{
	DbgLog("add InsertMongotask to threadPool, from = %s, to = %s, msgId = %s ",
		offlineMsg.GetFromId().c_str(), offlineMsg.GetGrpId().c_str(), offlineMsg.GetMsgId().c_str());
	return CMongoDbColl::InsertOne(offlineMsg, callBack, para);
}

bool CGrpOfflineMsgMgr::InsertGrpOfflineMsg(const CGrpOfflineMsg& offlineMsg, void* para, mongoInsertCallBack__ callBack)
{
    DbgLog("add InsertMongotask to threadPool, from = %s, to = %s, msgId = %s ",
		offlineMsg.GetFromId().c_str(), offlineMsg.GetGrpId().c_str(), offlineMsg.GetMsgId().c_str());
    return CMongoDbColl::InsertOne(offlineMsg, callBack, para);
}



//bool CGrpOfflineMsgMgr::InsertGrpNoMongoOperTask(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpTask, mongoNoOperCallBack callBack, void* para)
//{
//	return CMongoDbColl::InsertOne(pGrpTask, callBack, para);
//}


bool CGrpOfflineMsgMgr::DelOfflineMsg(const string& grpId, const string& msgId)
{
	return CMongoDbColl::DelOne(CGrpOfflineMsgKeys(grpId, msgId));
}

bool CGrpOfflineMsgMgr::DelOfflineMsg(const string& grpId, const string& msgId, mongoDelCallBack callBack, void* para)
{
	DbgLog("add removeMongotask to threadPool,  grpId = %s, msgId = %s ", grpId.c_str(), msgId.c_str());
	return CMongoDbColl::DelOne(CGrpOfflineMsgKeys(grpId, msgId), callBack, para);
}

CGrpOfflineMsg CGrpOfflineMsgMgr::GetGrpOfflineMsg(const string& grpId, const string& msgId)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn) return CGrpOfflineMsg();
	try
	{
		//val type is bsoncxx::stdx::optional< bsoncxx::document::value >
		if (auto val = pConn->GetCollection(GetDbName(), GetCollName()).find_one(document{}
			<< GRPOFFLINEMSG_FIELD_GRPID_STR << grpId << GRPOFFLINEMSG_FIELD_MSGID_STR << msgId << finalize))
		{
			return viewToGrpOfflineMsg(val->view());
		}
		return CGrpOfflineMsg();
	}
	catch (const std::exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
	}
	return CGrpOfflineMsg();
}

string CGrpOfflineMsgMgr::GetGrpOfflineMsgData(const string& grpId, const string& msgId)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn) return  string("");
	try
	{
		//val type is bsoncxx::stdx::optional< bsoncxx::document::value >
		if (auto val = pConn->GetCollection(GetDbName(), GetCollName()).find_one(document{}
			<< GRPOFFLINEMSG_FIELD_GRPID_STR << grpId << GRPOFFLINEMSG_FIELD_MSGID_STR << msgId << finalize))
		{
			if (val->view()[GRPOFFLINEMSG_FIELD_MSGDATA_STR])
			{
				return val->view()[GRPOFFLINEMSG_FIELD_MSGDATA_STR].get_utf8().value.to_string();
			}
		}
		return string("");
	}
	catch (const std::exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
	}
	return string("");
}

bsoncxx::builder::basic::document getFilterDocFromMsgIds(const string& indexStr, const string& indexVal, const std::vector<string>& msgIds)
{
	using bsoncxx::builder::basic::array;

	array arrMsgIds;
	for (auto&& msgId : msgIds)
	{
		arrMsgIds.append(msgId);
	}

	bsoncxx::builder::basic::document subDoc{};
	subDoc.append(kvp("$in", arrMsgIds));

	bsoncxx::builder::basic::document doc{};
	doc.append(kvp(indexStr, indexVal));
	doc.append(kvp(GRPOFFLINEMSG_FIELD_MSGID_STR, subDoc));

	return doc;
}

bsoncxx::builder::basic::document getFilterDocFromMsgIds(const string& indexStr, const string& indexVal, const std::set<string>& msgIds)
{
	using bsoncxx::builder::basic::array;

	array arrMsgIds;
	for (auto&& msgId : msgIds)
	{
		arrMsgIds.append(msgId);
	}

	bsoncxx::builder::basic::document subDoc{};
	subDoc.append(kvp("$in", arrMsgIds));

	bsoncxx::builder::basic::document doc{};
	doc.append(kvp(indexStr, indexVal));
	doc.append(kvp(GRPOFFLINEMSG_FIELD_MSGID_STR, subDoc));

	return doc;
}


std::map<string, CGrpOfflineMsg> CGrpOfflineMsgMgr::GetGrpOfflineMsg(const string& grpId, const std::vector<string>& msgIds)
{
	std::map<string, CGrpOfflineMsg> result;

	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn) return result;


	auto fileterDoc = getFilterDocFromMsgIds(GRPOFFLINEMSG_FIELD_GRPID_STR, grpId, msgIds);

	try
	{
		mongocxx::cursor cursor = pConn->GetCollection(GetDbName(), GetCollName()).find(fileterDoc.view());
		for (auto&& view : cursor)
		{
			CGrpOfflineMsg grpOfflineMsg = viewToGrpOfflineMsg(view);
			result.insert(std::pair<string, CGrpOfflineMsg>(grpOfflineMsg.GetMsgId(), grpOfflineMsg));
		}
	}
	catch (const std::exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
		//throw - 1;
	}

	return result;
}

std::map<string, string> CGrpOfflineMsgMgr::GetGrpOfflineMsgData(const string& grpId, const std::vector<string>& msgIds)
{
	std::map<string, string> result;

	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn) return result;


	auto fileterDoc = getFilterDocFromMsgIds(GRPOFFLINEMSG_FIELD_GRPID_STR, grpId, msgIds);

	try
	{
		mongocxx::cursor cursor = pConn->GetCollection(GetDbName(), GetCollName()).find(fileterDoc.view());
		for (auto&& doc : cursor)
		{
			string msgId;
			if (doc[GRPOFFLINEMSG_FIELD_MSGID_STR])
			{
				msgId = doc[GRPOFFLINEMSG_FIELD_MSGID_STR].get_utf8().value.to_string();
			}

			string msgData;
			if (doc[GRPOFFLINEMSG_FIELD_MSGDATA_STR])
			{
				msgData = doc[GRPOFFLINEMSG_FIELD_MSGDATA_STR].get_utf8().value.to_string();
			}

			if (!msgId.empty())
			{
				result.insert(std::pair<string, string>(msgId, msgData));
			}

			log("======oringial msg size ,grpId:%s, msgId:%s , msgData len = %d:",grpId.c_str(), msgId.c_str(), msgData.size());
		}
	}
	catch (const std::exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
		//throw - 1;
	}

	return result;
}

std::vector<MsgIdContents> CGrpOfflineMsgMgr::GetGrpOfflineMsgData(const string& grpId, const std::set<string>& msgIds)
{
    std::vector<MsgIdContents> res;

	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn) return res;


	auto fileterDoc = getFilterDocFromMsgIds(GRPOFFLINEMSG_FIELD_GRPID_STR, grpId, msgIds);

	try
	{
		mongocxx::cursor cursor = pConn->GetCollection(GetDbName(), GetCollName()).find(fileterDoc.view());
		for (auto&& doc : cursor)
		{
			string msgId;
			if (doc[GRPOFFLINEMSG_FIELD_MSGID_STR])
			{
				msgId = doc[GRPOFFLINEMSG_FIELD_MSGID_STR].get_utf8().value.to_string();
			}

			string msgData;
			if (doc[GRPOFFLINEMSG_FIELD_MSGDATA_STR])
			{
				msgData = doc[GRPOFFLINEMSG_FIELD_MSGDATA_STR].get_utf8().value.to_string();
			}

            string grpId;
            if (doc[GRPOFFLINEMSG_FIELD_GRPID_STR]) {
                grpId = doc[GRPOFFLINEMSG_FIELD_GRPID_STR].get_utf8().value.to_string();
            }

			if (!msgId.empty())
			{
				res.push_back(MsgIdContents{msgId, msgData});
			}

            log("[Get offlineMsg data]======oringial msg size ,grpId:%s, msgId:%s , msgData len = %d:",grpId.c_str(), msgId.c_str(), msgData.size());
		}
	}
	catch (const std::exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
		//throw - 1;
	}

	return res;
}
