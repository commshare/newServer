/******************************************************************************
Filename: offlineMsg.cpp
Author:TongHuaizhi 			Version:1.0.0 		Date:2017/06/05
Description:
******************************************************************************/
#include "offlineMsg.h"
#include "offlineMsgMgr.h"
//#include "mongoPool.h"
#include "mongoDbManager.h"
#include "mongoTask.h"
#include "util.h"
#include "im_time.h"
#include "im.pub.pb.h"
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/stdx.hpp>
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::sub_document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::document;
using namespace std;
using namespace im;

#define OFFLINEMSG_TOTAL_COUNT				"totalCount"
#define OFFLINEMSG_CHAT_COUNT				"chatCount"

im::OfflineTotal viewToOfflineMsgTotal(const view& doc)
{
	im::OfflineTotal total;

	if (doc["_id"])
	{
		auto idview = doc["_id"].get_document().view();
		if (idview[OFFLINEMSG_FIELD_FROMID_STR])
		{
			total.set_sfromid(idview[OFFLINEMSG_FIELD_FROMID_STR].get_utf8().value.to_string());
		}
		
		if (idview[OFFLINEMSG_FIELD_TOID_STR])
		{
			total.set_stoid(idview[OFFLINEMSG_FIELD_TOID_STR].get_utf8().value.to_string());
		}
	}

	if (doc[OFFLINEMSG_FIELD_CMDID_STR])
	{
		total.set_cmdid(doc[OFFLINEMSG_FIELD_CMDID_STR].get_int32());
	}

	if (MES_GRPCHAT_DELIVER == total.cmdid() && doc[OFFLINEMSG_FIELD_MSGID_STR])
	{
		total.set_srecentmsgid(doc[OFFLINEMSG_FIELD_MSGID_STR].get_utf8().value.to_string());
	}

	if (doc[OFFLINEMSG_FIELD_CREATETIME_STR])
	{
		total.set_recenttimestamp(doc[OFFLINEMSG_FIELD_CREATETIME_STR].get_int64());
	}
	
	if (doc[OFFLINEMSG_TOTAL_COUNT])
	{
		total.set_unreadtotal(doc[OFFLINEMSG_TOTAL_COUNT].get_int32());
	}

	if (doc[OFFLINEMSG_CHAT_COUNT])
	{
		total.set_unreadchatcount(doc[OFFLINEMSG_CHAT_COUNT].get_int32());
	}

	if (doc[OFFLINEMSG_FIELD_MSGDATA_STR])
	{
		total.set_srecentcontent(doc[OFFLINEMSG_FIELD_MSGDATA_STR].get_utf8().value.to_string());
	}

	DbgLog("%s-%s offline msg count =  %d, chatCount = %d", total.sfromid().c_str(), total.stoid().c_str(),
		total.unreadtotal(), total.unreadchatcount());
	return total;
}


COfflineMsgMgr::COfflineMsgMgr(const string& dbName, const string& collName)
	:CMongoDbColl(dbName, collName)
{

}

bool COfflineMsgMgr::InsertOfflineMsg(const COfflineMsg& offlineMsg)
{
	bool bInsertSuccess = CMongoDbColl::InsertOne(offlineMsg);
	if (!bInsertSuccess)
	{
		WarnLog("insert offlineMsg %x:%s (%s-->%s) failed", offlineMsg.GetCmdId(),
		offlineMsg.GetMsgId().c_str(), offlineMsg.GetFromId().c_str(), offlineMsg.GetToId().c_str());
	}
	return bInsertSuccess;
}

unsigned short COfflineMsgMgr::InsertOfflineMsg(const COfflineMsg& offlineMsg, int)
{

    unsigned short bInsertSuccess = CMongoDbColl::InsertOne(offlineMsg, 0xffff);
    if (!bInsertSuccess)
    {
        WarnLog("insert offlineMsg %x:%s (%s-->%s) failed", offlineMsg.GetCmdId(),
        offlineMsg.GetMsgId().c_str(), offlineMsg.GetFromId().c_str(), offlineMsg.GetToId().c_str());
    }
    return bInsertSuccess;
}

