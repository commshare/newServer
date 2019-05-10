/******************************************************************************
Filename: custDataMgr.cpp
Description:
******************************************************************************/
#include "custDataMgr.h"
#include "mongoDbManager.h"
#include "mongoTask.h"
#include "util.h"
#include "im_time.h"
#include "common.h"
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/sub_document.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::document;
using namespace std;
using namespace CUST;

CCustMsgMgr::CCustMsgMgr(const string& dbName, const string& collName)
	:CMongoDbColl(dbName, collName)
{

}

bool CCustMsgMgr::insertCustMsg(const CCustMsg& custMsg)
{
	bool bInsertSuccess = CMongoDbColl::InsertOne(custMsg);
	if (!bInsertSuccess)
	{
		WarnLog("insert userId[%s]->serviceId[%s] msgId[%s] failed",
			custMsg.m_sFromId.c_str(),
			custMsg.m_sServiceId.c_str(),
			custMsg.m_sMsgId.c_str());
	}
	return bInsertSuccess;
}

bool CCustMsgMgr::updateToCustId(const string& fromId,const string& serviceId, const string& msgId,const string& toId)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("updateToCustId  get CMongoDBconn failed[msgId:%s fromId:%s toId:%s serviceId:%s]",
				msgId.c_str(),
				fromId.c_str(),
				toId.c_str(),
				serviceId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{},doc{};
		key.append(
			kvp(CUSTMSG_FIELD_FROM_ID, fromId),
			kvp(CUSTMSG_FIELD_MSG_ID, msgId),
			kvp(CUSTMSG_FIELD_SERVICEID, serviceId)
			);
		
	
		doc.append(kvp("$set",
				[toId](bsoncxx::builder::basic::sub_document subdoc) {
					subdoc.append(kvp(CUSTMSG_FIELD_TO_ID, toId));
			}));		
			
		if (pConn->GetCollection(GetDbName(), GetCollName()).update_many(key.view(), doc.view()))
		{
			return true;
		}
		else
		{
			WarnLog("updateToCustId[%s] %s failed", toId.c_str(),bsoncxx::to_json(key.view()).c_str());
		}
	}
	catch (const exception& xcp)
	{
		WarnLog("updateToCustId[%s] msgId[%s] failed, exception catched:%s",toId.c_str(),msgId.c_str(),xcp.what());
	}

	return false;
}

bool CCustMsgMgr::updateProcessed(const string&fromId,const string& toId,const string &msgId)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("updateProcessed  get CMongoDBconn failed[msgId:%s fromId:%s toId:%s",
				msgId.c_str(),
				fromId.c_str(),
				toId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{},doc{};
		key.append(
			kvp(CUSTMSG_FIELD_FROM_ID, fromId),
			kvp(CUSTMSG_FIELD_MSG_ID, msgId),
			kvp(CUSTMSG_FIELD_TO_ID, toId)
			);
		
	
		doc.append(kvp("$set",
				[toId](bsoncxx::builder::basic::sub_document subdoc) {
					subdoc.append(kvp(CUSTMSG_FIELD_PROCESSED, 1));
			}));		
			
		if (pConn->GetCollection(GetDbName(), GetCollName()).update_many(key.view(), doc.view()))
		{
			return true;
		}
		else
		{
			WarnLog("updateProcessed[1] %s failed", bsoncxx::to_json(key.view()).c_str());
		}
	}
	catch (const exception& xcp)
	{
		WarnLog("updateProcessed[1] msgId[%s] failed, exception catched:%s",msgId.c_str(),xcp.what());
	}

	return false;
}



