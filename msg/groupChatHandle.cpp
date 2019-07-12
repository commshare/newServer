#include"groupChatHandle.h"

#include "im.pub.pb.h"
#include "im.mes.pb.h"
//#include "im.group.pb.h"
#include "im.push.android.pb.h"
#include "util.h"
#include "im_time.h"
#include "redisLoginInfoMgr.h"
#include "im_loginInfo.h"
#include "commonTaskMgr.h"
#include "thread_pool_manager.h"

CGroupChatHandle::CGroupChatHandle(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader)
	, m_nNumberOfInst(nNumOfInst)
{

}

CGroupChatHandle::~CGroupChatHandle()
{

}


bool IsGrpIdValid(const string& grpId)
{
	const int len = grpId.length();
	if (len < 20 || len > 40) return false;
	int digitCount = 0;
	for (int i = 0; i < len; ++i)
	{
		const char& currentChar = grpId[i];
		if ((currentChar >= 'a' && currentChar <= 'z') || (currentChar >= 'A' && currentChar <= 'Z'))
			continue;
		else if (currentChar >= '0' && currentChar <= '9')
			++digitCount;
		else
			return false;
	}
	return digitCount > 0;
}

bool CGroupChatHandle::RegistPacketExecutor(void)
{
	CmdRegist(im::GROUP_CHAT, m_nNumberOfInst,  CommandProc(&CGroupChatHandle::OnGrpMsgChat));
	CmdRegist(im::MES_GRPCHAT_DELIVER_ACK, m_nNumberOfInst,  CommandProc(&CGroupChatHandle::OnGrpMsgChatDeliverAck));
	CmdRegist(im::GROUP_CHATCANCEL, m_nNumberOfInst, CommandProc(&CGroupChatHandle::OnGrpMsgChatCancel));
	CmdRegist(im::MES_CHATCANCEL_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CGroupChatHandle::OnGrpMsgChatCancelDeliverAck));
	return true;
}

void GrpChatMsgInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, unsigned short bInsertSuccess, void* paras)
{
	if (NULL == paras) return;
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CGroupChatHandle* pHandle = (CGroupChatHandle*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}
	const std::shared_ptr<CGrpOfflineMsg> pMsg = dynamic_pointer_cast<CGrpOfflineMsg>(pOfflineMsg);
	pHandle->OnGrpChatInserted(*pMsg, bInsertSuccess, pCallBackPara->m_sessionID);

	delete pCallBackPara;		//回调函数负责释放资源
}

void CGroupChatHandle::OnGrpMsgChat(std::shared_ptr<CImPdu> pPdu)
{
	im::MESGrpChat msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		return ;
	msg.set_msgtime(getCurrentTime());//set server time

	if (msg.sgrpid().empty() || msg.sfromid().empty() || msg.smsgid().empty())
	{
		WarnLog("command_id(0x%x) %s, %s-->%s, grpId or fromId not specified", pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sgrpid().c_str());
		return ;
	}
	DbgLog("recv group msg! msgid=%s groupId=%s fromId=%s", msg.smsgid().c_str(), msg.sgrpid().c_str(), msg.sfromid().c_str());
	if (!IsGrpIdValid(msg.sgrpid()))
	{
		WarnLog("command_id(0x%x) %s, %s-->%s, invalid grpId specified", pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sgrpid().c_str());
		return ;
	}
	
	im::ErrCode retCode = msg.scontent().empty() ? im::ERR_CHAT_UNHEALTHY : im::NON_ERR;
	if (retCode != NON_ERR)
	{
		ErrLog("msg content is null %s", msg.scontent().c_str());
		sendChatAck(msg, pPdu->GetSessionId(), retCode, "content is illegal");
		return ;
	}

	uint64_t nowTick = getCurrentTime_usec();
	GROUP_INFO_ grpInfo;
	string strCode = "";
	if(m_dataInterface.getGroupInfo(m_grpInfoUrl, m_sAppSecret, msg.sgrpid(), grpInfo, strCode))
	{
		if(GRP_NORMAL != grpInfo.status)
		{
			sendChatAck(msg, pPdu->GetSessionId(), im::ERR_CHAT_FORBIDDEN, "content is illegal");
			ErrLog("group is dismiss! grp_id=%s", msg.sgrpid().c_str());
			return;
		}
	}
	else
	{
		sendChatAck(msg, pPdu->GetSessionId(), im::ERR_GROUP_NETWORKEXCEPTION, "request php fail!");
		ErrLog("get group info from http fail grp_id=%s", msg.sgrpid().c_str());
		return;
	}
	DbgLog("msgid=%s, get group info time %llu", msg.smsgid().c_str(), getCurrentTime_usec() - nowTick);

	DbgLog("insertOffline group begin add task! msgId=%s, groupId=%s",msg.smsgid().c_str(), msg.sgrpid().c_str());	
	m_grpOfflineMsgMgr.InsertGrpOfflineMsg(CGrpOfflineMsg(msg), new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()), GrpChatMsgInsertedCallBack);
}