bool COfflineMsgMgr::InsertOfflineMsg(const COfflineMsg& offlineMsg, mongoInsertCallBack callBack, void* para)
{
	DbgLog("add InsertMongotask to threadPool, cmdID = 0x%X, from = %s, to = %s, msgId = %s ",
		offlineMsg.GetCmdId(), offlineMsg.GetFromId().c_str(), offlineMsg.GetToId().c_str(), offlineMsg.GetMsgId().c_str());
	return CMongoDbColl::InsertOne(offlineMsg, callBack, para);
}

bool COfflineMsgMgr::InsertOfflineMsg(const COfflineMsg& offlineMsg, mongoInsertCallBack__ callBack, void* para)
{
    DbgLog("add InsertMongotask to threadPool, cmdID = 0x%X, from = %s, to = %s, msgId = %s ",
        offlineMsg.GetCmdId(), offlineMsg.GetFromId().c_str(), offlineMsg.GetToId().c_str(), offlineMsg.GetMsgId().c_str());
    return CMongoDbColl::InsertOne(offlineMsg, callBack, para);
}

bool COfflineMsgMgr::InsertOfflineMsg(const std::vector<COfflineMsg>& offlineMsgs)
{
	std::vector<std::shared_ptr<IMongoDataEntry> > msgDatas;
	for (unsigned int i = 0; i < offlineMsgs.size(); ++i)
	{
		msgDatas.push_back(std::shared_ptr<IMongoDataEntry>(new COfflineMsg(offlineMsgs[i])));
	}

	bool bInsertSuccess = CMongoDbColl::InsertMany(msgDatas);
	if (!bInsertSuccess)
	{
		WarnLog("insert offlineMsgs failed");
	}
	return bInsertSuccess;
}

bool COfflineMsgMgr::InsertOfflineMsg(const std::vector<COfflineMsg>& offlineMsgs, mongoInsertManyCallBack callBack, void* para)
{
	std::vector<std::shared_ptr<IMongoDataEntry> > msgDatas;
	for (unsigned int i = 0; i < offlineMsgs.size(); ++i)
	{
		msgDatas.push_back(std::shared_ptr<IMongoDataEntry>(new COfflineMsg(offlineMsgs[i])));
	}
	return CMongoDbColl::InsertMany(msgDatas, callBack, para);
}


bool COfflineMsgMgr::DelOfflineMsg(const string& toId, const int cmdId, const string& msgId)
{
	return CMongoDbColl::DelOne(COfflineMsgKeys(toId, cmdId, msgId));
}

bool COfflineMsgMgr::DelOfflineMsg(const string& toId, const int cmdId, const string& msgId, mongoDelCallBack callBack, void* para)
{

	DbgLog("add removeMongotask to threadPool, cmdID = 0x%X, to = %s, msgId = %s ", cmdId, toId.c_str(), msgId.c_str());
	return CMongoDbColl::DelOne(COfflineMsgKeys(toId, cmdId, msgId), callBack, para);
}

bool COfflineMsgMgr::DelOfflineMsg(const string& toId, const int cmdId, const std::vector<string>& msgIds)
{
	return CMongoDbColl::DelMany(COfflineMsgDelSpecificUserManyMsgKeys(toId, cmdId, msgIds));
}

bool COfflineMsgMgr::DelOfflineMsg(const std::vector<string>& toIds, const int cmdId, const string& msgId)
{
	return CMongoDbColl::DelMany(COfflineMsgDelManyUserMsgKeys(toIds, cmdId, msgId));
}

