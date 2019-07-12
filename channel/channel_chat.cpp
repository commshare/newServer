#include "channel_chat.h"
#include <algorithm>
#include <list>
#include "util.h"
#include "encdec.h"
#include "im.mes.pb.h"
#include "im.pub.pb.h"
#include "im_time.h"
#include "redisMgr.h"
#include "mongoDbManager.h"
#include "common.h"
#include "thread_pool_manager.h"
#include "HttpClient.h"
#include <json/json.h>
#include "channelMongoOperator.h"

using namespace im;
using namespace std;

//string currDbName = "channelSvr";

const int ProcessCount = 500;
//你有一条新消息！
const string NewMsgStr("\344\275\240\346\234\211\344\270\200\346\235\241\346\226\260\346\266\210\346\201\257");

CChannelChat::CChannelChat(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader)
	, m_nNumberOfInst(nNumOfInst)
{
}

CChannelChat::~CChannelChat()
{

}

bool CChannelChat::OnChannelChat(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);

	im::RadioChat msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return false;
	}

	msg.set_msgtime(getCurrentTime());//set server time

	if (msg.sradioid().empty() || msg.sfromid().empty())
	{
		WarnLog("channel chat (0x%x) %s, %s-->%s, grpId or fromId not specified\r\n", pPdu->GetCommandId(), msg.smsgid().c_str(),
			msg.sfromid().c_str(), msg.sradioid().c_str());
		return false;
	}
	
	im::ErrCode retCode = msg.scontent().empty() ? ERR_CHAT_UNHEALTHY : NON_ERR;
	if (retCode != NON_ERR)
	{
		sendChatAck(msg, pPdu->GetSessionId(), retCode, "content is illegal");
		return true;
	}

	CThreadPoolManager::getInstance()->getInsertGroupMsgPool()->add_task(&CChannelChat::insertChannelChatMsgToDataBase, this, msg, pPdu->GetSessionId());
	return true;
}