bool CCustMsgMgr::getUserToCustLastMsg(const string& userId,const string& custId,const string& serviceId,CUST_LAST_MSG_INFO& lastMsg)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if( !pConn)
	{
		ErrLog("get MongoDbConn failed!");
		return false;
	}
	
	//using bsoncxx::builder::basic::array;

	//array arrCusts;
	//list<string>::iterator pCust=custs.begin();
	//for (; pCust!=custs.end(); pCust++)
	//{
	//	arrCusts.append(*pCust);
	//}

	//bsoncxx::builder::basic::document subDoc{};
	//subDoc.append(kvp("$in", arrCusts));

	bsoncxx::builder::basic::document fileterDoc{};
	fileterDoc.append(kvp(CUSTMSG_FIELD_FROM_ID, userId));
	fileterDoc.append(kvp(CUSTMSG_FIELD_TO_ID, custId));
	fileterDoc.append(kvp(CUSTMSG_FIELD_SERVICEID, serviceId));

	
	mongocxx::options::find findOption{};
	findOption.limit(1);
	
	//查询条件
	bsoncxx::builder::basic::document sortDoc{};
	sortDoc.append(kvp(CUSTMSG_FIELD_MSG_TIME, -1));
	
	findOption.sort(sortDoc.view());

	try
	{
		mongocxx::stdx::optional<bsoncxx::document::value> valRet = pConn->GetCollection(GetDbName(), GetCollName()).find_one(fileterDoc.view(),findOption);

		if(valRet)
		{
			bsoncxx::document::view view = (*valRet).view();
			lastMsg.custId = view[CUSTMSG_FIELD_TO_ID].get_utf8().value.to_string();
			lastMsg.content = view[CUSTMSG_FIELD_CONTENT].get_utf8().value.to_string();
			lastMsg.msgTime = view[CUSTMSG_FIELD_MSG_TIME].get_int64();
		}
		else
		{
			ErrLog("get user[%s]->cust[%s] last msg failed !",userId.c_str(),custId.c_str());
			return false;
		}
		
	}
	catch (const std::exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
		return false;
	}

	return true;

}


bool CCustMsgMgr::getCustMsgByPage(const string& custId,const string&userId,const string& serviceId,uint32_t pageIndex,uint32_t pageSize)
{


}

CCustSessionMgr::CCustSessionMgr(const string& dbName, const string& collName)
	:CMongoDbColl(dbName, collName)
{

}


bool CCustSessionMgr::insertCustSession(const CCustSession& custSession)
{
	bool bInsertSuccess = CMongoDbColl::InsertOne(custSession);
	if (!bInsertSuccess)
	{
		WarnLog("insertCustSession[%s] custId[%s] userId[%s] serviceId[%s] failed",
			custSession.m_sSessionId.c_str(),
			custSession.m_sCustId.c_str(),
			custSession.m_sUserId.c_str(),
			custSession.m_sServiceId.c_str());
	}
	return bInsertSuccess;
}


bool CCustSessionMgr::updateCustSessionEndTime(const string& sessionId,uint64_t timeStamp)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("get CMongoDBconn failed[sessionId:%s]",sessionId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{},doc{};
		key.append(
			kvp(CUSTSESSION_FIELD_SESSIONID, sessionId)
			);
		
	
		doc.append(kvp("$set",
				[timeStamp](bsoncxx::builder::basic::sub_document subdoc) {
					subdoc.append(kvp(CUSTSESSION_FIELD_ENDTIME, (int64_t)timeStamp));
			}));		
			
		if (pConn->GetCollection(GetDbName(), GetCollName()).update_many(key.view(), doc.view()))
		{
			return true;
		}
		else
		{
			WarnLog("updateCustSessionEndTime failed[%s]",bsoncxx::to_json(key.view()).c_str());
		}
	}
	catch (const exception& xcp)
	{
		WarnLog("updateCustSessionEndTime failed[sessionId:%s], exception catched:%s",sessionId.c_str(),xcp.what());
	}

	return false;

}


CCustWaitMgr::CCustWaitMgr(const string& dbName, const string& collName)
	:CMongoDbColl(dbName, collName)
{

}


bool CCustWaitMgr::insertCustWait(const CCustWait& custWaitRec)
{
	bool bInsertSuccess = CMongoDbColl::InsertOne(custWaitRec);
	if (!bInsertSuccess)
	{
		WarnLog("insertCustWait custId[%s] userId[%s] serviceId[%s] failed",
			custWaitRec.m_sCustId.c_str(),
			custWaitRec.m_sUserId.c_str(),
			custWaitRec.m_sServiceId.c_str());
	}
	return bInsertSuccess;
}

