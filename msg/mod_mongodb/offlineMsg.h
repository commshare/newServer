/******************************************************************************
Filename: offlineMsg.h
Author:TongHuaizhi 			Version:1.0.0 		Date:2017/06/05
Description: 数据类，保存离线消息的信息,主要用在mongodb中离线消息的存储和获取
******************************************************************************/
#ifndef __OFFLINEMSG_H__
#define __OFFLINEMSG_H__
#include <string>
#include "mongoDbColl.h"
#include "bsoncxx/builder/basic/document.hpp"
#include "im.mes.pb.h"
#include "im.sig.pb.h"
#include "ostype.h"


#define OFFLINEMSG_FIELD_MSGID_STR			"msgId"
#define OFFLINEMSG_FIELD_FROMID_STR			"fromId"
#define OFFLINEMSG_FIELD_TOID_STR			"toId"
#define OFFLINEMSG_FIELD_CMDID_STR			"cmdId"
#define OFFLINEMSG_FIELD_MSGDATA_STR		"msgData"		//保存整条消息序列化之后的结果
#define OFFLINEMSG_FIELD_CREATETIME_STR		"createTime"
#define OFFLINEMSG_FIELD_ISCHATDELIVER_STR	"isChatDeliver"
#define OFFLINEMSG_FIELD_ISPULLED_STR	    "bPulled"

#define OFFLINEMSG_FIELD_CALLID_STR			"callId"		// 视频/语言 通话的id
#define OFFLINEMSG_FIELD_STATUS_STR			"status"		// 视频/语言 通话状态

#define OFFLINEMSG_TOTAL_COUNT				"totalCount"
#define OFFLINEMSG_CHAT_COUNT				"chatCount"

using std::string;
using bsoncxx::document::view;

class COfflineMsg : public IMongoDataEntry
{
public:
	COfflineMsg();
	COfflineMsg(const im::MESChat& msg);				// MES_CHAT_DELIVERE
	COfflineMsg(const im::MESChatCancel& msg);			// MES_CHATCANCEL_DELIVER
	COfflineMsg(const im::MESChatDeliveredAck& msg);	// MES_CHAT_DELIVERED_NOTIFICATION
	COfflineMsg(const im::MESChatRead& msg);			// MES_CHAT_READ_DELIVER
	COfflineMsg(const im::MESOfflineMsgDelivered& msg);	// MES_OFFLINEMSG_DELIVERED_NOTIFICATION
	COfflineMsg(const im::MESAddFriend& msg);			// MES_ADDFRIEND_DELIVER
	COfflineMsg(const im::MESAddFriendAns& msg);
	COfflineMsg(const im::SIGSponsorCall& msg);			// SIG_SPONSORP2PCALL_DELIVER
	COfflineMsg(const im::SIGHangUp& msg);				// SIG_P2PCALLHANGUPDElIVER
    COfflineMsg(const im::SIGSponsorCallAns& msg);      //SIG_SPONSORP2PCALL_ANS_DELIVER
	
	COfflineMsg(const im::MESGrpChat& msg);				// MES_GRPCHAT_DELIVERE
	COfflineMsg(const im::MESGrpNotify& msg);
	COfflineMsg(const im::MESJoinGrp& msg);
	COfflineMsg(const im::MESExchangeKeyDeliver& msg);
    COfflineMsg(const im::MESExchangeKeyDeliverAck& msg);//SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER

	// 通知消息构造 offlinemsg by Abner
	COfflineMsg(const im::MSGCommonNotify& msg);
	// 通话网络信息交换
	COfflineMsg(const im::SIGP2PCallExchangeNatInfo& msg);
	
	//COfflineMsg(const im::MESJoinGrpAns& msg);
	COfflineMsg(const string& fromId, const string& toId, const string& msgId, uint32_t cmdId, const string& msgData, uint64_t createTime, const string& fromGrpUserId=string(""), int32_t status = 0);

	bool isValid()const;