void CChannelChat::insertChannelChatMsgToDataBase(im::RadioChat msg, const UidCode_t& sessionId)
{
	im::ErrCode retCode = NON_ERR;
	CHANNEL_INIFO_ chnnInfo;
	CRedisMgr::getInstance()->getChannelInfo(msg.sradioid(), chnnInfo);
	retCode = chnnInfo.status != CHNN_NORMAL ? ERR_RADIO_DISMISS : NON_ERR;
	if(NON_ERR != retCode)
	{
		sendChatAck(msg, sessionId, retCode);
		return;
	}

	retCode = chnnInfo.unspeak == 1 ? ERR_RADIO_NOSPEAK : NON_ERR;
	std::vector<std::string> vecAdmin;
	CRedisMgr::getInstance()->getChannelAdmin(msg.sradioid(), vecAdmin);
	bool bFind = std::find(vecAdmin.begin(), vecAdmin.end(), msg.sfromid()) != vecAdmin.end();
	if(!bFind && NON_ERR != retCode)
	{
		sendChatAck(msg, sessionId, retCode);
		return;
	}
	
	
	std::vector<std::string> vecRadio;
	CRedisMgr::getInstance()->getUserChannel(msg.sfromid(), vecRadio);
	retCode = std::find(vecRadio.begin(), vecRadio.end(), msg.sradioid()) == vecRadio.end() ? ERR_RADIO_ISDELETE : NON_ERR;
	if(NON_ERR != retCode)
	{
		sendChatAck(msg, sessionId, retCode);
		return;
	}
	
	// 更新mongo
	DbgLog("%s channel msg insert mongo begin", msg.smsgid().c_str());
	string dirCollectionName = "chnn_" + msg.sradioid();
	ChannelMongoOperator::getInstance()->checkAndCreateCollection(dirCollectionName);
	string currId = ChannelMongoOperator::getInstance()->InsertChannelOfflineMsg(msg, dirCollectionName);
	DbgLog("%s channel msg insert mongo end, mongo index %s", msg.smsgid().c_str(), currId.c_str());
	if(currId.empty())
	{
		ErrLog("insert mongo fail! collection:%s msgId=%s radioId=%s", dirCollectionName.c_str(), msg.smsgid().c_str(), msg.sradioid().c_str());
		retCode = EXCEPT_ERR;
		sendChatAck(msg, sessionId, retCode);
		return;
	}
	else if(currId == "E11000")
	{
		ErrLog("repeat mongo fail! collection:%s msgId=%s radioId=%s", dirCollectionName.c_str(), msg.smsgid().c_str(), msg.sradioid().c_str());
		sendChatAck(msg, sessionId, retCode);
		return;
	}
	
	DbgLog("%s channel msg insert redis begin", msg.smsgid().c_str());
	msg.set_sid(currId);
	if(!CRedisMgr::getInstance()->pushMsgToMsgQueue(msg, currId))
		retCode = EXCEPT_ERR;
	DbgLog("%s channel msg insert redis end", msg.smsgid().c_str());
	sendChatAck(msg, sessionId, retCode);

	// 获取到频道里面的成员
	std::vector<string> vecMember;

	DbgLog("%s channel msg get radio member list begin", msg.smsgid().c_str());
//	string strCode = "";
//	if(!CRedisMgr::getInstance()->getChannelMemberList(m_chnnMemListUrl, m_sAppSecret, msg.sradioid(), vecMember, strCode))
//	{
//		ErrLog("get channel member list fail! code= %s", strCode.c_str());
//		return;
//	}

	if(!CRedisMgr::getInstance()->getChannelUser(msg.sradioid(), vecMember) || vecMember.empty())
		ErrLog("get channel member list fail! radioid= %s", msg.sradioid().c_str());

	DbgLog("%s channel msg get radio member list end", msg.smsgid().c_str());
	std::vector<string> tmpMember;
	int nTick = 0;
	for(auto& itor : vecMember)
	{
		nTick++;
		tmpMember.emplace_back(itor);
		if(nTick >= ProcessCount)
		{
			CThreadPoolManager::getInstance()->getSendGroupMsgPool()->add_task(&CChannelChat::sendChannelChat, this, msg, tmpMember);
			tmpMember.clear();
			nTick = 0;
		}
	}
	if(nTick > 0)
		CThreadPoolManager::getInstance()->getSendGroupMsgPool()->add_task(&CChannelChat::sendChannelChat, this, msg, tmpMember);
	
	vecMember.clear();
	nTick = 0;
	tmpMember.clear();
	// 离线推送
	if(!CRedisMgr::getInstance()->getChannelOfflineUser(msg.sradioid(), vecMember) || vecMember.empty())
		ErrLog("get channel offline member list fail! radioid= %s", msg.sradioid().c_str());
	
	for(auto& itor : vecMember)
	{
		nTick++;
		tmpMember.emplace_back(itor);
		if(nTick >= ProcessCount)
		{
			CThreadPoolManager::getInstance()->getSendGroupMsgPool()->add_task(&CChannelChat::pushChannelChat, this, msg.sfromid(), msg.smsgid(), msg.sradioid(), tmpMember);
			tmpMember.clear();
			nTick = 0;
		}
	}
	if(nTick > 0)
		CThreadPoolManager::getInstance()->getSendGroupMsgPool()->add_task(&CChannelChat::pushChannelChat, this,  msg.sfromid(), msg.smsgid(), msg.sradioid(), tmpMember);

	vecMember.clear();
	nTick = 0;
	tmpMember.clear();
}


void CChannelChat::sendChannelChat(im::RadioChat msg, std::vector<string> vecMember)
{
	DbgLog("send channel msg to member! useId:%s, msgId:%s, radioId:%s", msg.sfromid().c_str(), msg.smsgid().c_str(), msg.sradioid().c_str());
	DbgLog("%s channel msg send to members begin", msg.smsgid().c_str());
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
		sendReq(&msg, RADIO_CHAT_DELIVER, itor.strIp, itor.nPort);
	}
	DbgLog("%s channel msg send to members end", msg.smsgid().c_str());
}