bool CCustWaitMgr::insertCustWaits(list<CCustWait>& custWaitRecs)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("conn invalied");
		return false;
	}

	
	vector<bsoncxx::document::value> documents;
	list<CCustWait>::iterator pCustWait = custWaitRecs.begin();
	
	for (; pCustWait != custWaitRecs.end(); pCustWait++)
	{
		documents.push_back(pCustWait->ToDoc().extract());
	}

	try
	{
		bsoncxx::stdx::optional< mongocxx::result::insert_many > result 
			= pConn->GetCollection(GetDbName(), GetCollName()).insert_many(documents);
	}
	catch (const exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
		return false;
	}
	
	return true;

}

bool CCustWaitMgr::getCustWait(const string& custId,const string& userId,const string& serviceId,CCustWait& custWaitRec)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("getCustWait  get CMongoDBconn failed[custId:%s userId:%s serviceId:%s]",
				custId.c_str(),
				userId.c_str(),
				serviceId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(CUSTWAIT_FIELD_CUSTID, custId),
			kvp(CUSTWAIT_FIELD_USERID, userId),
			kvp(CUSTWAIT_FIELD_SERVICEID, serviceId)
			);

		//mongocxx::options::find_one_and_delete option;

		mongocxx::options::find findOption{};
		findOption.limit(1);
	
		//查询条件
		bsoncxx::builder::basic::document sortDoc{};
		sortDoc.append(kvp(CUSTMSG_FIELD_MSG_TIME, -1));
	
		findOption.sort(sortDoc.view());
		
		mongocxx::stdx::optional<bsoncxx::document::value> valRet = pConn->GetCollection(GetDbName(),GetCollName()).find_one(key.view());

		if(valRet)
		{
			bsoncxx::document::view view = (*valRet).view();
			custWaitRec.m_sCustId = view[CUSTWAIT_FIELD_CUSTID].get_utf8().value.to_string();
			custWaitRec.m_sUserId = view[CUSTWAIT_FIELD_USERID].get_utf8().value.to_string();
			custWaitRec.m_sServiceId = view[CUSTWAIT_FIELD_SERVICEID].get_utf8().value.to_string();
			custWaitRec.m_encrypt = view[CUSTWAIT_FIELD_ENCRYPT].get_int32();
			custWaitRec.m_sLastMsg = view[CUSTWAIT_FIELD_LASTMSG].get_utf8().value.to_string();
			custWaitRec.m_LastMsgTime = view[CUSTWAIT_FIELD_LASTMSG_TIME].get_int64();

			return true;
		}
		else
		{
			ErrLog("getCustWait[custId:%s userId:%s serviceId:%s]",custId.c_str(),userId.c_str(),serviceId.c_str());
			return false;
		}
		
	}
	catch (const exception& xcp)
	{
		WarnLog("getCustWait[custId:%s userId:%s serviceId:%s]",
				custId.c_str(),
				userId.c_str(),
				serviceId.c_str(),
				xcp.what());
	}

	return false;
}

bool CCustWaitMgr::getCustWaits(const string& custId,list<string>& users,const string& serviceId,list<CCustWait>& custWaitRecs)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("getCustWaits  get CMongoDBconn failed[custId:%s serviceId:%s]",
				custId.c_str(),
				serviceId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(CUSTWAIT_FIELD_CUSTID, custId),
			kvp(CUSTWAIT_FIELD_SERVICEID, serviceId)
			);

		using bsoncxx::builder::basic::array;

		array arrUsers;
		list<string>::iterator pUser=users.begin();
		for (; pUser!=users.end(); pUser++)
		{
			arrUsers.append(*pUser);
		}

		bsoncxx::builder::basic::document subDoc{};
		subDoc.append(kvp("$in", arrUsers));
		
		key.append(kvp(CUSTPROCESSED_FIELD_USERID,subDoc));
		
		mongocxx::cursor cursor = pConn->GetCollection(GetDbName(),GetCollName()).find(key.view());
		
		for (auto&& view : cursor)
		{
			CCustWait custWaitRec;
			//AutoSessionRec custWaitRec(new CCustWait);
			custWaitRec.m_sCustId = view[CUSTWAIT_FIELD_CUSTID].get_utf8().value.to_string();
			custWaitRec.m_sUserId = view[CUSTWAIT_FIELD_USERID].get_utf8().value.to_string();
			custWaitRec.m_sServiceId = view[CUSTWAIT_FIELD_SERVICEID].get_utf8().value.to_string();
			custWaitRec.m_encrypt = view[CUSTWAIT_FIELD_ENCRYPT].get_int32();
			custWaitRec.m_sLastMsg = view[CUSTWAIT_FIELD_LASTMSG].get_utf8().value.to_string();
			custWaitRec.m_LastMsgTime = view[CUSTWAIT_FIELD_LASTMSG_TIME].get_int64();

			custWaitRecs.push_back(custWaitRec);

		}

		return true;
		
	}
	catch (const exception& xcp)
	{
		WarnLog("getCustWait[custId:%s serviceId:%s]",
				custId.c_str(),
				serviceId.c_str(),
				xcp.what());
	}

	return false;

}

