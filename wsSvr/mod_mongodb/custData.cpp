/******************************************************************************
Filename: custData.cpp
Description:
******************************************************************************/
#include "custData.h"
#include "mongoTask.h"
#include "im_time.h"
#include "util.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using namespace std;



bsoncxx::builder::basic::document CCustMsg::KeyDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(CUSTMSG_FIELD_FROM_ID, m_sFromId),
		kvp(CUSTMSG_FIELD_MSG_ID, m_sMsgId)
		);
	return doc;
}

bsoncxx::builder::basic::document CCustMsg::ToDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(CUSTMSG_FIELD_CMD, (int32_t)m_command),
		kvp(CUSTMSG_FIELD_FROM_ID, m_sFromId),
		kvp(CUSTMSG_FIELD_TO_ID, m_sToId),
		kvp(CUSTMSG_FIELD_SERVICEID, m_sServiceId),
		kvp(CUSTMSG_FIELD_MSG_ID, m_sMsgId),
		kvp(CUSTMSG_FIELD_ENCRYPT, (int32_t)m_encrypt),
		kvp(CUSTMSG_FIELD_CONTENT, m_sContent),
		kvp(CUSTMSG_FIELD_MSG_TIME, (int64_t)m_msgTime),
		kvp(CUSTMSG_FIELD_CREATE_TIME, (int64_t)m_createTime),
		kvp(CUSTMSG_FIELD_UPDATE_TIME, (int64_t)m_updateTime),
		kvp(CUSTMSG_FIELD_PROCESSED, m_processed)
		);
	return doc;
}

bool CCustMsg::IsValid() const
{
	return !m_sMsgId.empty() && !m_sFromId.empty();
}
 
std::shared_ptr<IMongoDataEntry> CCustMsg::Clone()const
{
	return std::shared_ptr<IMongoDataEntry>(new CCustMsg(*this));
}

unsigned int CCustMsg::hashVal() const
{
	return BKDRHash(m_sFromId.c_str());
}

#if 0
CCustMsg viewToUserBehavior(const view& doc)
{
	string fromId;
	if (doc[UserBehavior_FIELD_FROMID_STR])
	{
		fromId = doc[UserBehavior_FIELD_FROMID_STR].get_utf8().value.to_string();
	}

	string grpId;
	if (doc[UserBehavior_FIELD_GRPID_STR])
	{
		grpId = doc[UserBehavior_FIELD_GRPID_STR].get_utf8().value.to_string();
	}

	string msgId;
	if (doc[UserBehavior_FIELD_MSGID_STR])
	{
		msgId = doc[UserBehavior_FIELD_MSGID_STR].get_utf8().value.to_string();
	}

	int createTime = 0;
	if (doc[UserBehavior_FIELD_CREATETIME_STR])
	{
		createTime = doc[UserBehavior_FIELD_CREATETIME_STR].get_int64();
	}

	string msgData;
	if (doc[UserBehavior_FIELD_MSGDATA_STR])
	{
		msgData = doc[UserBehavior_FIELD_MSGDATA_STR].get_utf8().value.to_string();
	}

	return CCustMsg(grpId, msgId, msgData, createTime, fromId);
}
#endif


bsoncxx::builder::basic::document CCustSession::KeyDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(CUSTSESSION_FIELD_SESSIONID, m_sSessionId)
		);
	return doc;
}

bsoncxx::builder::basic::document CCustSession::ToDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(CUSTSESSION_FIELD_CUSTID, m_sCustId),
		kvp(CUSTSESSION_FIELD_USERID, m_sUserId),
		kvp(CUSTSESSION_FIELD_SERVICEID, m_sServiceId),
		kvp(CUSTSESSION_FIELD_SESSIONID, m_sSessionId),
		kvp(CUSTSESSION_FIELD_BEGINTIME, (int64_t)m_beginTime),
		kvp(CUSTSESSION_FIELD_ENDTIME, (int64_t)m_endTime)
		);
	return doc;
}

bool CCustSession::IsValid() const
{
	return !m_sCustId.empty()  && !m_sCustId.empty() && !m_sServiceId.empty() && !m_sSessionId.empty();
}

std::shared_ptr<IMongoDataEntry> CCustSession::Clone()const
{
	return std::shared_ptr<IMongoDataEntry>(new CCustSession(*this));
}

unsigned int CCustSession::hashVal() const
{
	return BKDRHash(m_sSessionId.c_str());
}


CCustWait::CCustWait(CCustProcessed sessionRec)
{
	m_encrypt = sessionRec.m_encrypt;
	m_LastMsgTime = sessionRec.m_LastMsgTime;
	m_sCustId = sessionRec.m_sCustId;
	m_sLastMsg = sessionRec.m_sLastMsg;
	m_sServiceId = sessionRec.m_sServiceId;
	m_sUserId = sessionRec.m_sUserId;
}

CCustWait::CCustWait(CServiceWait sessionRec)
{
	m_encrypt = sessionRec.m_encrypt;
	m_LastMsgTime = sessionRec.m_LastMsgTime;
	m_sCustId = sessionRec.m_sCustId;
	m_sLastMsg = sessionRec.m_sLastMsg;
	m_sServiceId = sessionRec.m_sServiceId;
	m_sUserId = sessionRec.m_sUserId;
}


bsoncxx::builder::basic::document CCustWait::KeyDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(CUSTWAIT_FIELD_CUSTID, m_sCustId),
		kvp(CUSTWAIT_FIELD_USERID, m_sUserId),
		kvp(CUSTWAIT_FIELD_SERVICEID, m_sServiceId)
		);
	return doc;
}

