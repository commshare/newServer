#include "configfilereader.h"
#include "friendhandle.h"
#include "im_loginInfo.h"
#include "im.cm.pb.h"
#include "im.mes.pb.h"
#include "im.pub.pb.h"
//#include "im.group.pb.h"
#include "im.pushSvrAPNsMsg.pb.h"
#include "mysqlFriendMgr.h"
#include "redisFriendMgr.h"
#include "redisLoginInfoMgr.h"
#include "redisUserCfgMgr.h"
#include "serverinfo.h"
#include "util.h"
#include "commonTaskMgr.h"
#include "notify_handle.h"

using namespace im;
using namespace std;

CNotifyHandle::CNotifyHandle(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader), m_nNumberOfInst(nNumOfInst)
{

}

CNotifyHandle::~CNotifyHandle()
{
	
}

bool CNotifyHandle::RegistPacketExecutor(void)
{
	CmdRegist(SVR_GROUP_RELATIN_NOTIFY, m_nNumberOfInst,  CommandProc(&CNotifyHandle::groupRelationNotify));
	CmdRegist(SVR_FRIEND_RELATION_NOTIFY, m_nNumberOfInst,  CommandProc(&CNotifyHandle::friendRelationNotify));
	CmdRegist(SVR_COMMON_MSG_NOTIFY, m_nNumberOfInst,  CommandProc(&CNotifyHandle::commonMsgNotify));

	CmdRegist(MES_ADDFRIEND_DELIVER_ACK, m_nNumberOfInst,  CommandProc(&CNotifyHandle::OnAddFriendDeliverAck));
	CmdRegist(MES_ADDFRIEND_ANS_DELIVER_ACK, m_nNumberOfInst,  CommandProc(&CNotifyHandle::OnAddFriendAnsDeliverAck));
	CmdRegist(MS_COMMONNOTIFY_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CNotifyHandle::OnCommonNotifyMsgDeliverAck));
	CmdRegist(MES_GRPNOTIFY_DELIVER_ACK, m_nNumberOfInst,  CommandProc(&CNotifyHandle::OnGrpNotifyDeliverAck));
	CmdRegist(MES_JOINGRP_DELIVER_ACK, m_nNumberOfInst,  CommandProc(&CNotifyHandle::OnJoinGrpDeliverAck));
	return true;
}