bool COfflineMsgMgr::UpdateOfflineMsg(const string& toId, const int cmdId, const string& msgId, const COfflineMsg& offlineMsg)
{
	return CMongoDbColl::UpdateOne(COfflineMsgKeys(toId, cmdId, msgId), offlineMsg);
}
/* 返回值表示生成任务是否成功*/
bool COfflineMsgMgr::UpdateOfflineMsg(const string& toId, const int cmdId, const string& msgId, const COfflineMsg& offlineMsg, mongoInsertCallBack callBack, void* para)
{
	return CMongoDbColl::UpdateOne(COfflineMsgKeys(toId, cmdId, msgId), offlineMsg,callBack,para);
}

//COfflineMsg COfflineMsgMgr::GetOfflineMsg(const int cmdId, const string& msgId)
//{
//	std::unique_ptr<CMongoDbConn> pConn =
//		CMongoDbManager::getInstance()->GetDBConn();
//
//	if (!pConn) return COfflineMsg();
//	try
//	{
//		//val type is bsoncxx::stdx::optional< bsoncxx::document::value >
//		if (auto val = pConn->GetCollection(GetDbName(), GetCollName()).find_one(document{} << OFFLINEMSG_FIELD_CMDID_STR << cmdId
//																<< OFFLINEMSG_FIELD_MSGID_STR << msgId << finalize))
//		{
//			return viewToOfflineMsg(val->view());
//		}
//		return COfflineMsg();
//	}
//	catch (const std::exception& xcp)
//	{
//		WarnLog("exception catched:%s", xcp.what());
//	}
//	return COfflineMsg();
//}

im::MESOfflineMsgAck COfflineMsgMgr::GetUserOfflineMsg(const string& toId, const string& fromId /*= string("")*/, int maxLen, int limitNum /*= 0*/, int cmdId) throw (int)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();
	if (!pConn) throw MONGO_ERROR_CONN_UNVALID;

	im::MESOfflineMsgAck offlineMsgAck;
    offlineMsgAck.set_stoid(toId);

	bsoncxx::builder::basic::document doc{};
	doc.append(kvp(OFFLINEMSG_FIELD_TOID_STR, toId));
//	if (!fromId.empty())
//	{
//		offlineMsgAck.set_sfromid(fromId);
//		doc.append(kvp(OFFLINEMSG_FIELD_FROMID_STR, fromId));
//	}

//	if (cmdId != 0)
//	{
//		doc.append(kvp(OFFLINEMSG_FIELD_CMDID_STR, cmdId));
//	}

	doc.append(kvp(OFFLINEMSG_FIELD_ISPULLED_STR,
		[](sub_document subdoc) {
		subdoc.append(kvp("$exists", false));
	}));


	mongocxx::options::find findOption{};
	if (limitNum > 0)
	{
		findOption.limit(limitNum);
	}

	//查询条件
	bsoncxx::builder::basic::document sortDoc{};
    sortDoc.append(kvp(OFFLINEMSG_FIELD_FROMID_STR, -1));
    sortDoc.append(kvp(OFFLINEMSG_FIELD_CREATETIME_STR, 1));

	findOption.sort(sortDoc.view());


	try
	{
		mongocxx::cursor cursor = pConn->GetCollection(GetDbName(), GetCollName()).find(doc.view(), findOption);
        im::OfflineMsgData *pMsgData = new im::OfflineMsgData();
        for (auto&& view : cursor)
        {
             const COfflineMsg offlineMsg = viewToOfflineMsg(view);
             pMsgData->set_cmdid(offlineMsg.GetCmdId());
             pMsgData->set_smsgdata(offlineMsg.GetMsgData());

             pMsgData->set_smsgid(offlineMsg.GetMsgId());// Sam
             pMsgData->set_sfromid(offlineMsg.GetFromId());
             pMsgData->set_ucreatetime(offlineMsg.GetCreateTime());
             if (MES_GRPCHAT_DELIVER == pMsgData->cmdid())
             {
                 maxLen -= 200;
             }
			 // 语音/视频 通话要把状态给带出来
             if(SIG_SPONSORP2PCALL_DELIVER == pMsgData->cmdid())
             {
                pMsgData->set_status(offlineMsg.GetMsgStatus());
             }

//             std::cout << bsoncxx::to_json(view) << std::endl;
             if (offlineMsgAck.ByteSize() + pMsgData->ByteSize() >= maxLen - 2000)
             {
             	pMsgData->Clear();
                break;
             }

             im::OfflineMsgData* msgData = offlineMsgAck.add_msglist();
             *msgData = *pMsgData;
             pMsgData->Clear();

             DbgLog("------cmdId:%d, msgdataSize:%d, msgId:%s, fromId:%s, createTime:%ld", offlineMsg.GetCmdId(), offlineMsg.GetMsgData().size(), offlineMsg.GetMsgId().c_str(), offlineMsg.GetFromId().c_str(), offlineMsg.GetCreateTime());
        }
        delete pMsgData;//recycle memory
        offlineMsgAck.set_errcode(NON_ERR);
    }
    catch (const std::exception& xcp)
    {
        WarnLog("exception catched:%s", xcp.what());
        offlineMsgAck.set_errcode(EXCEPT_ERR);
    }

    return offlineMsgAck;
}



