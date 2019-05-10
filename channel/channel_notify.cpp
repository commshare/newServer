#include "channel_notify.h"
#include <algorithm>
#include <list>
#include "util.h"
#include "encdec.h"
#include "im.pub.pb.h"
#include "redisMgr.h"
#include "common.h"
#include "thread_pool_manager.h"
#include <json/json.h>
#include "im_time.h"
#include "channelMongoOperator.h"

const int ProcessCount = 500;

CChannelNotify::CChannelNotify(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader)
	, m_nNumberOfInst(nNumOfInst)
{

}

CChannelNotify::~CChannelNotify()
{

}

bool CChannelNotify::OnChannelNotify(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	im::SVRRadioMsgNotify msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return false;
	}
	
	im::SVRMSGNotifyACK notifyAck;
	notifyAck.set_smsgid(msg.smsgid());
	notifyAck.set_msgtime(getCurrentTime());
	notifyAck.set_errcode(im::NON_ERR);
	sendAck(&notifyAck, im::SVR_RADIO_RELATIN_NOTIFY_ACK, pPdu->GetSessionId());
	
	CThreadPoolManager::getInstance()->getInsertGroupMsgPool()->add_task(&CChannelNotify::insertChannelNotifyToDataBase, this, msg);
	return true;
}

void CChannelNotify::insertChannelNotifyToDataBase(im::SVRRadioMsgNotify msg)
{
	// 解析数据,维护缓存队列
	updateUserChannelInfo(msg);
	// 创建通知创建mongo集合
	if(im::SVRRADIO_TYPE_CREATE == msg.notifytype())
	{
		string dirCollectionName = "chnn_" + msg.sradioid();
		ChannelMongoOperator::getInstance()->checkAndCreateCollection(dirCollectionName);
		return;
	}
	// 组装数据，插入数据库，分发出来
	im::RadioChat chatMsg;
	packagingRadioChat(msg, chatMsg);
	
//	if(im::SVRRADIO_TYPE_NOSPEAK != msg.notifytype() || im::SVRRADIO_TYPE_SPEAKING == msg.notifytype())		// 禁言不保存数据库
//	{
//		DbgLog("%s channel msg insert mongo begin", msg.smsgid().c_str());
//		string dirCollectionName = "chnn_" + msg.sradioid();
//		ChannelMongoOperator::getInstance()->checkAndCreateCollection(dirCollectionName);
//		string currId = ChannelMongoOperator::getInstance()->InsertChannelOfflineMsg(chatMsg, dirCollectionName);
//		if(currId.empty() || currId == "E11000")
//		{
//			ErrLog("insert mongo fail! collection:%s msgId=%s radioId=%s", dirCollectionName.c_str(), msg.smsgid().c_str(), msg.sradioid().c_str());
//			return;
//		}
//		
//		chatMsg.set_sid(currId);
//		if(!CRedisMgr::getInstance()->pushMsgToMsgQueue(chatMsg, currId))
//		{
//			ErrLog("insert redis fail! collection:%s msgId=%s radioId=%s", dirCollectionName.c_str(), msg.smsgid().c_str(), msg.sradioid().c_str());
//			return;
//		}
//	}

	// 获取到频道里面的成员
	std::vector<string> vecMember;
	DbgLog("%s channel msg get radio member list begin", msg.smsgid().c_str());
//	string strCode = "";
//	if(!CRedisMgr::getInstance()->getChannelMemberList(m_chnnMemListUrl, m_sAppSecret, msg.sradioid(), vecMember, strCode))
//	{
//		ErrLog("get channel member list fail! code= %s", strCode.c_str());
//		return;
//	}

	// 被踢或主动退出的成员需要通知
	if(im::SVRRADIO_TYPE_MEMBER_QUIT == msg.notifytype())
	{
		vecMember.emplace_back(msg.sopruserid());
	}
	else if(im::SVRRADIO_TYPE_MEMBER_REMOVE == msg.notifytype())
	{
		for (int i = 0; i < msg.smnpleduserid_size(); i++)
		{
			vecMember.emplace_back(msg.smnpleduserid(i));
		}
	}
	CRedisMgr::getInstance()->getChannelUser(msg.sradioid(), vecMember);
	if(vecMember.empty())
	{
		ErrLog("get channel member list fail! radioid= %s", msg.sradioid().c_str());
		return;
	}
	
	DbgLog("%s channel msg get radio member list end", msg.smsgid().c_str());
	
	std::vector<string> tmpMember;
	int nTick = 0;
	for(auto& itor : vecMember)
	{
		nTick++;
		tmpMember.emplace_back(itor);
		if(nTick >= ProcessCount)
		{
			CThreadPoolManager::getInstance()->getSendGroupMsgPool()->add_task(&CChannelNotify::sendChannelNotify, this, chatMsg, tmpMember);
			tmpMember.clear();
			nTick = 0;
		}
	}
	if(nTick > 0)
		CThreadPoolManager::getInstance()->getSendGroupMsgPool()->add_task(&CChannelNotify::sendChannelNotify, this, chatMsg, tmpMember);
	vecMember.clear();
}

