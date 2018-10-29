/******************************************************************************
Filename: offlineMsg.cpp
Author:TongHuaizhi 			Version:1.0.0 		Date:2017/06/05
Description:
******************************************************************************/
#include "offlineMsg.h"
#include "mongoTask.h"
//#include "threadpool.h"
#include "mongoDbManager.h"
#include "im_time.h"
#include "util.h"
#include "im.pub.pb.h"
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
using namespace im;





COfflineMsg::COfflineMsg()
	: m_nCreateTime(0)
{

}

COfflineMsg::COfflineMsg(const MESChat& msg)
	:m_nCreateTime(getCurrentTime()), m_nCmdId(MES_CHAT_DELIVER), m_nIsChatDeliver(1), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sfromid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}


COfflineMsg::COfflineMsg(const im::MESChatCancel& msg)
	:m_nCreateTime(msg.msgtime()), m_nCmdId(MES_CHATCANCEL_DELIVER), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sfromid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}


COfflineMsg::COfflineMsg(const MESChatDeliveredAck& msg)
	: m_nCreateTime(getCurrentTime()), m_nCmdId(MES_CHAT_DELIVERED_NOTIFICATION), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sfromid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const MESChatRead& msg)
	: m_nCreateTime(getCurrentTime()), m_nCmdId(MES_CHAT_READ_DELIVER), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sfromid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const MESOfflineMsgDelivered& msg)
	: m_nCreateTime(getCurrentTime()), m_nCmdId(MES_OFFLINEMSG_DELIVERED_NOTIFICATION), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sfromid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}


COfflineMsg::COfflineMsg(const MESAddFriend& msg)
	: m_nCreateTime(getCurrentTime()), m_nCmdId(MES_ADDFRIEND_DELIVER), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sfromid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const MESAddFriendAns& msg)
	: m_nCreateTime(getCurrentTime()), m_nCmdId(MES_ADDFRIEND_ANS_DELIVER), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sfromid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}
COfflineMsg::COfflineMsg(const string& fromId, const string& toId, const string& msgId, uint32_t cmdId, const string& msgData, uint64_t createTime, const string& fromGrpUserId, int32_t status)
			:m_nCreateTime(createTime), m_nCmdId(cmdId), m_sMsgId(msgId), m_sFromId(fromId),m_sFromGrpUserId(fromGrpUserId), m_sToId(toId), m_sMsgData(msgData), m_nStatus(status)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////
COfflineMsg::COfflineMsg(const im::MESGrpChat& msg)
	: m_nCreateTime(msg.msgtime()), m_nCmdId(MES_GRPCHAT_DELIVER), m_nIsChatDeliver(1)
	, m_sMsgId(msg.smsgid()),m_sFromId(msg.sgrpid()),m_sFromGrpUserId(msg.sfromid()), m_sToId(msg.stoid())
{

}

COfflineMsg::COfflineMsg(const im::MESGrpNotify& msg)
	: m_nCreateTime(msg.msgtime()), m_nCmdId(MES_GRPNOTIFY_DELIVER), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sgrpid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const im::MESJoinGrp& msg)
	: m_nCreateTime(getCurrentTime()), m_nCmdId(MES_JOINGRP_DELIVER), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sgrpid()), m_sFromGrpUserId(msg.sfromid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const im::MESExchangeKeyDeliver& msg)
	: m_nCreateTime(getCurrentTime()), m_nCmdId(MES_EXCHANGE_KEY_DELIVER), m_sMsgId(msg.smsgid()),
	m_sFromId((msg.exchangemode() & 0x10) == 0 ? msg.sgrpid() : msg.sfromid()), m_sFromGrpUserId(msg.sfromid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const im::MESExchangeKeyDeliverAck& msg)
	: m_nCreateTime(getCurrentTime()), m_nCmdId(MES_EXCHANGE_KEY_DELIVERD_NOTIFY), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sgrpid()), m_sFromGrpUserId(msg.sfromid()), m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const im::SIGSponsorCall& msg)
	: m_nCreateTime(getCurrentTime()), m_nCmdId(SIG_SPONSORP2PCALL_DELIVER), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sfromid()), m_sToId(msg.sinviteid())
	, m_sCallId(msg.scallid())
{
	msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const im::SIGHangUp& msg)
	: m_nCreateTime(getCurrentTime()), m_nCmdId(SIG_P2PCALLHANGUPDElIVER), m_sMsgId(msg.smsgid()),
	m_sFromId(msg.sfromid()), m_sToId(msg.stoid())
{
    msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const SIGSponsorCallAns &msg)
    : m_nCreateTime(getCurrentTime()), m_nCmdId(SIG_SPONSORP2PCALL_ANS_DELIVER), m_sMsgId(msg.smsgid()),
    m_sFromId(msg.sfromid()), m_sToId(msg.stoid())
{
    msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const im::MSGCommonNotify& msg)
	: m_nCreateTime(getCurrentTime())
	, m_nCmdId(MS_COMMONNOTIFY_DELIVER)
	, m_sMsgId(msg.smsgid())
	, m_sFromId(msg.sfromid())
	, m_sToId(msg.stoid())
{
	msg.SerializeToString(&m_sMsgData);
}