bool CGroupChatHandle::OnGrpChatInserted(const CGrpOfflineMsg& offlineMsg, unsigned short bInsertSuccess, const UidCode_t& sessionID)
{
	DbgLog("insertOffline group end handle task。msgId=%s, groupId=%s", offlineMsg.GetMsgId().c_str(), offlineMsg.GetGrpId().c_str());
	if (!(bInsertSuccess & MONGO_OPERATION_SUCCESS))
	{
		sendChatAck(offlineMsg, sessionID, im::EXCEPT_ERR, "insert failed");
		return false;
	}
	else
	{
		if(bInsertSuccess & MONGO_OPERATION_REPLACE_ONE)
		{
			sendChatAck(offlineMsg, sessionID, im::NON_ERR, "repeat msg");
			return false;
		}
	}
	
	uint64_t nowTick = getCurrentTime_usec();
	MAP_GRP_MEMBER_INFO mapMemberInfo;
	CGetDataInterfaceManager dataInterface;
	string strCode = "";
	if(!dataInterface.getGroupMemberInfoList(m_grpMemberInfoUrl, m_sAppSecret, offlineMsg.GetGrpId(), mapMemberInfo, strCode))
	{
		sendChatAck(offlineMsg, sessionID, im::ERR_GROUP_NETWORKEXCEPTION, "request php fail!");
		ErrLog("get group member list fail");
		return false;
	}
	DbgLog("Thread[%lu] get group member list time %llu! groupId=%s msgId=%s",pthread_self(), getCurrentTime_usec() - nowTick, offlineMsg.GetGrpId().c_str(), offlineMsg.GetMsgId().c_str());
	auto itor = mapMemberInfo.find(offlineMsg.GetFromId());
	if(itor == mapMemberInfo.end())
	{
		sendChatAck(offlineMsg, sessionID, ERR_CHAT_FORBIDDEN, "not grp mem");
		m_grpOfflineMsgMgr.DelOfflineMsg(offlineMsg.GetGrpId(), offlineMsg.GetMsgId(), NULL, NULL);
		return false;
	}
	sendChatAck(offlineMsg, sessionID, NON_ERR, "");
	
	im::MESGrpChat msg;
	msg.ParseFromString(offlineMsg.GetMsgData());
	CThreadPoolManager::getInstance()->getGroupMsgSendPool()->add_task(&CGroupChatHandle::sendGroupMsg, this,  msg, mapMemberInfo);
//	sendGroupMsg(msg, mapMemberInfo);
	DbgLog("process msg[%s] end",msg.smsgid().c_str());
	return true;
}

