/******************************************************************************
Filename: offlineMsg.cpp
Author:TongHuaizhi 			Version:1.0.0 		Date:2017/06/05
Description:
******************************************************************************/
#include "grpOfflineMsg.h"
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

#define GRPOFFLINEMSG_FIELD_FROMID_STR			"fromId"
#define GRPOFFLINEMSG_FIELD_CREATETIME_STR		"createTime"

CGrpOfflineMsg::CGrpOfflineMsg()
	: m_nCreateTime(0)
{

}

CGrpOfflineMsg::CGrpOfflineMsg(const im::GroupChat& msg)
	: m_nCreateTime(msg.msgtime()), m_sMsgId(msg.smsgid()), m_sFromId(msg.sfromid()), m_sGrpId(msg.sgroupid())
{
	msg.SerializeToString(&m_sMsgData);
}

CGrpOfflineMsg::CGrpOfflineMsg(const string& grpId, const string& msgId, const string& msgData, uint64_t createTime, const string& fromId)
	: m_nCreateTime(createTime), m_sMsgId(msgId), m_sFromId(fromId), m_sGrpId(grpId), m_sMsgData(msgData)
{

}

bsoncxx::builder::basic::document CGrpOfflineMsg::KeyDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(GRPOFFLINEMSG_FIELD_MSGID_STR, GetMsgId()),
		kvp(GRPOFFLINEMSG_FIELD_FROMID_STR, GetFromId()),
		kvp(GRPOFFLINEMSG_FIELD_GRPID_STR, GetGrpId())
		);
	return doc;
}

bsoncxx::builder::basic::document CGrpOfflineMsg::ToDoc() const
{
	bsoncxx::builder::basic::document doc{};
	doc.append(
		kvp(GRPOFFLINEMSG_FIELD_MSGID_STR, GetMsgId()),
		kvp(GRPOFFLINEMSG_FIELD_FROMID_STR, GetFromId()),
		kvp(GRPOFFLINEMSG_FIELD_GRPID_STR, GetGrpId()),
		kvp(GRPOFFLINEMSG_FIELD_MSGDATA_STR, GetMsgData()),
		kvp(GRPOFFLINEMSG_FIELD_CREATETIME_STR, GetCreateTime())
		);
	return doc;
}

bool CGrpOfflineMsg::IsValid() const
{
	return m_nCreateTime != 0 && !m_sMsgId.empty();
}

std::shared_ptr<IMongoDataEntry> CGrpOfflineMsg::Clone()const
{
	return std::shared_ptr<IMongoDataEntry>(new CGrpOfflineMsg(*this));
}

unsigned int CGrpOfflineMsg::hashVal() const
{
	return BKDRHash(m_sGrpId.c_str());
}

CGrpOfflineMsg viewToGrpOfflineMsg(const view& doc)
{
	string fromId;
	if (doc[GRPOFFLINEMSG_FIELD_FROMID_STR])
	{
		fromId = doc[GRPOFFLINEMSG_FIELD_FROMID_STR].get_utf8().value.to_string();
	}

	string grpId;
	if (doc[GRPOFFLINEMSG_FIELD_GRPID_STR])
	{
		grpId = doc[GRPOFFLINEMSG_FIELD_GRPID_STR].get_utf8().value.to_string();
	}

	string msgId;
	if (doc[GRPOFFLINEMSG_FIELD_MSGID_STR])
	{
		msgId = doc[GRPOFFLINEMSG_FIELD_MSGID_STR].get_utf8().value.to_string();
	}

	int createTime = 0;
	if (doc[GRPOFFLINEMSG_FIELD_CREATETIME_STR])
	{
		createTime = doc[GRPOFFLINEMSG_FIELD_CREATETIME_STR].get_int64();
	}

	string msgData;
	if (doc[GRPOFFLINEMSG_FIELD_MSGDATA_STR])
	{
		msgData = doc[GRPOFFLINEMSG_FIELD_MSGDATA_STR].get_utf8().value.to_string();
	}

	return CGrpOfflineMsg(grpId, msgId, msgData, createTime, fromId);
}