COfflineMsg::COfflineMsg(const im::SIGP2PCallExchangeNatInfo& msg)
	: m_nCreateTime(getCurrentTime())
	, m_nCmdId(SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER)
	, m_sMsgId(msg.smsgid())
	, m_sFromId(msg.sfromid())
	, m_sToId(msg.stoid())

{
	msg.SerializeToString(&m_sMsgData);
}



//COfflineMsg::COfflineMsg(const im::MESJoinGrpAns& msg)
//	: m_nCreateTime(getCurrentTime()), m_nCmdId(MES_JOINGRP_ANS_DELIVER), m_sMsgId(msg.smsgid()),
//	m_sFromId(msg.sgrpid()), m_sFromGrpUserId(msg.sfromid()), m_sToId(msg.stoid())
//{
//	msg.SerializeToString(&m_sMsgData);
//}

bool COfflineMsg::isValid() const
{
	return m_nCreateTime != 0 && m_nCmdId != 0/*&& !mMsgChat.smsgid().empty()*/;
}


bsoncxx::builder::basic::document COfflineMsg::KeyDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(kvp(OFFLINEMSG_FIELD_TOID_STR, GetToId()));
	if (!GetMsgId().empty())
	{
		doc.append(kvp(OFFLINEMSG_FIELD_MSGID_STR, GetMsgId()));
	}

	if (GetCmdId() != 0)
	{
		doc.append(kvp(OFFLINEMSG_FIELD_CMDID_STR, GetCmdId()));
	}
	return doc;
}

bsoncxx::builder::basic::document COfflineMsg::ToDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(OFFLINEMSG_FIELD_MSGID_STR, GetMsgId()),
		kvp(OFFLINEMSG_FIELD_FROMID_STR, GetFromId()),
		kvp(OFFLINEMSG_FIELD_TOID_STR, GetToId()),
		kvp(OFFLINEMSG_FIELD_CMDID_STR, GetCmdId()),
		kvp(OFFLINEMSG_FIELD_MSGDATA_STR, GetMsgData()),
		kvp(OFFLINEMSG_FIELD_CREATETIME_STR, GetCreateTime()),
		kvp(OFFLINEMSG_FIELD_ISCHATDELIVER_STR, IsChatDeliver())
		);

	// 视频/语言通话添加字段
	if(!m_sCallId.empty()  && m_nCmdId == SIG_SPONSORP2PCALL_DELIVER)
	{
		doc.append(kvp(OFFLINEMSG_FIELD_CALLID_STR, GetCallId()));
		doc.append(kvp(OFFLINEMSG_FIELD_STATUS_STR, GetMsgStatus()));
	}

	return doc;
}

bool COfflineMsg::IsValid() const
{
	return m_nCmdId!= 0 && !m_sMsgId.empty();
}

std::shared_ptr<IMongoDataEntry> COfflineMsg::Clone()const
{
	return std::shared_ptr<IMongoDataEntry>(new COfflineMsg(*this));
}

unsigned int COfflineMsg::hashVal() const
{
	return COfflineMsgKeys(*this).hashVal();
}


COfflineMsg viewToOfflineMsg(const view& doc)
{
	string fromId;
	if (doc[OFFLINEMSG_FIELD_FROMID_STR])
	{
		fromId = doc[OFFLINEMSG_FIELD_FROMID_STR].get_utf8().value.to_string();
	}

	string toId;
	if (doc[OFFLINEMSG_FIELD_TOID_STR])
	{
		toId = doc[OFFLINEMSG_FIELD_TOID_STR].get_utf8().value.to_string();
	}

	string msgId;
	if (doc[OFFLINEMSG_FIELD_MSGID_STR])
	{
		msgId = doc[OFFLINEMSG_FIELD_MSGID_STR].get_utf8().value.to_string();
	}

	uint32_t cmdId;
	if (doc[OFFLINEMSG_FIELD_CMDID_STR])
	{
		cmdId = doc[OFFLINEMSG_FIELD_CMDID_STR].get_int32();
	}

	//int createTime = 0;
	uint64_t createTime = 0;
	if (doc[OFFLINEMSG_FIELD_CREATETIME_STR])
	{
		createTime = doc[OFFLINEMSG_FIELD_CREATETIME_STR].get_int64();
	}

	string msgData;
	if (doc[OFFLINEMSG_FIELD_MSGDATA_STR])
	{
		msgData = doc[OFFLINEMSG_FIELD_MSGDATA_STR].get_utf8().value.to_string();
	}
	
	// 语音/视频 通话要把状态给带出来
 	if(cmdId == SIG_SPONSORP2PCALL_DELIVER)
	{
		uint32_t status = 0;
		if(doc[OFFLINEMSG_FIELD_STATUS_STR])
			status = doc[OFFLINEMSG_FIELD_STATUS_STR].get_int32();
		return COfflineMsg(fromId, toId, msgId, cmdId, msgData, createTime, "", status);
	}
	else
		return COfflineMsg(fromId, toId, msgId, cmdId, msgData, createTime);

}

//////////////////////////////////////////////////////////////////////////////////////////////