//bool COfflineMsgMgr::CreateIndex(const string& indexName, bool unique /*= false*/, bool backGround /*= true*/)
//{
//	std::unique_ptr<CMongoDbColl> pConn =
//		CMongoDbManager::getInstance()->GetDBConn(GetDbName(), GetCollName());
//	if (!pConn) return false;
//
//	return pConn->CreateIndex(indexName, unique, backGround);
//}
//
//bool COfflineMsgMgr::CreateIndex(const std::vector<string>& indexNames)
//{
//	std::unique_ptr<CMongoDbColl> pConn =
//		CMongoDbManager::getInstance()->GetDBConn(GetDbName(), GetCollName());
//	if (!pConn) return false;
//
//	return pConn->CreateIndex(indexNames);
//}

im::MESOfflineSummaryAck COfflineMsgMgr::GetUserOfflineMsgSummaryGrpByFromId(const string & toId, const string& fromId, int maxLen, int cmdId)
{
	im::MESOfflineSummaryAck summaryAck;
	summaryAck.set_suserid(toId);
	summaryAck.set_errcode(NON_ERR);

	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();
	if (!pConn) return summaryAck;

	mongocxx::pipeline pipeLine;
	//match field
	bsoncxx::builder::basic::document matchdoc{};
	matchdoc.append(kvp(OFFLINEMSG_FIELD_TOID_STR, toId));
	if (!fromId.empty())
	{
		matchdoc.append(kvp(OFFLINEMSG_FIELD_FROMID_STR, fromId));
	}
	if (cmdId != 0)
	{
		matchdoc.append(kvp(OFFLINEMSG_FIELD_CMDID_STR, cmdId));
	}
	//matchdoc.append(kvp(OFFLINEMSG_FIELD_ISPULLED_STR, 0));

	//matchdoc.append(kvp(OFFLINEMSG_FIELD_ISPULLED_STR,
	//	[](sub_document subdoc) {
	//	subdoc.append(kvp("$exists", false));
	//}));
	matchdoc.append(kvp(OFFLINEMSG_FIELD_ISPULLED_STR,
		[](sub_document subdoc) {
		subdoc.append(kvp("$exists", false));
	}));

	bsoncxx::builder::basic::document sortdoc{};
	sortdoc.append(kvp(OFFLINEMSG_FIELD_FROMID_STR, 1));
	sortdoc.append(kvp(OFFLINEMSG_FIELD_ISCHATDELIVER_STR, 1));

	pipeLine.match(matchdoc.view()).sort(sortdoc.view());

	//group field
	auto grpDoc = document{}
		<< "_id"
		<< open_document
		<< OFFLINEMSG_FIELD_TOID_STR << "$" OFFLINEMSG_FIELD_TOID_STR
		<< OFFLINEMSG_FIELD_FROMID_STR << "$" OFFLINEMSG_FIELD_FROMID_STR
		<< close_document
		<< OFFLINEMSG_CHAT_COUNT
		<< open_document
		<< "$sum" << "$" OFFLINEMSG_FIELD_ISCHATDELIVER_STR
		<< close_document 
		<< OFFLINEMSG_TOTAL_COUNT
		<< open_document
		<< "$sum" << 1
		<< close_document
		<< OFFLINEMSG_FIELD_CREATETIME_STR
		<< open_document
		<< "$last" << "$" OFFLINEMSG_FIELD_CREATETIME_STR
		<< close_document
		<< OFFLINEMSG_FIELD_CMDID_STR
		<< open_document
		<< "$last" << "$" OFFLINEMSG_FIELD_CMDID_STR
		<< close_document
		<< OFFLINEMSG_FIELD_MSGID_STR
		<< open_document
		<< "$last" << "$" OFFLINEMSG_FIELD_MSGID_STR
		<< close_document
		<< OFFLINEMSG_FIELD_MSGDATA_STR
		<< open_document
		<< "$last" << "$" OFFLINEMSG_FIELD_MSGDATA_STR
		<< close_document
		<< finalize;
	pipeLine.group(grpDoc.view());

	//pipeLine.count("count");
	try
	{
		mongocxx::cursor cursor = pConn->GetCollection(GetDbName(), GetCollName()).aggregate(pipeLine);
		for (auto&& view : cursor)
		{
			im::OfflineTotal* total = summaryAck.add_offlinetotals();
			*total = viewToOfflineMsgTotal(view);
			if (summaryAck.ByteSize() >= maxLen - 2000)
			{
				break;
			}
		}
		DbgLog("summaryAck ls Size = %d", summaryAck.offlinetotals_size());
	}
	catch (const std::exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
	}

	return summaryAck;
}