bool CCustWaitMgr::getCustWaits(const string& custId,const string& serviceId,uint32_t num,list<CCustWait>& custWaitRecs)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("getCustWaits  get CMongoDBconn failed[custId:%s serviceId:%s]",
				custId.c_str(),
				serviceId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(CUSTWAIT_FIELD_CUSTID, custId),
			kvp(CUSTWAIT_FIELD_SERVICEID, serviceId)
			);

		//查询条件
		mongocxx::options::find findOption{};
		findOption.limit(num);
		bsoncxx::builder::basic::document sortDoc{};
		sortDoc.append(kvp(CUSTMSG_FIELD_MSG_TIME, -1));
	
		findOption.sort(sortDoc.view());
		
		mongocxx::cursor cursor = pConn->GetCollection(GetDbName(),GetCollName()).find(key.view());
		
		for (auto&& view : cursor)
		{
			CCustWait custWaitRec;
			//AutoSessionRec custWaitRec(new CCustWait);
			custWaitRec.m_sCustId = view[CUSTWAIT_FIELD_CUSTID].get_utf8().value.to_string();
			custWaitRec.m_sUserId = view[CUSTWAIT_FIELD_USERID].get_utf8().value.to_string();
			custWaitRec.m_sServiceId = view[CUSTWAIT_FIELD_SERVICEID].get_utf8().value.to_string();
			custWaitRec.m_encrypt = view[CUSTWAIT_FIELD_ENCRYPT].get_int32();
			custWaitRec.m_sLastMsg = view[CUSTWAIT_FIELD_LASTMSG].get_utf8().value.to_string();
			custWaitRec.m_LastMsgTime = view[CUSTWAIT_FIELD_LASTMSG_TIME].get_int64();

			custWaitRecs.push_back(custWaitRec);

		}

		return true;
		
	}
	catch (const exception& xcp)
	{
		WarnLog("getCustWait[custId:%s serviceId:%s]",
				custId.c_str(),
				serviceId.c_str(),
				xcp.what());
	}

	return false;

}

bool CCustWaitMgr::deleteCustWait(const string& custId,const string& userId,const string& serviceId,CCustWait& custWaitRec)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("deleteCustWait  get CMongoDBconn failed[custId:%s userId:%s serviceId:%s]",
				custId.c_str(),
				userId.c_str(),
				serviceId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(CUSTWAIT_FIELD_CUSTID, custId),
			kvp(CUSTWAIT_FIELD_USERID, userId),
			kvp(CUSTWAIT_FIELD_SERVICEID, serviceId)
			);

		//mongocxx::options::find_one_and_delete option;
		
		mongocxx::stdx::optional<bsoncxx::document::value> valRet = pConn->GetCollection(GetDbName(),GetCollName()).find_one_and_delete(key.view());

		if(valRet)
		{
			bsoncxx::document::view view = (*valRet).view();
			custWaitRec.m_sCustId = view[CUSTWAIT_FIELD_CUSTID].get_utf8().value.to_string();
			custWaitRec.m_sUserId = view[CUSTWAIT_FIELD_USERID].get_utf8().value.to_string();
			custWaitRec.m_sServiceId = view[CUSTWAIT_FIELD_SERVICEID].get_utf8().value.to_string();
			custWaitRec.m_encrypt = view[CUSTWAIT_FIELD_ENCRYPT].get_int32();
			custWaitRec.m_sLastMsg = view[CUSTWAIT_FIELD_LASTMSG].get_utf8().value.to_string();
			custWaitRec.m_LastMsgTime = view[CUSTWAIT_FIELD_LASTMSG_TIME].get_int64();

			return true;
		}
		else
		{
			ErrLog("deleteCustWait[custId:%s userId:%s serviceId:%s]",custId.c_str(),userId.c_str(),serviceId.c_str());
			return false;
		}
		
	}
	catch (const exception& xcp)
	{
		WarnLog("deleteCustWait[custId:%s userId:%s serviceId:%s]",
				custId.c_str(),
				userId.c_str(),
				serviceId.c_str(),
				xcp.what());
	}

	return false;
}