void CChannelNotify::sendChannelNotify(im::RadioChat msg, std::vector<string> vecMember)
{
	DbgLog("send channel notify msg to member! useId:%s, msgId:%s, radioId:%s", msg.sfromid().c_str(), msg.smsgid().c_str(), msg.sradioid().c_str());
	DbgLog("%s channel notify msg send to members begin", msg.smsgid().c_str());
	std::vector<USER_LOGIN_INIFO_> userInfoList;
	if(!CRedisMgr::getInstance()->getUserLoginInfoList(vecMember, userInfoList))
	{
		return;
	}
	
	for(auto& itor : userInfoList)
	{
		if(0 == itor.status || itor.userId == msg.sfromid())
			continue;
		msg.set_stoid(itor.userId);
		sendReq(&msg, RADIO_NOTIFY_DELIVER, itor.strIp, itor.nPort);
	}
	DbgLog("%s channel notify msg send to members end", msg.smsgid().c_str());
}

bool CChannelNotify::updateUserChannelInfo(const im::SVRRadioMsgNotify& msg)
{
	im::SVRRadioNotifyType type = msg.notifytype();
	if(im::SVRRADIO_TYPE_MEMBER_QUIT == type || im::SVRRADIO_TYPE_APPLY == type)	// 主动申请及退出
	{
		if(im::SVRRADIO_TYPE_MEMBER_QUIT == type)
		{
			CRedisMgr::getInstance()->removeUserToChannel(msg.sradioid(), msg.sopruserid());
			CRedisMgr::getInstance()->removeOfflineUserToChannel(msg.sradioid(), msg.sopruserid());
			CRedisMgr::getInstance()->removeChannelToUser(msg.sopruserid(), msg.sradioid());
		}
		else
		{
			CRedisMgr::getInstance()->addChannelToUser(msg.sopruserid(), msg.sradioid());
			
			USER_LOGIN_INIFO_ userInfo;
			if(!CRedisMgr::getInstance()->getUserLoginInfo(msg.sopruserid(), userInfo) || 0 == userInfo.status)
				return false;
			CRedisMgr::getInstance()->addUserToChannel(msg.sradioid(), msg.sopruserid());
		}
	}
	else if(im::SVRRADIO_TYPE_MEMBER_REMOVE == type || im::SVRRADIO_TYPE_INVITE == type)
	{
		std::vector<string> vecMember;
		for (int i = 0; i < msg.smnpleduserid_size(); i++)
		{
			vecMember.emplace_back(msg.smnpleduserid(i));
		}
		
		if(im::SVRRADIO_TYPE_MEMBER_REMOVE == type)
		{
			CRedisMgr::getInstance()->removeUserToChannel(msg.sradioid(), vecMember);
			CRedisMgr::getInstance()->removeOfflineUserToChannel(msg.sradioid(), vecMember);
			CRedisMgr::getInstance()->removeChannelToUser(vecMember, msg.sradioid());
		}
		else
		{
			// 添加到用户频道表中
			CRedisMgr::getInstance()->addChannelToUser(vecMember, msg.sradioid());
			// 在线的用户添加到频道用户表中
			std::vector<USER_LOGIN_INIFO_> userInfoList;
			if(!CRedisMgr::getInstance()->getUserLoginInfoList(vecMember, userInfoList) || userInfoList.size() <= 0)
				return false;
			vecMember.clear();
			for(auto& itor : userInfoList)
			{
				if(1 == itor.status)
					vecMember.emplace_back(itor.userId);
			}
			if(vecMember.size() <= 0)
				return false;
			
			CRedisMgr::getInstance()->addUserToChannel(msg.sradioid(), vecMember);
		}
	}
	else if(im::SVRRADIO_TYPE_DISMISS == type)
	{
		CRedisMgr::getInstance()->setChannelStatus(msg.sradioid(), CHNN_DISMISS);
		CRedisMgr::getInstance()->deleteChannelUserKey(msg.sradioid());
		CRedisMgr::getInstance()->deleteChannelOfflineUserKey(msg.sradioid());
	}
	else if(im::SVRRADIO_TYPE_CREATE == type)
	{
		CHANNEL_INIFO_ chnnInfo;
		chnnInfo.radioId = msg.sradioid();
		chnnInfo.status = CHNN_NORMAL;
		chnnInfo.unspeak = 0;
		CRedisMgr::getInstance()->setChannelInfo(chnnInfo);
		CRedisMgr::getInstance()->addChannelAdmin(msg.sradioid(), msg.sopruserid());
	}
	else if(im::SVRRADIO_TYPE_NOSPEAK == type || im::SVRRADIO_TYPE_SPEAKING == type)
	{
		int nSpeak = im::SVRRADIO_TYPE_NOSPEAK == type ? 1 : 0;
		CRedisMgr::getInstance()->setChannelSpeak(msg.sradioid(), nSpeak);
	}
	else if(im::SVRRADIO_TYPE_ADMIN_SET == type || im::SVRRADIO_TYPE_ADMIN_UNSET == type || im::SVRRADIO_TYPE_MASTER_CHANGED == type)
	{
		std::vector<string> vecMember;
		for (int i = 0; i < msg.smnpleduserid_size(); i++)
		{
			vecMember.emplace_back(msg.smnpleduserid(i));
		}
		if(im::SVRRADIO_TYPE_ADMIN_SET == type)
			CRedisMgr::getInstance()->addChannelAdminList(msg.sradioid(), vecMember);
		else if(im::SVRRADIO_TYPE_ADMIN_UNSET == type)
			CRedisMgr::getInstance()->removeChannelAdminList(msg.sradioid(), vecMember);
		else
		{
			CRedisMgr::getInstance()->addChannelAdminList(msg.sradioid(), vecMember);
			CRedisMgr::getInstance()->removeChannelAdmin(msg.sradioid(), msg.sopruserid());
		}
	}
	return true;
}