bsoncxx::builder::basic::document CCustWait::ToDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(CUSTWAIT_FIELD_CUSTID, m_sCustId),
		kvp(CUSTWAIT_FIELD_USERID, m_sUserId),
		kvp(CUSTWAIT_FIELD_SERVICEID, m_sServiceId),
		kvp(CUSTWAIT_FIELD_ENCRYPT, (int32_t)m_encrypt),
		kvp(CUSTWAIT_FIELD_LASTMSG, m_sLastMsg),
		kvp(CUSTWAIT_FIELD_LASTMSG_TIME, (int64_t)m_LastMsgTime)
		);
	return doc;
}

bool CCustWait::IsValid() const
{
	return !m_sCustId.empty()  && !m_sUserId.empty() && !m_sServiceId.empty();
}

std::shared_ptr<IMongoDataEntry> CCustWait::Clone()const
{
	return std::shared_ptr<IMongoDataEntry>(new CCustWait(*this));
}

unsigned int CCustWait::hashVal() const
{
	return BKDRHash(m_sServiceId.c_str());
}


CCustProcessed::CCustProcessed(CCustWait sessionRec)
{
	m_encrypt = sessionRec.m_encrypt;
	m_LastMsgTime = sessionRec.m_LastMsgTime;
	m_sCustId = sessionRec.m_sCustId;
	m_sLastMsg = sessionRec.m_sLastMsg;
	m_sServiceId = sessionRec.m_sServiceId;
	m_sUserId = sessionRec.m_sUserId;

}

CCustProcessed::CCustProcessed(CServiceWait sessionRec)
{
	m_encrypt = sessionRec.m_encrypt;
	m_LastMsgTime = sessionRec.m_LastMsgTime;
	m_sCustId = sessionRec.m_sCustId;
	m_sLastMsg = sessionRec.m_sLastMsg;
	m_sServiceId = sessionRec.m_sServiceId;
	m_sUserId = sessionRec.m_sUserId;
}


bsoncxx::builder::basic::document CCustProcessed::KeyDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(CUSTWAIT_FIELD_CUSTID, m_sCustId),
		kvp(CUSTWAIT_FIELD_USERID, m_sUserId),
		kvp(CUSTWAIT_FIELD_SERVICEID, m_sServiceId)
		);
	return doc;
}

bsoncxx::builder::basic::document CCustProcessed::ToDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(CUSTWAIT_FIELD_CUSTID, m_sCustId),
		kvp(CUSTWAIT_FIELD_USERID, m_sUserId),
		kvp(CUSTWAIT_FIELD_SERVICEID, m_sServiceId),
		kvp(CUSTWAIT_FIELD_ENCRYPT, (int32_t)m_encrypt),
		kvp(CUSTWAIT_FIELD_LASTMSG, m_sLastMsg),
		kvp(CUSTWAIT_FIELD_LASTMSG_TIME, (int64_t)m_LastMsgTime)
		);
	return doc;
}

bool CCustProcessed::IsValid() const
{
	return !m_sCustId.empty()  && !m_sUserId.empty() && !m_sServiceId.empty();
}

std::shared_ptr<IMongoDataEntry> CCustProcessed::Clone()const
{
	return std::shared_ptr<IMongoDataEntry>(new CCustProcessed(*this));
}

unsigned int CCustProcessed::hashVal() const
{
	return BKDRHash(m_sServiceId.c_str());
}


CServiceWait::CServiceWait(CCustWait sessionRec)
{
	m_encrypt = sessionRec.m_encrypt;
	m_LastMsgTime = sessionRec.m_LastMsgTime;
	m_sCustId = sessionRec.m_sCustId;
	m_sLastMsg = sessionRec.m_sLastMsg;
	m_sServiceId = sessionRec.m_sServiceId;
	m_sUserId = sessionRec.m_sUserId;
}


CServiceWait::CServiceWait(CCustProcessed sessionRec)
{
	m_encrypt = sessionRec.m_encrypt;
	m_LastMsgTime = sessionRec.m_LastMsgTime;
	m_sCustId = sessionRec.m_sCustId;
	m_sLastMsg = sessionRec.m_sLastMsg;
	m_sServiceId = sessionRec.m_sServiceId;
	m_sUserId = sessionRec.m_sUserId;
}


bsoncxx::builder::basic::document CServiceWait::KeyDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(CUSTWAIT_FIELD_USERID, m_sUserId),
		kvp(CUSTWAIT_FIELD_SERVICEID, m_sServiceId)
		);
	return doc;
}

bsoncxx::builder::basic::document CServiceWait::ToDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(CUSTWAIT_FIELD_USERID, m_sUserId),
		kvp(CUSTWAIT_FIELD_SERVICEID, m_sServiceId),
		kvp(CUSTWAIT_FIELD_ENCRYPT, (int32_t)m_encrypt),
		kvp(CUSTWAIT_FIELD_LASTMSG, m_sLastMsg),
		kvp(CUSTWAIT_FIELD_LASTMSG_TIME, (int64_t)m_LastMsgTime)
		);
	return doc;
}

bool CServiceWait::IsValid() const
{
	return !m_sUserId.empty() && !m_sServiceId.empty();
}

std::shared_ptr<IMongoDataEntry> CServiceWait::Clone()const
{
	return std::shared_ptr<IMongoDataEntry>(new CServiceWait(*this));
}

unsigned int CServiceWait::hashVal() const
{
	return BKDRHash(m_sServiceId.c_str());
}