bool CCustWaitMgr::deleteCustWaits(const string& custId,list<string>& users,const string& serviceId)
{
	if( users.size() < 1)
	{
		DbgLog("no user to delete");
		return true;
	}
	
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("get mongoconn failed!");
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(CUSTWAIT_FIELD_CUSTID, custId),
			kvp(CUSTWAIT_FIELD_SERVICEID, serviceId)
			);

		using bsoncxx::builder::basic::array;

		array arrUsers;
		
		list<string>::iterator pUser=users.begin();
		for (; pUser!=users.end(); pUser++)
		{
			arrUsers.append(*pUser);
		}

		bsoncxx::builder::basic::document subDoc{};
		subDoc.append(kvp("$in", arrUsers));
		
		key.append(kvp(CUSTPROCESSED_FIELD_USERID,subDoc));
		
		if (pConn->GetCollection(GetDbName(), GetCollName()).delete_many(key.view()))
		{
			return true;
		}
		else
		{
			WarnLog("delCustWaits failed");
			return false;
		}
		
		
	}
	catch (const exception& xcp)
	{
		WarnLog("delCustWaits[custId:%s serviceId:%s] exception",
				custId.c_str(),
				serviceId.c_str(),
				xcp.what());
	}

	return false;
	
}

CCustProcessedMgr::CCustProcessedMgr(const string& dbName, const string& collName)
	:CMongoDbColl(dbName, collName)
{

}


bool CCustProcessedMgr::insertCustProcessed(const CCustProcessed& custProcessedRec)
{
	bool bInsertSuccess = CMongoDbColl::InsertOne(custProcessedRec);
	if (!bInsertSuccess)
	{
		WarnLog("custProcessedRec custId[%s] userId[%s] serviceId[%s] failed",
			custProcessedRec.m_sCustId.c_str(),
			custProcessedRec.m_sUserId.c_str(),
			custProcessedRec.m_sServiceId.c_str());
	}
	return bInsertSuccess;
}

bool CCustProcessedMgr::insertCustProcessed(CCustBase* custProcessedRec)
{
	bool bInsertSuccess = CMongoDbColl::InsertOne(*custProcessedRec);
	if (!bInsertSuccess)
	{
		WarnLog("custProcessedRec custId[%s] userId[%s] serviceId[%s] failed",
			custProcessedRec->m_sCustId.c_str(),
			custProcessedRec->m_sUserId.c_str(),
			custProcessedRec->m_sServiceId.c_str());
	}
	return bInsertSuccess;
}


bool CCustProcessedMgr::insertCustProcesseds(list<CCustProcessed>& custProcessedRecs)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("conn invalied");
		return false;
	}

	
	vector<bsoncxx::document::value> documents;
	list<CCustProcessed>::iterator pRec = custProcessedRecs.begin();
	
	for (; pRec != custProcessedRecs.end(); pRec++)
	{
		documents.push_back(pRec->ToDoc().extract());
	}

	try
	{
		bsoncxx::stdx::optional< mongocxx::result::insert_many > result 
			= pConn->GetCollection(GetDbName(), GetCollName()).insert_many(documents);
	}
	catch (const exception& xcp)
	{
		WarnLog("exception catched:%s", xcp.what());
		return false;
	}
	
	return true;

}