void CNotifyHandle::groupRelationNotify(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SVRMSGGroupRelationNotify groupNotify;
	if (!pPdu->GetBodyData() || !groupNotify.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("group relation msg=%s groupId=%s type=%d extend=%s", groupNotify.smsgid().c_str(), groupNotify.sgrpid().c_str(), groupNotify.notifytype(), groupNotify.extend().c_str());
	if(GRPRELATION_TYPE_MASTER_ANS_APPLY == groupNotify.notifytype() || GRPRELATION_TYPE_MASTER_ANS_INVITE == groupNotify.notifytype())	// 加群需要同意 或 拉人需要同意
	{
		OnJoinGrp(groupNotify, pPdu->GetSessionId());
	}
	else
	{
		OnGrpNotify(groupNotify, pPdu->GetSessionId());
	}

	SVRMSGNotifyACK notifyAck;
	notifyAck.set_smsgid(groupNotify.smsgid());
	notifyAck.set_msgtime(getCurrentTime());
	notifyAck.set_errcode(im::NON_ERR);
	sendAck(&notifyAck, im::SVR_GROUP_RELATIN_NOTIFY_ACK, pPdu->GetSessionId());
}

void CNotifyHandle::friendRelationNotify(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SVRMSGFriendRelationNotify friendNotify;
	if (!pPdu->GetBodyData() || !friendNotify.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("friend relation msg=%s fromId=%s toId=%s type=%d extend=%s", friendNotify.smsgid().c_str(), friendNotify.sfromid().c_str(), friendNotify.stoid().c_str(), friendNotify.notifytype(), friendNotify.extend().c_str());
	if(FRIEND_NOTIFY_TYPE_ADD == friendNotify.notifytype()) // 加好友
	{
		AddFriendHandle(friendNotify, pPdu->GetSessionId());
	}
	else if(FRIEND_NOTIFY_TYPE_ANS == friendNotify.notifytype()) // 好友同意
	{
		AddFriendAnsHandle(friendNotify, pPdu->GetSessionId());
	}

	SVRMSGNotifyACK notifyAck;
	notifyAck.set_smsgid(friendNotify.smsgid());
	notifyAck.set_msgtime(getCurrentTime());
	notifyAck.set_errcode(im::NON_ERR);
	sendAck(&notifyAck, im::SVR_FRIEND_RELATION_NOTIFY_ACK, pPdu->GetSessionId());
	
}

void CNotifyHandle::commonMsgNotify(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SVRMSGCommonMsgNotify commNotify;
	if (!pPdu->GetBodyData() || !commNotify.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("common notify msg=%s fromId=%s extend=%s", commNotify.smsgid().c_str(), commNotify.sfromid().c_str(), commNotify.extend().c_str());
	CommonMsgNotifyHandle(commNotify, pPdu->GetSessionId());

	SVRMSGNotifyACK notifyAck;
	notifyAck.set_smsgid(commNotify.smsgid());
	notifyAck.set_msgtime(getCurrentTime());
	notifyAck.set_errcode(im::NON_ERR);
	sendAck(&notifyAck, im::SVR_COMMON_MSG_NOTIFY_ACK, pPdu->GetSessionId());
}

void JoinGrpDeliverInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, unsigned short result, void* paras)
{
	if (NULL == paras) return;
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CNotifyHandle* pHandle = (CNotifyHandle*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}
	const std::shared_ptr<COfflineMsg> pMsg = dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
	pHandle->OnJoinGrpDeliverInserted(*pMsg, result, pCallBackPara->m_sessionID, pCallBackPara->m_bNeedSendPush);

	delete pCallBackPara;		//回调函数负责释放资源
}


void CNotifyHandle::OnJoinGrp(const im::SVRMSGGroupRelationNotify& groupNotify, const UidCode_t& sessionID)
{
	im::MESJoinGrp joinGroup;
	joinGroup.set_sfromid(groupNotify.sopruserid());
	joinGroup.set_sgrpid(groupNotify.sgrpid());
	joinGroup.set_smsgid(groupNotify.smsgid());
	joinGroup.set_msgtime(groupNotify.msgtime());
	joinGroup.set_extend(groupNotify.extend());
	joinGroup.set_soperid(groupNotify.soperid());

	auto sendMsg = [this](im::MESJoinGrp& joinGroup, const UidCode_t& sessionID)
	{
		//if receiver was online, send addFriendDeliver to receiver
		bool bneedSendPush = true;
		DbgLog("join group to_id %s", joinGroup.stoid().c_str());
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(joinGroup.stoid());
		if (pLogin && pLogin->IsLogin())
		{
			sendReq(&joinGroup, MES_JOINGRP_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			bneedSendPush = false;
			log("****send MES_JOINGRP_DELIVER(0x%x) %s to %s", MES_JOINGRP_DELIVER, joinGroup.smsgid().c_str(), joinGroup.stoid().c_str());
		}
		//save addFriendDeliver msg to mongodb, if insert failed ,can try to send deliver
		m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(joinGroup), JoinGrpDeliverInsertedCallBack, new OfflineMsgInsertCallBackParas_t(this, sessionID, bneedSendPush));
	};
	
	DbgLog("join group type %d", groupNotify.notifytype());
	if(groupNotify.notifytype() == im::GRPRELATION_TYPE_MASTER_ANS_APPLY)
	{
		// 获取master （php）
		GROUP_INFO_ grpInfo;
		string strCode = "";
		if(!m_dataInterface.getGroupInfo(m_grpInfoUrl, m_sAppSecret, groupNotify.sgrpid(), grpInfo, strCode))
		{
			ErrLog("get group info from http fail grp_id=%s", groupNotify.sgrpid().c_str());
			return;
		}
		joinGroup.set_stoid(grpInfo.masterId);
		joinGroup.set_reqtype(0);
		sendMsg(joinGroup, sessionID);
	}
	else if(groupNotify.notifytype() == im::GRPRELATION_TYPE_MASTER_ANS_INVITE)
	{
		if(groupNotify.smnpleduserid_size() <= 0)
			ErrLog("join group mnpled userid is empty");
		joinGroup.set_reqtype(1);
		for(int i = 0; i < groupNotify.smnpleduserid_size(); ++i)
		{
			joinGroup.set_stoid(groupNotify.smnpleduserid(i));
			sendMsg(joinGroup, sessionID);
		}
	}
}

void CNotifyHandle::OnJoinGrpDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush)
{
	if (bNeedSendPush)
	{
		DbgLog("join group deliver insert to_id %s", Msg.GetToId().c_str());
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(Msg.GetToId());
		if (pLogin)
		{
			// 获取权限数据（PHP）
			USER_INFO_ usrInfo;
			string strCode = "";
			CGetDataInterfaceManager dataInterface;
			if(!dataInterface.getUserInfo(m_usrInfoUrl, m_sAppSecret, Msg.GetToId(), usrInfo, strCode))
			{
				ErrLog("get user info from http fail usr_id=%s", Msg.GetToId().c_str());
				return;
			}
			
			if (usrInfo.newMsg && checkDeviceLastUser(Msg.GetToId(), pLogin->GetDeviceToken()))
			{
				sendPush(Msg.GetFromId(), Msg.GetToId(), Msg.GetMsgId(), im::GRP_CONTACTS, NewMsgStr, 
								usrInfo.pushType, usrInfo.pushToken, usrInfo.voipToken, usrInfo.versionCode);
			}
		}
		
	}
}

void CNotifyHandle::OnJoinGrpDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESJoinGrpDeliverAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("JoinGrpDeliverAck(0x%x) %s received", pPdu->GetCommandId()/*MES_JOINGRP_DELIVER_ACK*/, msg.smsgid().c_str());

	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_JOINGRP_DELIVER, msg.smsgid());
}

void OnGrpMsgNotifyStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pChatTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CNotifyHandle* pHandle = (CNotifyHandle*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::SVRMSGGroupRelationNotify> pChat = dynamic_pointer_cast<im::SVRMSGGroupRelationNotify>(pChatTask);

	pHandle->HandleMsgGrpNotifyTask(*pChat, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}


void CNotifyHandle::OnGrpNotify(const im::SVRMSGGroupRelationNotify& groupNotify, const UidCode_t& sessionID)
{
	static int notifyCount = 0;
	++notifyCount;
	log("MESGrpNotify-%d (0x%x) %s received , %s",notifyCount, MES_GRPINTERNOTIFY, groupNotify.smsgid().c_str(), groupNotify.sgrpid().c_str());
	im::SVRMSGGroupRelationNotify* pTmp = new im::SVRMSGGroupRelationNotify(groupNotify);
	std::shared_ptr<im::SVRMSGGroupRelationNotify> ptrNotify = std::shared_ptr<im::SVRMSGGroupRelationNotify>(pTmp);
	DbgLog("process add task msgId=%s",groupNotify.smsgid().c_str());
	bool IsCreateTaskSuccess = CCommonTaskMgr::InsertCommonTask(ptrNotify, OnGrpMsgNotifyStartUp, new OfflineMsgInsertCallBackParas_t(this, sessionID));
}

void GrpNotifyDeliverInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, bool bInsertSuccess, void* paras)
{
	if (NULL == paras) return;
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CNotifyHandle* pHandle = (CNotifyHandle*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}
	const std::shared_ptr<COfflineMsg> pMsg = dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
	pHandle->OnGrpNotifyDeliverInserted(*pMsg, bInsertSuccess, pCallBackPara->m_bNeedSendPush);

	delete pCallBackPara;		//回调函数负责释放资源
}