COfflineMsgKeys::COfflineMsgKeys(const string& toId, int32_t cmdId, const string& msgId)
	:m_toId(toId), m_cmdId(cmdId), m_msgId(msgId)
{

}

COfflineMsgKeys::COfflineMsgKeys(const COfflineMsg& msg)
	: m_toId(msg.GetToId()), m_cmdId(msg.GetCmdId()), m_msgId(msg.GetMsgId())
{

}

bsoncxx::builder::basic::document COfflineMsgKeys::ToDoc()const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(kvp(OFFLINEMSG_FIELD_TOID_STR, m_toId));
	if (!m_msgId.empty())
	{
		doc.append(kvp(OFFLINEMSG_FIELD_MSGID_STR, m_msgId));
	}

	if (m_cmdId != 0)
	{
		doc.append(kvp(OFFLINEMSG_FIELD_CMDID_STR, m_cmdId));
	}
	return doc;
}

std::shared_ptr<IMongoDataDelKeys> COfflineMsgKeys::Clone()const
{
	return std::shared_ptr<IMongoDataDelKeys>(new COfflineMsgKeys(*this));
}

unsigned int COfflineMsgKeys::hashVal() const
{
	return BKDRHash(m_toId.c_str());
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCallOfflineMsgKeys::CCallOfflineMsgKeys(const string& toId, int32_t cmdId, const string& scallId)
			:m_toId(toId), m_cmdId(cmdId), m_scallId(scallId)
{

}
	
bsoncxx::builder::basic::document CCallOfflineMsgKeys::ToDoc()const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(kvp(OFFLINEMSG_FIELD_TOID_STR, m_toId));
	if(!m_scallId.empty())
		doc.append(kvp(OFFLINEMSG_FIELD_CALLID_STR, m_scallId));

	if (m_cmdId != 0)
		doc.append(kvp(OFFLINEMSG_FIELD_CMDID_STR, m_cmdId));
	return doc;
}

std::shared_ptr<IMongoDataDelKeys> CCallOfflineMsgKeys::Clone()const
{
	return std::shared_ptr<IMongoDataDelKeys>(new CCallOfflineMsgKeys(*this));
}

unsigned int CCallOfflineMsgKeys::hashVal() const
{
	return BKDRHash(m_toId.c_str());
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
COfflineMsgDelSpecificUserManyMsgKeys::COfflineMsgDelSpecificUserManyMsgKeys(const string& toId, int32_t cmdId, const std::vector<string>& msgIds)
	:m_toId(toId), m_cmdId(cmdId), m_msgIds(msgIds)
{

}


bsoncxx::builder::basic::document COfflineMsgDelSpecificUserManyMsgKeys::ToDoc() const
{
	using bsoncxx::builder::basic::array;

	array arrMsgIds;
	for (auto&& msgId : m_msgIds)
	{
		arrMsgIds.append(msgId);
	}

	bsoncxx::builder::basic::document subDoc{};
	subDoc.append(kvp("$in", arrMsgIds));

	bsoncxx::builder::basic::document doc{};
	doc.append(kvp(OFFLINEMSG_FIELD_TOID_STR, m_toId));
	if (m_cmdId != 0)
	{
		doc.append(kvp(OFFLINEMSG_FIELD_CMDID_STR, m_cmdId));
	}
	doc.append(kvp(OFFLINEMSG_FIELD_MSGID_STR, subDoc));

	return doc;
}


std::shared_ptr<IMongoDataDelKeys> COfflineMsgDelSpecificUserManyMsgKeys::Clone() const
{
	return std::shared_ptr<IMongoDataDelKeys>(new COfflineMsgDelSpecificUserManyMsgKeys(*this));
}

unsigned int COfflineMsgDelSpecificUserManyMsgKeys::hashVal() const
{
	return BKDRHash(m_toId.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////

COfflineMsgDelManyUserMsgKeys::COfflineMsgDelManyUserMsgKeys(const std::vector<string>& toIds, const int cmdId, const string& msgId)
	:m_toIds(toIds), m_cmdId(cmdId), m_msgId(msgId)
{

}


bsoncxx::builder::basic::document COfflineMsgDelManyUserMsgKeys::ToDoc() const
{
	using bsoncxx::builder::basic::array;

	array arrToIds;
	for (auto&& toId : m_toIds)
	{
		arrToIds.append(toId);
	}

	bsoncxx::builder::basic::document subDoc{};
	subDoc.append(kvp("$in", arrToIds));

	bsoncxx::builder::basic::document doc{};

	doc.append(kvp(OFFLINEMSG_FIELD_TOID_STR, subDoc));
	doc.append(kvp(OFFLINEMSG_FIELD_MSGID_STR, m_msgId));
	if (m_cmdId != 0)
	{
		doc.append(kvp(OFFLINEMSG_FIELD_CMDID_STR, m_cmdId));
	}

	return doc;
}

std::shared_ptr<IMongoDataDelKeys> COfflineMsgDelManyUserMsgKeys::Clone() const
{
	return std::shared_ptr<IMongoDataDelKeys>(new COfflineMsgDelManyUserMsgKeys(*this));
}