void CChannelChat::sendChatAck(const RadioChat& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc)
{
	DbgLog("channel msg send ack! useId:%s, msgId:%s, radioId:%s", msg.sfromid().c_str(), msg.smsgid().c_str(), msg.sradioid().c_str());
	im::RadioChatAck radioChatAck;
	radioChatAck.set_smsgid(msg.smsgid());
	radioChatAck.set_sfromid(msg.sfromid());
	radioChatAck.set_sradioid(msg.sradioid());
	radioChatAck.set_msgtime(msg.msgtime());
	radioChatAck.set_errcode(retCode);
	sendAck(&radioChatAck, RADIO_CHAT_ACK, sessionId);
	if (retCode != NON_ERR)
	{
		WarnLog("****send channel chat ack(0x%x) %s to %s, code = 0X%X,time %zd, %s", RADIO_CHAT_ACK,
			radioChatAck.smsgid().c_str(), radioChatAck.sfromid().c_str(), retCode, msg.msgtime(), desc.c_str());
	}
}

bool CChannelChat::OnChannelCancelChat(std::shared_ptr<CImPdu> pPdu)
{
	im::RadioCancelChat msg;
	if(!parseRadioCancelMsg(pPdu, msg))
		return false;
	CThreadPoolManager::getInstance()->getInsertGroupMsgPool()->add_task(&CChannelChat::channelCancelChatHandle, this, msg, pPdu->GetSessionId());
	return true;
}

void CChannelChat::channelCancelChatHandle(const im::RadioCancelChat msg, const UidCode_t sessionId)
{
	cancelChatMsgToDataBase(msg, sessionId, false);
}


bool CChannelChat::OnChannelAdminCancelChat(std::shared_ptr<CImPdu> pPdu)
{
	im::RadioCancelChat msg;
	if(!parseRadioCancelMsg(pPdu, msg))
		return false;
	CThreadPoolManager::getInstance()->getInsertGroupMsgPool()->add_task(&CChannelChat::channelAdminCancelChatHandle, this, msg, pPdu->GetSessionId());
	return true;
}

void CChannelChat::channelAdminCancelChatHandle(const im::RadioCancelChat msg, const UidCode_t sessionId)
{
	if(!checkCancelMsgIsAdmin(msg, sessionId))
		return;
	cancelChatMsgToDataBase(msg, sessionId, true);
}


bool CChannelChat::parseRadioCancelMsg(std::shared_ptr<CImPdu> pPdu, im::RadioCancelChat& msg)
{
	if(nullptr == pPdu)
		return false;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		return false;
	msg.set_msgtime(getCurrentTime());//set server time
	if (msg.sradioid().empty() || msg.sfromid().empty())
	{
		WarnLog("channel chat (0x%x) %s, %s-->%s, grpId or fromId not specified\r\n", pPdu->GetCommandId(), msg.smsgid().c_str(),
			msg.sfromid().c_str(), msg.sradioid().c_str());
		return false;
	}
	return true;
}

bool CChannelChat::checkCancelMsgIsAdmin(const im::RadioCancelChat& msg, const UidCode_t& sessionId)
{
	std::vector<std::string> vecAdmin;
	CRedisMgr::getInstance()->getChannelAdmin(msg.sradioid(), vecAdmin);
	im::ErrCode retCode = std::find(vecAdmin.begin(), vecAdmin.end(), msg.sfromid()) == vecAdmin.end() ? ERR_RADIO_ADMIN : NON_ERR;
	if(NON_ERR != retCode)
	{
		sendCancelChatAck(msg, sessionId, RADIO_ADMIN_CANCEL_CHAT_ACK, retCode);
		return false;
	}
	return true;
}