bool CCustProcessedMgr::getCustProcessed(const string& custId,const string& userId,const string& serviceId,CCustProcessed& custProcessedRec)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("getCustProcessed  get CMongoDBconn failed[custId:%s userId:%s serviceId:%s]",
				custId.c_str(),
				userId.c_str(),
				serviceId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(CUSTPROCESSED_FIELD_CUSTID, custId),
			kvp(CUSTPROCESSED_FIELD_USERID, userId),
			kvp(CUSTPROCESSED_FIELD_SERVICEID, serviceId)
			);

		//mongocxx::options::find_one_and_delete option;
		
		mongocxx::stdx::optional<bsoncxx::document::value> valRet = pConn->GetCollection(GetDbName(),GetCollName()).find_one(key.view());

		if(valRet)
		{
			bsoncxx::document::view view = (*valRet).view();
			custProcessedRec.m_sCustId = view[CUSTPROCESSED_FIELD_CUSTID].get_utf8().value.to_string();
			custProcessedRec.m_sUserId = view[CUSTPROCESSED_FIELD_USERID].get_utf8().value.to_string();
			custProcessedRec.m_sServiceId = view[CUSTPROCESSED_FIELD_SERVICEID].get_utf8().value.to_string();
			custProcessedRec.m_encrypt = view[CUSTPROCESSED_FIELD_ENCRYPT].get_int32();
			custProcessedRec.m_sLastMsg = view[CUSTPROCESSED_FIELD_LASTMSG].get_utf8().value.to_string();
			custProcessedRec.m_LastMsgTime = view[CUSTPROCESSED_FIELD_LASTMSG_TIME].get_int64();

			return true;
		}
		else
		{
			ErrLog("getCustProcessed[custId:%s userId:%s serviceId:%s]",custId.c_str(),userId.c_str(),serviceId.c_str());
			return false;
		}
		
	}
	catch (const exception& xcp)
	{
		WarnLog("getCustProcessed[custId:%s userId:%s serviceId:%s]",
				custId.c_str(),
				userId.c_str(),
				serviceId.c_str(),
				xcp.what());
	}

	return false;
}


bool CCustProcessedMgr::getCustProcesseds(const string& custId,list<string>& users,const string& serviceId,list<CCustProcessed>& custProcessedRecs)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("getCustProcessedS by Userlist  get CMongoDBconn failed[custId:%s serviceId:%s]",
				custId.c_str(),
				serviceId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(CUSTPROCESSED_FIELD_CUSTID, custId),
			kvp(CUSTPROCESSED_FIELD_SERVICEID, serviceId)
			);

		using bsoncxx::builder::basic::array;

		array arrUsers;
		list<string>::iterator pUser=users.begin();
		for (; pUser!=users.end(); pUser++)
		{
			arrUsers.append(*pUser);
		}

		bsoncxx::builder::basic::document subDoc{};
		subDoc.append(kvp("$in", arrUsers));
		
		key.append(kvp(CUSTPROCESSED_FIELD_USERID,subDoc));

		
		DbgLog("find key[%s]",bsoncxx::to_json(key.view()).c_str());
		
		mongocxx::cursor cursor = pConn->GetCollection(GetDbName(),GetCollName()).find(key.view());

		for (auto&& view : cursor)
		{	
			CCustProcessed custProcessedRec;
			//AutoSessionRec custProcessedRec(new CCustProcessed);
			custProcessedRec.m_sCustId = view[CUSTPROCESSED_FIELD_CUSTID].get_utf8().value.to_string();
			custProcessedRec.m_sUserId = view[CUSTPROCESSED_FIELD_USERID].get_utf8().value.to_string();
			custProcessedRec.m_sServiceId = view[CUSTPROCESSED_FIELD_SERVICEID].get_utf8().value.to_string();
			custProcessedRec.m_encrypt = view[CUSTPROCESSED_FIELD_ENCRYPT].get_int32();
			custProcessedRec.m_sLastMsg = view[CUSTPROCESSED_FIELD_LASTMSG].get_utf8().value.to_string();
			custProcessedRec.m_LastMsgTime = view[CUSTPROCESSED_FIELD_LASTMSG_TIME].get_int64();

			custProcessedRecs.push_back(custProcessedRec);
		}

		return true;
		
	}
	catch (const exception& xcp)
	{
		WarnLog("getCustProcessedS by Userlist [custId:%s serviceId:%s] exception[%s]",
				custId.c_str(),
				serviceId.c_str(),
				xcp.what());
	}

	return false;
	
}