void CNotifyHandle::HandleMsgGrpNotifyTask(const im::SVRMSGGroupRelationNotify& groupNotify, const UidCode_t& sessionId)
{
	DbgLog("process handle begin msgId=%s thread=%lu", groupNotify.smsgid().c_str(), pthread_self());
	im::MESGrpNotify deliverMsg;
	deliverMsg.set_sgrpid(groupNotify.sgrpid());
	deliverMsg.set_smsgid(groupNotify.smsgid());
	deliverMsg.set_msgtime(groupNotify.msgtime());

	if (!groupNotify.sopruserid().empty())
	{
		deliverMsg.set_sopruserid(groupNotify.sopruserid());
	}
	DbgLog("group notify type is %d group_id is %s msg_id is %s", groupNotify.notifytype(), groupNotify.sgrpid().c_str(), groupNotify.smsgid().c_str());
	deliverMsg.set_scontent(groupNotify.scontent());
	deliverMsg.set_notifytype((NotifyType)groupNotify.notifytype());
	deliverMsg.set_errcode(im::NON_ERR);
	deliverMsg.set_extend(groupNotify.extend());
	
	for (int i = 0; i < groupNotify.smnpleduserid_size(); ++i)
	{
		deliverMsg.add_smnpleduserid(groupNotify.smnpleduserid(i));
	}

	auto sendMsg = [this](std::string strToid, im::MESGrpNotify& deliverMsg, const UidCode_t& sessionId)
	{
		DbgLog("group notify to_id %s", strToid.c_str());
		deliverMsg.set_stoid(strToid);
		//创建插入任务，放到任务队列中
		m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(deliverMsg), GrpNotifyDeliverInsertedCallBack, new OfflineMsgInsertCallBackParas_t(this, sessionId, false));
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(deliverMsg.stoid());
		if (pLogin )	//用户在线直接尝试推送消息
		{
			if (pLogin->IsLogin())
			{
				sendReq(&deliverMsg, MES_GRPNOTIFY_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
				log("****send MES_GRPNOTIFY_DELIVER(0x%x) %s to %s", MES_GRPNOTIFY_DELIVER, deliverMsg.smsgid().c_str(), deliverMsg.stoid().c_str());
			}
		}
	};
	
	DbgLog("group notify size %d, type %d", groupNotify.stoids_size(), groupNotify.notifytype());
	if(groupNotify.stoids_size() <= 0)
	{
		// 查询接口处理（PHP）
		VEC_GRP_MEMBER_INFO vecMemberInfo;
		string strCode = "";
		if(!m_dataInterface.getGroupMemberInfoList(m_grpMemberInfoUrl, m_sAppSecret, groupNotify.sgrpid(), vecMemberInfo, strCode))
		{
			ErrLog("get group member from http fail, group_id=%s", groupNotify.sgrpid().c_str());
			return;
		}
		for(auto itor : vecMemberInfo)
		{
			sendMsg(itor.usrId, deliverMsg, sessionId);
		}
		if(deliverMsg.notifytype() == NOTIFY_TYPE_GRPMEM_REMOVE)		// 踢人
		{
			for (int i = 0; i < groupNotify.smnpleduserid_size(); ++i)
			{
				sendMsg(groupNotify.smnpleduserid(i), deliverMsg, sessionId);
			}
		}
		
	}
	else
	{
		for(int i = 0; i < groupNotify.stoids_size(); ++i)
		{
			//deliverMsg.set_stoid(groupNotify.stoids(i));
			sendMsg(groupNotify.stoids(i), deliverMsg, sessionId);
		}
		
		if(deliverMsg.notifytype() == NOTIFY_TYPE_GRPMEM_REMOVE)		// 踢人
		{
			for (int i = 0; i < groupNotify.smnpleduserid_size(); ++i)
			{
				sendMsg(groupNotify.smnpleduserid(i), deliverMsg, sessionId);
			}
		}
	}

}

void CNotifyHandle::OnGrpNotifyDeliverInserted(const COfflineMsg& Msg, bool bInsertSuccess, bool bNeedSendPush)
{
	if (!bInsertSuccess || !bNeedSendPush) return;

//	std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(Msg.GetToId());
//	if (!pLogin) return;

//	const std::shared_ptr<CUserCfg> pUserCfg = CUserCfgMgr::GetUserCfg(Msg.GetToId());
//	if (!pUserCfg->IsGlobalNoInterruption())
//	{
//		sendPush(pLogin, Msg.GetFromId(), Msg.GetToId(), Msg.GetMsgId(), CONTACTS, NewMsgStr);
//	}
}

void CNotifyHandle::OnGrpNotifyDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESGrpNotifyDeliverAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("MESGrpNotifyDeliverAck(0x%x) %s received,dir %s --> %s",
		pPdu->GetCommandId()/*MES_GRPNOTIFY_DELIVER_ACK*/, msg.smsgid().c_str(), msg.suserid().c_str(), msg.sgrpid().c_str());

	//不要立刻执行，放到任务队列中,跟插入的任务要在同一个任务线程中
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_GRPNOTIFY_DELIVER, msg.smsgid(),NULL,NULL);
}

void AddFriendDeliverInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, unsigned short result, void* paras)
{
	if (NULL == paras) return;
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CNotifyHandle* pHandle = (CNotifyHandle*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}
	const std::shared_ptr<COfflineMsg> pMsg= dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
	pHandle->OnAddFriendDeliverInserted(*pMsg, result, pCallBackPara->m_sessionID, pCallBackPara->m_bNeedSendPush);

	delete pCallBackPara;		//回调函数负责释放资源
}


void CNotifyHandle::AddFriendHandle(const im::SVRMSGFriendRelationNotify& friendNotify, const UidCode_t& sessionID)
{
	im::MESAddFriend addFriend;
	addFriend.set_sfromid(friendNotify.sfromid());
	addFriend.set_stoid(friendNotify.stoid());
	addFriend.set_msgtime(getCurrentTime());
	addFriend.set_smsgid(friendNotify.smsgid());
	addFriend.set_smemoname(friendNotify.smemoname());
	addFriend.set_sselfintroduce(friendNotify.sselfintroduce());
	addFriend.set_soperid(friendNotify.soperid());
	addFriend.set_extend(friendNotify.extend());

	DbgLog("add friend from %s, to %s", friendNotify.sfromid().c_str(), friendNotify.stoid().c_str());
	bool bneedSendPush = true;
	std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(addFriend.stoid());
	if (pLogin && pLogin->IsLogin())
	{
		sendReq(&addFriend, MES_ADDFRIEND_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
		bneedSendPush = false;
		log("****send MES_ADDFRIEND_DELIVER(0x%x) %s to %s sent", MES_ADDFRIEND_DELIVER, addFriend.smsgid().c_str(), addFriend.stoid().c_str());
	}

	//save addFriendDeliver msg to mongodb, if insert failed ,can try to send deliver
	addFriend.set_sdesc("");
	addFriend.set_packetid(-1);
	m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(addFriend), AddFriendDeliverInsertedCallBack, new OfflineMsgInsertCallBackParas_t(this, sessionID, bneedSendPush));
}

void CNotifyHandle::OnAddFriendDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush)
{
	if (bNeedSendPush)
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(Msg.GetToId());
		if (nullptr != pLogin)
		{
			// 需要修改，调PHP接口
			USER_INFO_ usrInfo;
			CGetDataInterfaceManager dataInterface;
			string strCode = "";
			if(!dataInterface.getUserInfo(m_usrInfoUrl, m_sAppSecret, Msg.GetToId(), usrInfo, strCode))
			{
				ErrLog("get user info from http fail usr_id=%s", Msg.GetToId().c_str());
				return;
			}
			
			if (usrInfo.newMsg && checkDeviceLastUser(Msg.GetToId(), pLogin->GetDeviceToken()))
			{
				//你有一条新请求！
				sendPush(Msg.GetFromId(), Msg.GetToId(), Msg.GetMsgId(), im::GRP_CONTACTS, NewMsgStr, 
								usrInfo.pushType, usrInfo.pushToken, usrInfo.voipToken, usrInfo.versionCode);
			}
		}
	}
}

void CNotifyHandle::OnAddFriendDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESAddFriendDeliverAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("addFriendDeliverAck(0x%x) %s received", pPdu->GetCommandId()/*MES_ADDFRIEND_DELIVER_ACK*/, msg.smsgid().c_str());
	if (msg.suserid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}

	//delete addfriendDeliver msg from mongo 
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid() ,MES_ADDFRIEND_DELIVER, msg.smsgid());
}

void AddFriendAnsDeliverInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, unsigned short bInsertSuccess, void* paras)
{
	if (NULL == paras) return;
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CNotifyHandle* pHandle = (CNotifyHandle*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}
	const std::shared_ptr<COfflineMsg> pMsg = dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
	pHandle->OnAddFriendAnsDeliverInserted(*pMsg, bInsertSuccess, pCallBackPara->m_sessionID, pCallBackPara->m_bNeedSendPush);

	delete pCallBackPara;		//回调函数负责释放资源
}