	/* msgchat content and offlineMsg createTime readOnly after construct */
	const string&	GetMsgId()const		{ return m_sMsgId;/*mMsgChat.smsgid();*/ }
	const string&	GetFromId()const	{ return m_sFromId;/* mMsgChat.sfromid();*/ }
	const string&	GetFromGrpUserId()const{ return m_sFromGrpUserId; }
	const string&	GetToId()const		{ return m_sToId;/*mMsgChat.stoid();*/ }
	int32_t			GetCmdId()const		{ return m_nCmdId; }
	const string&	GetMsgData()const	{ return m_sMsgData; }
	int64_t			GetCreateTime()const{ return m_nCreateTime; }
	int32_t			IsChatDeliver()const{ return m_nIsChatDeliver; }
	
	int32_t			GetMsgStatus()const { return m_nStatus; }
	string			GetCallId()const { return m_sCallId;    }

	virtual bsoncxx::builder::basic::document ToDoc() const override;

	virtual bool IsValid() const override;
	
	virtual std::shared_ptr<IMongoDataEntry> Clone()const override;

	virtual unsigned int hashVal() const override;

	virtual bsoncxx::builder::basic::document KeyDoc() const override;

private:

	const uint64_t	m_nCreateTime;		//	离线消息记录生成时间
	const uint32_t	m_nCmdId = 0;
	const uint32_t  m_nIsChatDeliver = 0;
	const string	m_sMsgId = "";
	const string	m_sFromId = "";		//单聊时表示发起人， 群聊时表示的是群组ID,主要是为了能统一拉取离线消息
	const string	m_sFromGrpUserId = "";
	const string	m_sToId = "";
	string m_sMsgData = "";

	const string  m_sCallId = "";		// 针对voip 通话ID
	const uint32_t m_nStatus = 0;		// 0:正常 1:取消（针对voip）
};

COfflineMsg viewToOfflineMsg(const view& doc);

class COfflineMsgKeys :public IMongoDataDelKeys
{
public:
	COfflineMsgKeys(const COfflineMsg& msg);
	COfflineMsgKeys(const string& toId, int32_t cmdId, const string& msgId);
	virtual bsoncxx::builder::basic::document ToDoc()const override;

	virtual std::shared_ptr<IMongoDataDelKeys> Clone()const override;

	virtual unsigned int hashVal() const override;

private:
	const string m_toId;
	int32_t m_cmdId;
	const string m_msgId;
};

class CCallOfflineMsgKeys : public IMongoDataDelKeys
{
public:
	CCallOfflineMsgKeys(const string& toId, int32_t cmdId, const string& scallId);

	virtual bsoncxx::builder::basic::document ToDoc()const override;

	virtual std::shared_ptr<IMongoDataDelKeys> Clone()const override;

	virtual unsigned int hashVal() const override;

private:
	const string m_toId;
	int32_t m_cmdId;
	const string m_scallId;
};

//delete specific user's msgs with same cmdId
class COfflineMsgDelSpecificUserManyMsgKeys :public IMongoDataDelKeys
{
public:
	COfflineMsgDelSpecificUserManyMsgKeys(const string& toId, int32_t cmdId, const std::vector<string>& msgIds);
	virtual bsoncxx::builder::basic::document ToDoc()const override;

	virtual std::shared_ptr<IMongoDataDelKeys> Clone()const override;

	virtual unsigned int hashVal() const override;

private:
	const string m_toId;
	int32_t m_cmdId;
	const std::vector<string> m_msgIds;
};

//delete many use msgs with same msgId and same cmdId
class COfflineMsgDelManyUserMsgKeys :public IMongoDataDelKeys
{
public:
	COfflineMsgDelManyUserMsgKeys(const std::vector<string>& toIds, const int cmdId, const string& msgId);
	virtual bsoncxx::builder::basic::document ToDoc()const override;

	virtual std::shared_ptr<IMongoDataDelKeys> Clone()const override;

private:
	const std::vector<string> m_toIds;
	int32_t m_cmdId;
	const string m_msgId;
};
#endif // __OFFLINEMSG_H__