void CGroupChatHandle::sendGroupMsg(const im::MESGrpChat msg, const MAP_GRP_MEMBER_INFO mapMemberInfo)
{
	std::vector<im::MESGrpChat> deliverToGrpMemMsgs;
	
	im::MESGrpChat deliverMsg;
	deliverMsg.set_sfromid(msg.sfromid());
	deliverMsg.set_sgrpid(msg.sgrpid());
	if (msg.nnotifycount())
	{
		deliverMsg.set_nnotifycount(msg.nnotifycount());
	}
	deliverMsg.set_smsgid(msg.smsgid());
	deliverMsg.set_msgtime(msg.msgtime());

//	if (msg.msgtype())
//	{
//		deliverMsg.set_msgtype(msg.msgtype());
//	}
	
	if (msg.encrypt())
	{
		deliverMsg.set_encrypt(msg.encrypt());
	}
	deliverMsg.set_scontent(msg.scontent());
	deliverMsg.set_extend(msg.extend());
	for (int i = 0; i < msg.snotifyusers_size();++i)
	{
		deliverMsg.add_snotifyusers(msg.snotifyusers(i));
	}
	for(auto& itor : mapMemberInfo)
	{
		if(msg.sfromid().compare(itor.first) == 0) // 不需要发送给发送者
			continue;
		deliverMsg.set_stoid(itor.first);
		deliverToGrpMemMsgs.push_back(deliverMsg);
	}

	
	std::vector<COfflineMsg> offlineMsgs;
	const int nBatch = 20;
	int nCount = 0;
	for(auto& itor : deliverToGrpMemMsgs)
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(itor.stoid());
		if(nullptr != pLogin && pLogin->IsLogin())
		{
			sendReq(&itor, MES_GRPCHAT_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			DbgLog("****from_id %s send MES_GRPCHAT_DELIVER(0x%x) %s to %s", msg.sfromid().c_str(), im::MES_GRPCHAT_DELIVER, itor.smsgid().c_str(), itor.stoid().c_str());
			//在线用户直接发送消息，并记录起来放到后边存库，防止网络传输出问题，拉离线可以把消息拿到
			offlineMsgs.push_back(COfflineMsg(itor));
			nCount++;
			if(nCount >= nBatch)
			{
				nCount = 0;
				if(!m_offlineMsgMgr.InsertOfflineMsg(offlineMsgs))
					WarnLog("insert deliever msgs for grp msg %s failed", msg.smsgid().c_str());
				offlineMsgs.clear();
			}
		}
		else
		{
			if(m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(itor)))
			{
				if(nullptr == pLogin)
					continue;
				auto item = mapMemberInfo.find(itor.stoid());
				if(item == mapMemberInfo.end())
					continue;
				bool bPush = item->second.newMsg;
				if(item->second.newMsg)
				{
					bPush = !item->second.undisturb;
					if(!item->second.undisturb && item->second.isHide)
					{
						bPush = item->second.hideMsgSoundOn;
					}
				}
				DbgLog("user %s bPush = %d", itor.stoid().c_str(), bPush);
				if(bPush && checkDeviceLastUser(itor.stoid(), pLogin->GetDeviceToken()))
				{
					sendPush(msg.sfromid(), itor.stoid(), msg.smsgid(), im::GROUP_TALK, NewMsgStr, item->second.pushType, item->second.pushToken, item->second.voipToken, item->second.versionCode);
					DbgLog("****from_id %s push MES_GRPCHAT_DELIVER(0x%x) %s to %s  group %s", msg.sfromid().c_str(), im::MES_GRPCHAT_DELIVER,  msg.smsgid().c_str(), itor.stoid().c_str(), msg.sgrpid().c_str());
				}
			}
			else
			{
				ErrLog("insert msg fail! msgid=%s, grpid=%s, toid=%s", msg.smsgid().c_str(), msg.sgrpid().c_str(), itor.stoid().c_str());
			}
		}
	}
	
	if(offlineMsgs.size() > 0)
	{
		if(!m_offlineMsgMgr.InsertOfflineMsg(offlineMsgs))
			WarnLog("insert deliever msgs for grp msg %s failed", msg.smsgid().c_str());
		offlineMsgs.clear();
	}

	log("_MESGrpInterChat %s finish, dispatch to %d ",  msg.smsgid().c_str(), mapMemberInfo.size());
}

void CGroupChatHandle::OnGrpMsgChatDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	im::MESGrpChatDeliveredAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		return;
	DbgLog("MESGrpChatDeliveredAck(0x%x) %s received,dir %s --> %s", pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sgrpid().c_str());
	m_offlineMsgMgr.DelOfflineMsg(msg.sfromid(), im::MES_GRPCHAT_DELIVER, msg.smsgid(), NULL, NULL);
}

void OnGroupChatMsgCancelStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pChatTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CGroupChatHandle* pHandle = (CGroupChatHandle*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::GroupChatCancel> pChat = dynamic_pointer_cast<im::GroupChatCancel>(pChatTask);
	pHandle->HandleMsgChatCancelTask(*pChat, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}

void CGroupChatHandle::OnGrpMsgChatCancel(std::shared_ptr<CImPdu> pPdu)
{
	im::GroupChatCancel msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		return;
	if (msg.sgroupid().empty() || msg.sfromid().empty() || msg.smsgid().empty())
	{
		WarnLog("grpChatCancel (0x%x) %s, %s-->%s, grpId or fromId not specified\r\n", pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sgroupid().c_str());
		return;
	}
	if (!IsGrpIdValid(msg.sgroupid()))
	{
		WarnLog("grpChatCancle (0x%x) %s, %s-->%s, invalid grpId specified\r\n", pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sgroupid().c_str());
		return;
	}
	std::shared_ptr<im::GroupChatCancel> pMsg(new im::GroupChatCancel(msg));
	CCommonTaskMgr::InsertCommonTask(pMsg, OnGroupChatMsgCancelStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));
	
	log("grpChatCancel (0x%x) %s pre handled, %s-->%s", pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sgroupid().c_str());
}

bool CGroupChatHandle::HandleMsgChatCancelTask(const GroupChatCancel& msg, const UidCode_t& sessionID)
{
	DbgLog("process handle begin msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
	uint64_t nowTick = getCurrentTime_usec();
	MAP_GRP_MEMBER_INFO mapMemberInfo;
	CGetDataInterfaceManager dataInterface;
	string strCode = "";
	if(!dataInterface.getGroupMemberInfoList(m_grpMemberInfoUrl, m_sAppSecret, msg.sgroupid(), mapMemberInfo, strCode))
	{
		ErrLog("get group member list fail");
		return false;
	}
	DbgLog("Thread[%lu] get group member list time %llu",pthread_self(), getCurrentTime_usec() - nowTick);
	auto itor = mapMemberInfo.find(msg.sfromid());
	if(itor == mapMemberInfo.end())
	{
		sendChatCancleAck(msg, sessionID, ERR_CHAT_FORBIDDEN, "not grp mem");
		return false;
	}
	sendChatCancleAck(msg, sessionID, NON_ERR, "");
	
	sendGroupCancelMsg(msg, mapMemberInfo);
	
	log("thread %lu handle grpchat cancel msg %s get grp %s member", pthread_self(), msg.smsgid().c_str(), msg.sgroupid().c_str());
	DbgLog("process handle end msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
}

void CGroupChatHandle::sendGroupCancelMsg(const im::GroupChatCancel& msg, const MAP_GRP_MEMBER_INFO& mapMemberInfo)
{
	im::MESChatCancel deliverMsg;
	deliverMsg.set_sfromid(msg.sfromid());
	deliverMsg.set_sgroupid(msg.sgroupid());
	deliverMsg.set_smsgid(msg.smsgid());
	deliverMsg.set_msgtime(msg.msgtime());
	deliverMsg.set_ncanceltype(1);
	
	vector<im::MESChatCancel> deliverToGrpMemMsgs;
	vector<COfflineMsg> offlineMsgs;
	vector<string> toMemIds;
	for(auto& itor : mapMemberInfo)
	{
		toMemIds.push_back(itor.first);
		deliverMsg.set_stoid(itor.first);
		deliverMsg.set_sendstate(0);
		if(m_offlineMsgMgr.getOfflineMsgPullStatus(itor.first, im::MES_GRPCHAT_DELIVER, msg.smsgid()))
			deliverMsg.set_sendstate(1);
		offlineMsgs.push_back(COfflineMsg(deliverMsg));
		DbgLog("msg state id=%s state=%d toid=%s", msg.smsgid().c_str(), deliverMsg.sendstate(), deliverMsg.stoid().c_str());	
		if(msg.sfromid().compare(itor.first) == 0) // 不需要发送给发送者
			continue;
		deliverToGrpMemMsgs.push_back(deliverMsg);
	}

	for (size_t i = 0; i < deliverToGrpMemMsgs.size(); ++i)
	{
		im::MESChatCancel msgGrpChatCancel = deliverToGrpMemMsgs[i];
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msgGrpChatCancel.stoid());
		if (pLogin && pLogin->IsLogin())	//用户在线直接尝试推送消息
		{
			sendReq(&msgGrpChatCancel, MES_CHATCANCEL_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			DbgLog("****send MES_CHATCANCEL_DELIVER(0x%x) %s to %s", MES_CHATCANCEL_DELIVER, deliverMsg.smsgid().c_str(), deliverMsg.stoid().c_str());
		}
	}
	
	m_offlineMsgMgr.DelOfflineMsg(toMemIds, im::MES_GRPCHAT_DELIVER, msg.smsgid());
	bool isInsertSuccess = m_offlineMsgMgr.InsertOfflineMsg(offlineMsgs);
	log("group cancel chat %s finish! dispatch to %d mems", msg.smsgid().c_str(), mapMemberInfo.size());
}

void CGroupChatHandle::OnGrpMsgChatCancelDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	MESChatCancelAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		return;
	DbgLog("MESGrpChatCancleDeliveredAck(0x%x) %s received,dir %s", pPdu->GetCommandId(), msg.smsgid().c_str(), msg.suserid().c_str());

	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), im::MES_CHATCANCEL_DELIVER, msg.smsgid(), NULL, NULL);
}