void CChannelChat::cancelChatMsgToDataBase(const im::RadioCancelChat& msg, const UidCode_t& sessionId, bool isAdmin)
{
	DbgLog("%s channel cancel msg update mongo begin", msg.smsgid().c_str());
	uint16_t cmdId = isAdmin ? RADIO_ADMIN_CANCEL_CHAT_ACK : RADIO_CANCEL_CHAT_ACK;
	string dirCollectionName = "chnn_" + msg.sradioid();
	ChannelMongoOperator::getInstance()->checkAndCreateCollection(dirCollectionName);
	string restMsg = ChannelMongoOperator::getInstance()->GetChannelOfflineMsg(dirCollectionName, msg.smsgid());
	DbgLog("get offline msg %s", restMsg.c_str());
	if(!restMsg.empty())
	{
		// 删除redis中数据
		CRedisMgr::getInstance()->removeMsgFromMsgQueue(msg.sradioid(), restMsg);
		
	}
	if(ChannelMongoOperator::getInstance()->UpdateChannelOfflineMsg(dirCollectionName, msg.smsgid(), msg.extend(), isAdmin) <= 0)
	{
		sendCancelChatAck(msg, sessionId, cmdId, EXCEPT_ERR);
		return;
	}
	DbgLog("%s channel cancel msg update mongo end", msg.smsgid().c_str());
	sendCancelChatAck(msg, sessionId, cmdId, NON_ERR);

	// 分别转发
	std::vector<string> vecMember;
	DbgLog("%s channel msg get radio member list begin", msg.smsgid().c_str());
//	string strCode = "";
//	if(!CRedisMgr::getInstance()->getChannelMemberList(m_chnnMemListUrl, m_sAppSecret, msg.sradioid(), vecMember, strCode))
//	{
//		ErrLog("get channel member list fail! code= %s", strCode.c_str());
//		return;
//	}
	
	if(!CRedisMgr::getInstance()->getChannelUser(msg.sradioid(), vecMember) || vecMember.empty())
		ErrLog("get channel member list fail! radioid= %s", msg.sradioid().c_str());
	
	DbgLog("%s channel msg get radio member list end", msg.smsgid().c_str());
	std::vector<string> tmpMember;
	int nTick = 0;
	for(auto& itor : vecMember)
	{
		nTick++;
		tmpMember.emplace_back(itor);
		if(nTick >= ProcessCount)
		{
			CThreadPoolManager::getInstance()->getSendGroupMsgPool()->add_task(&CChannelChat::sendCancelChatMsg, this, msg, tmpMember, isAdmin);
			tmpMember.clear();
			nTick = 0;
		}
	}
	if(nTick > 0)
		CThreadPoolManager::getInstance()->getSendGroupMsgPool()->add_task(&CChannelChat::sendCancelChatMsg, this, msg, tmpMember, isAdmin);
	
	vecMember.clear();
	nTick = 0;
	tmpMember.clear();
	// 离线推送
	if(!CRedisMgr::getInstance()->getChannelOfflineUser(msg.sradioid(), vecMember) || vecMember.empty())
		ErrLog("get channel offline member list fail! radioid= %s", msg.sradioid().c_str());

	for(auto& itor : vecMember)
	{
		nTick++;
		tmpMember.emplace_back(itor);
		if(nTick >= ProcessCount)
		{
			CThreadPoolManager::getInstance()->getSendGroupMsgPool()->add_task(&CChannelChat::pushChannelChat, this, msg.sfromid(), msg.smsgid(), msg.sradioid(), tmpMember);
			tmpMember.clear();
			nTick = 0;
		}
	}
	if(nTick > 0)
		CThreadPoolManager::getInstance()->getSendGroupMsgPool()->add_task(&CChannelChat::pushChannelChat, this,  msg.sfromid(), msg.smsgid(), msg.sradioid(), tmpMember);

	vecMember.clear();
	nTick = 0;
	tmpMember.clear();
}