int COfflineMsgMgr::GetUserOfflineMsgSummaryBytoId(const string &toId)
{
    std::unique_ptr<CMongoDbConn> pConn =
        CMongoDbManager::getInstance()->GetDBConn();
    if (!pConn) return false;

//    mongocxx::pipeline pipeLine;
    bsoncxx::builder::basic::document matchdoc{};
    matchdoc.append(kvp(OFFLINEMSG_FIELD_TOID_STR, toId));

    matchdoc.append(kvp(OFFLINEMSG_FIELD_ISPULLED_STR,
        [](sub_document subdoc) {
        subdoc.append(kvp("$exists", false));
    }));

//    pipeLine.match(matchdoc.view());
//    auto groupDoc = document{}
//            <<"_id" << "null" <<"summary" << open_document << "$sum"
//            <<1<< close_document << finalize;
//    pipeLine.group(groupDoc.view());

    int sum = 0;
    try
    {
//        mongocxx::cursor cursor = pConn->GetCollection(GetDbName(), GetCollName()).aggregate(pipeLine);
        sum = pConn->GetCollection(GetDbName(), GetCollName()).count(bsoncxx::document::view_or_value(matchdoc));
    }
    catch (const std::exception& xcp)
    {
        WarnLog("exception catched:%s", xcp.what());
    }

    return sum;
}

im::MESOfflineSummaryAck COfflineMsgMgr::GetUserOfflineTotal(const string & toId, const string& fromId, int maxLen, int cmdId /*= 0*/)
{
    return GetUserOfflineMsgSummaryGrpByFromId(toId, fromId, maxLen, cmdId);
}