bool CChannelNotify::packagingRadioChat(const im::SVRRadioMsgNotify& msg, im::RadioChat& chat)
{
	chat.set_sfromid(msg.sopruserid());
	chat.set_sradioid(msg.sradioid());
	chat.set_smsgid(msg.smsgid());
	chat.set_msgtime(getCurrentTime());
	chat.set_encrypt(0);
	chat.set_extend(msg.extend());
	im::SVRRadioNotifyType type = msg.notifytype();
	Json::Value msgContent;
	msgContent["contentType"] = 101;
	msgContent["type"] = type;
	// 频道资料修改
	if(im::SVRRADIO_TYPE_NAME_CHANGED == type || im::SVRRADIO_TYPE_ICON_CHANGED == type || im::SVRRADIO_TYPE_NOTICE_CHANGED == type)
	{
		msgContent["text"] = msg.scontent();
	}
	// 主动加入和退出频道、解散频道
	else if(im::SVRRADIO_TYPE_MEMBER_QUIT == type || im::SVRRADIO_TYPE_APPLY == type || im::SVRRADIO_TYPE_DISMISS == type)
	{
		msgContent["oprUserId"] = msg.sopruserid();
	}
	// 被动加入和推出频道、频道主变更、设置和取消频道管理员
	else if(im::SVRRADIO_TYPE_MEMBER_REMOVE == type || im::SVRRADIO_TYPE_INVITE == type || im::SVRRADIO_TYPE_NOSPEAK == type 
			|| im::SVRRADIO_TYPE_MASTER_CHANGED == type || im::SVRRADIO_TYPE_ADMIN_SET == type || im::SVRRADIO_TYPE_ADMIN_UNSET == type || im::SVRRADIO_TYPE_SPEAKING == type)
	{
		msgContent["oprUserId"] = msg.sopruserid();
		for (int i = 0; i < msg.smnpleduserid_size(); i++)
		{
			msgContent["mnpledUserId"].append(msg.smnpleduserid(i));
		}
	}
	DbgLog("notify msg content=%s, extend=%s", msgContent.toStyledString().c_str(), msg.extend().c_str());
	chat.set_scontent(msgContent.toStyledString());
	return true;
}

bool CChannelNotify::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(im::SVR_RADIO_RELATIN_NOTIFY, m_nNumberOfInst, CommandProc(&CChannelNotify::OnChannelNotify));
	return true;
}