void CChannelChat::sendCancelChatMsg(im::RadioCancelChat msg, std::vector<string> vecMember, bool isAdmin)
{
	DbgLog("send cancel chat msg to member! useId:%s, msgId:%s, radioId:%s", msg.sfromid().c_str(), msg.smsgid().c_str(), msg.sradioid().c_str());
	DbgLog("%s cancel chat msg send to members begin", msg.smsgid().c_str());
	std::vector<USER_LOGIN_INIFO_> userInfoList;
	if(!CRedisMgr::getInstance()->getUserLoginInfoList(vecMember, userInfoList))
	{
		ErrLog("get user login info fail! msgId:%s, radioId:%s", msg.smsgid().c_str(), msg.sradioid().c_str());
		return;
	}

	uint16_t cmdId = isAdmin ? RADIO_ADMIN_CANCEL_CHAT_DELIVER : RADIO_CANCEL_CHAT_DELIVER;
	
	for(auto& itor : userInfoList)
	{
		if(0 == itor.status || itor.userId == msg.sfromid())
			continue;
		msg.set_stoid(itor.userId);
		sendReq(&msg, cmdId, itor.strIp, itor.nPort);
	}
	DbgLog("%s cancel chat msg send to members end", msg.smsgid().c_str());
}

void CChannelChat::sendCancelChatAck(const im::RadioCancelChat& msg, const UidCode_t& sessionId, uint16_t cmdId, im::ErrCode retCode)
{
	DbgLog("channel cancel chat msg send ack! useId:%s, msgId:%s, radioId:%s", msg.sfromid().c_str(), msg.smsgid().c_str(), msg.sradioid().c_str());
	im::RadioChatAck radioChatAck;
	radioChatAck.set_smsgid(msg.smsgid());
	radioChatAck.set_sfromid(msg.sfromid());
	radioChatAck.set_sradioid(msg.sradioid());
	radioChatAck.set_msgtime(msg.msgtime());
	radioChatAck.set_errcode(retCode);
	sendAck(&radioChatAck, cmdId, sessionId);
	if (retCode != NON_ERR)
	{
		WarnLog("****send channel chat ack(0x%x) %s to %s, code = 0X%X,time %zd", cmdId,
			radioChatAck.smsgid().c_str(), radioChatAck.sfromid().c_str(), retCode, msg.msgtime());
	}
}

void CChannelChat::pushChannelChat(const string& fromId, const string& msgId, const string& radioId, std::vector<string> vecMember)
{
	DbgLog("push channel msg to member! useId:%s, msgId:%s, radioId:%s", fromId.c_str(), msgId.c_str(), radioId.c_str());
	DbgLog("%s channel msg push to members begin", msgId.c_str());
	std::vector<USER_LOGIN_INIFO_> userInfoList;
	if(!CRedisMgr::getInstance()->getUserLoginInfoList(vecMember, userInfoList))
	{
		return;
	}
	
	for(auto& itor : userInfoList)
	{
		if(itor.pushToken.empty() || 1 == itor.status || itor.userId == fromId)
			continue;
		sendPush(fromId, itor.userId, msgId, CHNN_TALK, NewMsgStr, itor.nPushType, itor.pushToken);
	}
	DbgLog("%s channel msg push to members end", msgId.c_str());
}



bool CChannelChat::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(im::RADIO_CHAT, m_nNumberOfInst, CommandProc(&CChannelChat::OnChannelChat));
	CmdRegist(im::RADIO_ADMIN_CANCEL_CHAT, m_nNumberOfInst, CommandProc(&CChannelChat::OnChannelAdminCancelChat));
	CmdRegist(im::RADIO_CANCEL_CHAT, m_nNumberOfInst, CommandProc(&CChannelChat::OnChannelCancelChat));
	
	return true;
}