int64_t COfflineMsgMgr::GetUserUnreadPushMsgCount(const string& toId)
{
	using bsoncxx::builder::basic::array;
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();
	if (!pConn) return 0;

	array cmdTypes;
	cmdTypes.append(MES_ADDFRIEND_DELIVER);
	cmdTypes.append(MES_ADDFRIEND_ANS_DELIVER);
	cmdTypes.append(MES_JOINGRP_DELIVER);
	cmdTypes.append(MES_GRPCHAT_DELIVER);
	cmdTypes.append(MES_GRPNOTIFY_DELIVER);
	cmdTypes.append(MES_CHAT_DELIVER);
	

	bsoncxx::builder::basic::document subDoc{};
	subDoc.append(kvp("$in", cmdTypes));

	bsoncxx::builder::basic::document doc{};
	doc.append(kvp(OFFLINEMSG_FIELD_TOID_STR, toId));
	doc.append(kvp(OFFLINEMSG_FIELD_CMDID_STR, subDoc));
	doc.append(kvp(OFFLINEMSG_FIELD_ISPULLED_STR,
		[](sub_document subdoc) {
		subdoc.append(kvp("$exists", false));
	}));

	int64_t count = 0;
	try
	{
		log("%s", bsoncxx::to_json(doc.view()).c_str());
		count = pConn->GetCollection(GetDbName(), GetCollName()).count(doc.view());
	}
	catch (const std::exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
	}
	return count;
}

COfflineMsg* COfflineMsgMgr::GetOneOffineMsg(const string& toId, const int cmdId, const string& msgId)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	COfflineMsg* mongoData = nullptr;
	if (!pConn)
	{
		WarnLog("conn invalied");
		return mongoData;
	}
	
	COfflineMsgKeys key(toId, cmdId, msgId);
	try
	{
		log("get one msg mongo key %s", bsoncxx::to_json(key.ToDoc().view()).c_str());
		mongocxx::stdx::optional<bsoncxx::document::value> valRet = pConn->GetCollection(GetDbName(), GetCollName()).find_one(key.ToDoc().view());
		if(valRet)
		{
			bsoncxx::document::view view = (*valRet).view();
			std::string strMsgId = view[OFFLINEMSG_FIELD_MSGID_STR].get_utf8().value.to_string();
			std::string strFromId = view[OFFLINEMSG_FIELD_FROMID_STR].get_utf8().value.to_string();
			std::string strToId = view[OFFLINEMSG_FIELD_TOID_STR].get_utf8().value.to_string();
			int32_t cmdId = view[OFFLINEMSG_FIELD_CMDID_STR].get_int32();
			std::string strMsgData = view[OFFLINEMSG_FIELD_MSGDATA_STR].get_utf8().value.to_string();
			int64_t createTime = view[OFFLINEMSG_FIELD_CREATETIME_STR].get_int64();
			mongoData = new COfflineMsg(strFromId,strToId, strMsgId, cmdId, strMsgData, createTime);
		}
		else
		{
			WarnLog("update mongo data %s failed", bsoncxx::to_json(key.ToDoc().view()).c_str());
		}
	}
	catch (const exception& xcp)
	{
		WarnLog("get one msg failed, exception catched:%s", xcp.what());
	}

	return mongoData;

}

// 更改 呼叫/视频通话离线消息的状态
bool COfflineMsgMgr::updateCallOfflineMsgStatus(const string& toId, const int cmdId, const string& scallId, int nStatus)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("conn invalied");
		return false;
	}
	try
	{
		static bsoncxx::builder::basic::document doc{};
		static bool bUpdateIndex = false;
		if (!bUpdateIndex)
		{
			doc.append(kvp("$set",
				[nStatus](sub_document subdoc) {
					subdoc.append(kvp(OFFLINEMSG_FIELD_STATUS_STR, nStatus));
			}));
			bUpdateIndex = true;
		}
		CCallOfflineMsgKeys key(toId, cmdId, scallId);
		if (pConn->GetCollection(GetDbName(), GetCollName()).update_many(key.ToDoc().view(), doc.view()))
		{
			return true;
		}
		else
		{
			WarnLog("update call offline msg status %s failed", bsoncxx::to_json(key.ToDoc().view()).c_str());
		}
	}
	catch (const exception& xcp)
	{
		WarnLog("update call offline msg status failed, exception catched:%s", xcp.what());
	}

	return false;
}