void CGroupChatHandle::sendChatAck(const im::MESGrpChat& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc)
{
	GroupChatAck grpChatAck;
	grpChatAck.set_smsgid(msg.smsgid());
	grpChatAck.set_sfromid(msg.sfromid());
	grpChatAck.set_sgroupid(msg.sgrpid());
	grpChatAck.set_sendtime(msg.msgtime());
	grpChatAck.set_errcode(retCode);
	sendAck(&grpChatAck, GROUP_CHAT_ACK, sessionId);
	if (retCode != NON_ERR)
	{
		WarnLog("****send GroupChatAck(0x%x) %s to %s, code = 0X%X,time %zd, %s", GROUP_CHAT_ACK,
			grpChatAck.smsgid().c_str(), grpChatAck.sfromid().c_str(), retCode, msg.msgtime(), desc.c_str());
	}
}

void CGroupChatHandle::sendChatAck(const CGrpOfflineMsg& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc)
{
	GroupChatAck grpChatAck;
	grpChatAck.set_smsgid(msg.GetMsgId());
	grpChatAck.set_sfromid(msg.GetFromId());
	grpChatAck.set_sgroupid(msg.GetGrpId());
	grpChatAck.set_sendtime(msg.GetCreateTime());
	grpChatAck.set_errcode(retCode);
	sendAck(&grpChatAck, GROUP_CHAT_ACK, sessionId);
	if (retCode != NON_ERR)
	{
		WarnLog("****send GroupChatAck(0x%x) %s to %s, code = 0X%X,time %zd, %s", GROUP_CHAT_ACK,
			grpChatAck.smsgid().c_str(), grpChatAck.sfromid().c_str(), retCode, msg.GetCreateTime(), desc.c_str());
	}
}

void CGroupChatHandle::sendChatCancleAck(const im::GroupChatCancel& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc /*= string("")*/)
{
	im::GroupChatCancelAck grpChatCancelAck;
	grpChatCancelAck.set_smsgid(msg.smsgid());
	grpChatCancelAck.set_sfromid(msg.sfromid());
	grpChatCancelAck.set_sgroupid(msg.sgroupid());
	grpChatCancelAck.set_errcode(retCode);
	sendAck(&grpChatCancelAck, GROUP_CHATCANCEL_ACK, sessionId);
	if (retCode != NON_ERR)
	{
		WarnLog("****send GroupChatCancelAck(0x%x) %s to %s, code = 0X%X,time %zd, %s", GROUP_CHATCANCEL_ACK,
			grpChatCancelAck.smsgid().c_str(), grpChatCancelAck.sfromid().c_str(), retCode, getCurrentTime(), desc.c_str());
	}
}