void CNotifyHandle::AddFriendAnsHandle(const im::SVRMSGFriendRelationNotify& friendNotify, const UidCode_t& sessionID)
{
	im::MESAddFriendAns addFriendAns;
	addFriendAns.set_sfromid(friendNotify.sfromid());
	addFriendAns.set_stoid(friendNotify.stoid());
	addFriendAns.set_msgtime(getCurrentTime());
	addFriendAns.set_smsgid(friendNotify.smsgid());
	addFriendAns.set_smemoname(friendNotify.smemoname());
	addFriendAns.set_sans(friendNotify.sselfintroduce());
	addFriendAns.set_extend(friendNotify.extend());

	DbgLog("add friend from %s, to %s", friendNotify.sfromid().c_str(), friendNotify.stoid().c_str());
	bool bneedSendPush = true;		//同意才发
	std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(addFriendAns.stoid());
	if (pLogin && pLogin->IsLogin())	//if receiver was online, send addFriendDeliver to receiver
	{
		sendReq(&addFriendAns, MES_ADDFRIEND_ANS_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
		bneedSendPush = false;
		log("****send MES_ADDFRIEND_ANS_DELIVER(0x%x) %s to %s sent", MES_ADDFRIEND_ANS_DELIVER, addFriendAns.smsgid().c_str(), addFriendAns.stoid().c_str());
	}

	m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(addFriendAns), AddFriendAnsDeliverInsertedCallBack, new OfflineMsgInsertCallBackParas_t(this, sessionID, bneedSendPush));
}

void CNotifyHandle::OnAddFriendAnsDeliverInserted(const COfflineMsg& Msg, unsigned short bInsertSuccess, const UidCode_t& sessionID, bool bNeedSendPush)
{
	bNeedSendPush = bNeedSendPush && ((bInsertSuccess & MONGO_OPERATION_SUCCESS) && !(bInsertSuccess & MONGO_OPERATION_REPLACE_ONE));
	if (bNeedSendPush)
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(Msg.GetToId());
		if (pLogin)
		{
			// 需要修改，调PHP接口
			USER_INFO_ usrInfo;
			string strCode = "";
			CGetDataInterfaceManager dataInterface;
			if(!dataInterface.getUserInfo(m_usrInfoUrl, m_sAppSecret, Msg.GetToId(), usrInfo, strCode))
			{
				ErrLog("get user info from http fail usr_id=%s", Msg.GetToId().c_str());
				return;
			}
			
			if (usrInfo.newMsg && checkDeviceLastUser(Msg.GetToId(), pLogin->GetDeviceToken()))
			{
				sendPush(Msg.GetFromId(), Msg.GetToId(), Msg.GetMsgId(), im::GRP_CONTACTS, NewMsgStr, 
								usrInfo.pushType, usrInfo.pushToken, usrInfo.voipToken, usrInfo.versionCode);
			}
		}	
	}
}

void CNotifyHandle::OnAddFriendAnsDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESAddFriendAnsDeliverACK msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("addFriendAnsDeliverAck(0x%x) %s received", pPdu->GetCommandId()/*MES_ADDFRIEND_ANS_DELIVER_ACK*/, msg.smsgid().c_str());

	if (msg.suserid().empty() || msg.smsgid().empty() )
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_ADDFRIEND_ANS_DELIVER, msg.smsgid());
}

void OnCommonNotifyMsgStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pCommonNotifyMsgTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CNotifyHandle* pHandle = (CNotifyHandle*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::MSGCommonNotify> pChat = dynamic_pointer_cast<im::MSGCommonNotify>(pCommonNotifyMsgTask);
	//const im::MESChat& msg = *pChat;
	
	pHandle->HandleCommonNotifyMsgTask(*pChat, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}

void CNotifyHandle::CommonMsgNotifyHandle(const SVRMSGCommonMsgNotify& commNotify, const UidCode_t& sessionID)
{
	std::shared_ptr<im::MSGCommonNotify> msgNotify = std::make_shared<im::MSGCommonNotify>();
	msgNotify->set_sfromid(commNotify.sfromid());
	msgNotify->set_smsgid(commNotify.smsgid());
	msgNotify->set_scontent(commNotify.scontent());
	msgNotify->set_msgtime(commNotify.msgtime());
	msgNotify->set_notifytype((CommonNotifyType)commNotify.notifytype());
	for (int i = 0; i < commNotify.stoids_size(); i++)
	{
		msgNotify->add_stoids(commNotify.stoids(i));			
	}
	DbgLog("process add task msgId=%s",commNotify.smsgid().c_str());
	CCommonTaskMgr::InsertCommonTask(msgNotify, OnCommonNotifyMsgStartUp, new OfflineMsgInsertCallBackParas_t(this, sessionID));
	log("commonNotifyMsg (0x%x) %s prehandled , %s-->%s \n", im::SVR_COMMON_MSG_NOTIFY, msgNotify->smsgid().c_str(), msgNotify->sfromid().c_str(), msgNotify->stoid().c_str());
}

void CNotifyHandle::HandleCommonNotifyMsgTask(const im::MSGCommonNotify& msg, const UidCode_t& sessionId)
{
	DbgLog("process handle begin msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

	uint64_t checkUserRightTime = elspsedTimer.elapsed();
	std::vector<im::MSGCommonNotify> vecMsgCommNotifyDeliver;
	std::vector<COfflineMsg> vecOfflineMsg;

	im::MSGCommonNotify deliverMsg;
	deliverMsg.set_sfromid(msg.sfromid());
	deliverMsg.set_smsgid(msg.smsgid());
	deliverMsg.set_scontent(msg.scontent());
	deliverMsg.set_notifytype(msg.notifytype());
	deliverMsg.set_msgtime(msg.msgtime());
	for (int i = 0; i < msg.stoids_size();++i)
	{
		std::string strToid = msg.stoids(i);
		// 注释，可不做好友关系判断
//		FRIEND_INFO_ frdInfo;
//		string strCode = "";
//		CGetDataInterfaceManager dataInterface;
//		if(!dataInterface.getFriendInfo(m_friendInfoUrl, msg.sfromid(), strToid, frdInfo, strCode))
//		{
//			ErrLog("get friend info from http fail msgid=%s, usr_id=%s, friend_id=%s", msg.smsgid().c_str(), msg.sfromid().c_str(), strToid.c_str());
//			continue;
//		}
//		else
//		{
//			if(frdInfo.status != IS_NORMAL)
//			{
//				WarnLog("common notify msg %s from %s to %s not friend", msg.smsgid().c_str(), msg.sfromid().c_str(), strToid.c_str());
//				continue;
//			}
//		}
		deliverMsg.set_stoid(strToid);
		vecMsgCommNotifyDeliver.push_back(deliverMsg);
		vecOfflineMsg.push_back(COfflineMsg(deliverMsg));
	}

	bool bInsertSuccess = m_offlineMsgMgr.InsertOfflineMsg(vecOfflineMsg);
	uint64_t insertFinishTime = elspsedTimer.elapsed();

	if (!bInsertSuccess)
	{
		WarnLog("insert delever msgs for grp msg %s failed", msg.smsgid().c_str());
	}
	else
	{
		for(auto& itor : vecMsgCommNotifyDeliver)
		{
			
			std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(itor.stoid());
			if(nullptr == pLogin)
					continue;
										
			if (pLogin->IsLogin())	//用户在线直接尝试推送消息
			{
				sendReq(&itor, MS_COMMONNOTIFY_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			}
		}
	}
	DbgLog("CommonNotifyMsg %s Task handle in thread %llu finish , at %llu usec right checked, %llu mongo insertted, total use use %llu usecond, sessionId %u  \r\n\r\n",
		msg.smsgid().c_str(), pthread_self(), checkUserRightTime, insertFinishTime, elspsedTimer.elapsed(), sessionId);
}


// 收到消息接收方聊天消息推送应答
void CNotifyHandle::OnCommonNotifyMsgDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	std::shared_ptr<im::MSGCommonNotifyACK> pMsg(new im::MSGCommonNotifyACK);
		if (!pMsg) 
			return;
	MSGCommonNotifyACK& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

	if (msg.suserid().empty() || msg.smsgid().empty() )
	{
		ErrLog("!!!MESChatDeliveredAck(0x%x) %s lack of required parameter,dir %s\r\n",
			pPdu->GetCommandId(), msg.smsgid().c_str(), msg.suserid().c_str());
		return;
	}
	
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MS_COMMONNOTIFY_DELIVER, msg.smsgid());
}