bool CCustProcessedMgr::getCustProcessedsByPage(const string& custId,const string& serviceId,uint32_t pageInDex,uint32_t pageSize,list<CCustProcessed>& custProcessedRecs)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("getCustProcessedsByPage  get CMongoDBconn failed[custId:%s serviceId:%s]",
				custId.c_str(),
				serviceId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(CUSTPROCESSED_FIELD_CUSTID, custId),
			kvp(CUSTPROCESSED_FIELD_SERVICEID, serviceId)
			);

		mongocxx::options::find option;
		option.skip((pageInDex-1)*pageSize);
		option.limit(pageSize+1); //用于判断是否还有下一页

		bsoncxx::builder::basic::document sortDoc{};
		sortDoc.append(kvp(CUSTPROCESSED_FIELD_LASTMSG_TIME, -1));
	
		option.sort(sortDoc.view());
		
		mongocxx::cursor cursor = pConn->GetCollection(GetDbName(),GetCollName()).find(key.view(),option);

		for (auto&& view : cursor)
		{	
			CCustProcessed custProcessedRec;
			//AutoSessionRec custProcessedRec(new CCustProcessed);
			custProcessedRec.m_sCustId = view[CUSTPROCESSED_FIELD_CUSTID].get_utf8().value.to_string();
			custProcessedRec.m_sUserId = view[CUSTPROCESSED_FIELD_USERID].get_utf8().value.to_string();
			custProcessedRec.m_sServiceId = view[CUSTPROCESSED_FIELD_SERVICEID].get_utf8().value.to_string();
			custProcessedRec.m_encrypt = view[CUSTPROCESSED_FIELD_ENCRYPT].get_int32();
			custProcessedRec.m_sLastMsg = view[CUSTPROCESSED_FIELD_LASTMSG].get_utf8().value.to_string();
			custProcessedRec.m_LastMsgTime = view[CUSTPROCESSED_FIELD_LASTMSG_TIME].get_int64();

			custProcessedRecs.push_back(custProcessedRec);
		}

		return true;
		
	}
	catch (const exception& xcp)
	{
		WarnLog("getCustProcessedsByPage[custId:%s serviceId:%s]",
				custId.c_str(),
				serviceId.c_str(),
				xcp.what());
	}

	return false;

}

CServiceWaitMgr::CServiceWaitMgr(const string& dbName, const string& collName)
	:CMongoDbColl(dbName, collName)
{

}


bool CServiceWaitMgr::insertCustServiceWait(const CServiceWait& serviceWaitRec)
{
	bool bInsertSuccess = CMongoDbColl::InsertOne(serviceWaitRec);
	if (!bInsertSuccess)
	{
		WarnLog("insertCustServiceWait userId[%s] serviceId[%s] failed",
			serviceWaitRec.m_sUserId.c_str(),
			serviceWaitRec.m_sServiceId.c_str());
	}
	return bInsertSuccess;
}

