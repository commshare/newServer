/******************************************************************************
Filename: offlineMsg.cpp
Author:TongHuaizhi 			Version:1.0.0 		Date:2017/06/05
Description:
******************************************************************************/
#include "channel_offline_msg.h"
#include "mongoTask.h"
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

#define FIELD_FROMID_STR		"fromId"
#define FIELD_RADIOID_STR		"radioId"
#define FIELD_MSGID_STR			"msgId"
#define FIELD_MSGETIME_STR		"msgTime"
#define FIELD_ENCRYPT_STR		"encrypt"
#define FIELD_CONTENT_STR		"content"
#define FIELD_EXTEND_STR		"extend"

CChannelOfflineMsg::CChannelOfflineMsg(const im::RadioChat& msg)
	: m_sFromId(msg.sfromid())
	, m_sRadioId(msg.sradioid())
	, m_sMsgId(msg.smsgid())
	, m_nMsgTime(msg.msgtime())
	, m_nEncrypt(msg.encrypt())
	, m_sContent(msg.scontent())
	, m_sExtend(msg.extend())
{
	
}

bsoncxx::builder::basic::document CChannelOfflineMsg::ToDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(FIELD_FROMID_STR, m_sFromId),
		kvp(FIELD_RADIOID_STR, m_sRadioId),
		kvp(FIELD_MSGID_STR, m_sMsgId),
		kvp(FIELD_MSGETIME_STR, m_nMsgTime),
		kvp(FIELD_ENCRYPT_STR, m_nEncrypt),
		kvp(FIELD_CONTENT_STR, m_sContent),
		kvp(FIELD_EXTEND_STR, m_sExtend)
		);
	return doc;
}

bool CChannelOfflineMsg::IsValid() const
{
	return m_nMsgTime != 0 && !m_sMsgId.empty();
}

std::shared_ptr<IMongoDataEntry> CChannelOfflineMsg::Clone()const
{
	return std::shared_ptr<IMongoDataEntry>(new CChannelOfflineMsg(*this));
}

unsigned int CChannelOfflineMsg::hashVal() const
{
	return BKDRHash(m_sRadioId.c_str());
}

bsoncxx::builder::basic::document CChannelOfflineMsg::KeyDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(FIELD_MSGID_STR, m_sMsgId),
		kvp(FIELD_FROMID_STR, m_sFromId),
		kvp(FIELD_RADIOID_STR, m_sRadioId)
		);
	return doc;
}
