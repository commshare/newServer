/******************************************************************************
Filename: grpOfflineMsg.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/04
Description: 保存群组原始消息内容
******************************************************************************/
#ifndef __GRPOFFLINEMSG_H__
#define __GRPOFFLINEMSG_H__

#include <string>
#include "mongoDbColl.h"
#include "bsoncxx/builder/basic/document.hpp"
//#include "im.group.pb.h"
#include "im.mes.pb.h"
#include "ostype.h"

using std::string;
using bsoncxx::document::view;

#define GRPOFFLINEMSG_FIELD_GRPID_STR			"grpId"
#define GRPOFFLINEMSG_FIELD_MSGID_STR			"msgId"
#define GRPOFFLINEMSG_FIELD_MSGDATA_STR			"msgData"		//保存整条消息序列化之后的结果
#define GRPOFFLINEMSG_FIELD_CREATETIME_STR			"createTime"

class CGrpOfflineMsg : public IMongoDataEntry
{
public:
	CGrpOfflineMsg();

	CGrpOfflineMsg(const im::MESGrpChat& msg);				
	CGrpOfflineMsg(const string& grpId, const string& msgId, const string& msgData, uint64_t createTime, const string& fromId = string(""));

	/* msgchat content and offlineMsg createTime readOnly after construct */
	const string&	GetMsgId()const		{ return m_sMsgId; }
	const string&	GetFromId()const	{ return m_sFromId; }
	const string&	GetGrpId()const	{ return m_sGrpId; }
	const string&	GetMsgData()const	{ return m_sMsgData; }
	int64_t			GetCreateTime()const{ return m_nCreateTime; }

	virtual bsoncxx::builder::basic::document ToDoc() const override;

	virtual bool IsValid() const override;

	virtual std::shared_ptr<IMongoDataEntry> Clone()const override;

	virtual unsigned int hashVal() const override;

	virtual bsoncxx::builder::basic::document KeyDoc() const override;

private:

	const uint64_t	m_nCreateTime = 0;		//	离线消息记录生成时间
	const string	m_sMsgId = "";
	const string	m_sFromId = "";
	const string	m_sGrpId = "";
	string    m_sMsgData = "";

};
CGrpOfflineMsg viewToGrpOfflineMsg(const view& doc);

class CGrpOfflineMsgKeys :public IMongoDataDelKeys
{
public:
	CGrpOfflineMsgKeys(const CGrpOfflineMsg& msg);
	CGrpOfflineMsgKeys(const string& grpId, const string& msgId);
	virtual bsoncxx::builder::basic::document ToDoc()const override;

	virtual std::shared_ptr<IMongoDataDelKeys> Clone()const override;

	virtual unsigned int hashVal() const override;

private:
	const string m_grpId;
	const string m_msgId;
};

#endif // __GRPOFFLINEMSG_H__