bool CServiceWaitMgr::getServiceWaits(const string& serviceId,uint32_t num,list<CServiceWait>& serviceWaitRecs)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("getServiceWaits  get CMongoDBconn failed[serviceId:%s]",
				serviceId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(SERVICEWAIT_FIELD_SERVICEID, serviceId)
			);

		mongocxx::options::find option;
		option.limit(num);

		bsoncxx::builder::basic::document sortDoc{};
		sortDoc.append(kvp(SERVICEWAIT_FIELD_LASTMSG_TIME, -1));
	
		option.sort(sortDoc.view());
		
		mongocxx::cursor cursor = pConn->GetCollection(GetDbName(),GetCollName()).find(key.view(),option);

		for (auto&& view : cursor)
		{	
			CServiceWait serviceWaitRec;
			//AutoSessionRec serviceWaitRec(new CServiceWait);
			serviceWaitRec.m_sUserId = view[SERVICEWAIT_FIELD_USERID].get_utf8().value.to_string();
			serviceWaitRec.m_sServiceId = view[SERVICEWAIT_FIELD_SERVICEID].get_utf8().value.to_string();
			serviceWaitRec.m_encrypt = view[SERVICEWAIT_FIELD_ENCRYPT].get_int32();
			serviceWaitRec.m_sLastMsg = view[SERVICEWAIT_FIELD_LASTMSG].get_utf8().value.to_string();
			serviceWaitRec.m_LastMsgTime = view[SERVICEWAIT_FIELD_LASTMSG_TIME].get_int64();

			serviceWaitRecs.push_back(serviceWaitRec);
		}

		return true;
		
	}
	catch (const exception& xcp)
	{
		WarnLog("getServiceWaits[serviceId:%s]",
				serviceId.c_str(),
				xcp.what());
	}

	return false;
	
}


bool CServiceWaitMgr::deleteServiceWait(const string& userId,const string& serviceId,CServiceWait& serviceWaitRec)
{
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("deleteServiceWait  get CMongoDBconn failed[userId:%s serviceId:%s]",
				userId.c_str(),
				serviceId.c_str());
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(SERVICEWAIT_FIELD_USERID, userId),
			kvp(SERVICEWAIT_FIELD_SERVICEID, serviceId)
			);

		//mongocxx::options::find_one_and_delete option;
		
		mongocxx::stdx::optional<bsoncxx::document::value> valRet = pConn->GetCollection(GetDbName(),GetCollName()).find_one_and_delete(key.view());

		if(valRet)
		{
			bsoncxx::document::view view = (*valRet).view();
			serviceWaitRec.m_sUserId = view[SERVICEWAIT_FIELD_USERID].get_utf8().value.to_string();
			serviceWaitRec.m_sServiceId = view[SERVICEWAIT_FIELD_SERVICEID].get_utf8().value.to_string();
			serviceWaitRec.m_encrypt = view[SERVICEWAIT_FIELD_ENCRYPT].get_int32();
			serviceWaitRec.m_sLastMsg = view[SERVICEWAIT_FIELD_LASTMSG].get_utf8().value.to_string();
			serviceWaitRec.m_LastMsgTime = view[SERVICEWAIT_FIELD_LASTMSG_TIME].get_int64();

			return true;
		}
		else
		{
			ErrLog("deleteServiceWait[userId:%s serviceId:%s]",userId.c_str(),serviceId.c_str());
			return false;
		}
		
	}
	catch (const exception& xcp)
	{
		WarnLog("deleteServiceWait[userId:%s serviceId:%s]",
				userId.c_str(),
				serviceId.c_str(),
				xcp.what());
	}

	return false;
}

bool CServiceWaitMgr::deleteServiceWaits(const string& serviceId,list<string>& users)
{
	if(users.size() < 1) 
	{
		DbgLog("no user to delete");
		return true;
	}
	std::unique_ptr<CMongoDbConn> pConn =
		CMongoDbManager::getInstance()->GetDBConn();

	if (!pConn)
	{
		WarnLog("get mongoconn failed!");
		return false;
	}
	try
	{
		bsoncxx::builder::basic::document key{};
		key.append(
			kvp(CUSTWAIT_FIELD_SERVICEID, serviceId)
			);

		using bsoncxx::builder::basic::array;

		array arrUsers;
		list<string>::iterator pUser=users.begin();
		for (; pUser!=users.end(); pUser++)
		{
			arrUsers.append(*pUser);
		}

		bsoncxx::builder::basic::document subDoc{};
		subDoc.append(kvp("$in", arrUsers));
		
		key.append(kvp(CUSTPROCESSED_FIELD_USERID,subDoc));
		
		if (pConn->GetCollection(GetDbName(), GetCollName()).delete_many(key.view()))
		{
			return true;
		}
		else
		{
			WarnLog("delServiceWaits failed");
			return false;
		}
		
		
	}
	catch (const exception& xcp)
	{
		WarnLog("delServiceWaits[serviceId:%s] exception",
				serviceId.c_str(),
				xcp.what());
	}

	return false;
}


