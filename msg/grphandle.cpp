/******************************************************************************
Filename: grphandle.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/29
Description: 
******************************************************************************/
#include "configfilereader.h"
#include "grphandle.h"
#include "im_time.h"
#include "im_loginInfo.h"
#include "im.mes.pb.h"
#include "im.pub.pb.h"
#include "im.push.android.pb.h"
#include "mysqlFriendMgr.h"
#include "mysqlUserMgr.h"
#include "redisFriendMgr.h"
#include "redisLoginInfoMgr.h"
#include "serverinfo.h"
#include "util.h"
#include "commonTaskMgr.h"
#include "redisUserGrpCfgMgr.h"
#include "redisUserCfgMgr.h"

using namespace im;
using namespace std;

CGrpMsgHandler::CGrpMsgHandler(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader), m_nNumberOfInst(nNumOfInst)
{

}

CGrpMsgHandler::~CGrpMsgHandler()
{

}


//消息处理线程,将数据进行分发
void OnGrpMsgChatStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pChatTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CGrpMsgHandler* pHandle = (CGrpMsgHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::_MESGrpInterChat> pChat = dynamic_pointer_cast<im::_MESGrpInterChat>(pChatTask);
	//const im::_MESGrpInterChat& msg = *pChat;

	pHandle->HandleMsgGrpChatTask(*pChat, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}


void CGrpMsgHandler::HandleMsgGrpChatTask(const im::_MESGrpInterChat& msg, const UidCode_t& sessionId)
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

	std::vector<im::MESGrpChat> deliverToGrpMemMsgs;
	std::vector<COfflineMsg> offlineMsgs;

	im::MESGrpChat deliverMsg;
	deliverMsg.set_sfromid(msg.sfromid());
	deliverMsg.set_sgrpid(msg.sgrpid());
	if (msg.nnotifycount())
	{
		deliverMsg.set_nnotifycount(msg.nnotifycount());
	}
	deliverMsg.set_smsgid(msg.smsgid());
	deliverMsg.set_msgtime(msg.msgtime());

	if (msg.msgtype())
	{
		deliverMsg.set_msgtype(msg.msgtype());
	}
	if (msg.encrypt())
	{
		deliverMsg.set_encrypt(msg.encrypt());
	}
	deliverMsg.set_scontent(msg.scontent());
	for (int i = 0; i < msg.snotifyusers_size();++i)
	{
		deliverMsg.add_snotifyusers(msg.snotifyusers(i));
	}


	for (int i = 0; i < msg.stoid_size();++i)
	{
		deliverMsg.set_stoid(msg.stoid(i));
		deliverToGrpMemMsgs.push_back(deliverMsg);
		offlineMsgs.push_back(COfflineMsg(deliverMsg));
	}

	for (size_t i = 0; i < deliverToGrpMemMsgs.size(); i++)
	{
		im::MESGrpChat& mesGrpChat = deliverToGrpMemMsgs[i];
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(mesGrpChat.stoid());
		if (!pLogin) continue;
		if (pLogin->IsLogin())	//用户在线直接尝试推送消息
		{
			sendReq(&mesGrpChat, MES_GRPCHAT_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			DbgLog("****send MES_GRPCHAT_DELIVER(0x%x) %s to %s", MES_GRPCHAT_DELIVER, mesGrpChat.smsgid().c_str(), mesGrpChat.stoid().c_str());
		}
		else
		{
			do{
				string sDeviceId = pLogin->GetDeviceID();
		    string sUserID = CLoginInfoMgr::getDeviceLastUserID(sDeviceId);

			if(!sUserID.empty() && sUserID != mesGrpChat.stoid())  //other user login the same device,no need to push
			{
				DbgLog("Ohter user %s login device %s,no push to user %s ",sUserID.c_str(),sDeviceId.c_str(),mesGrpChat.stoid().c_str());
				break;
			}
			std::shared_ptr<CUserCfg> pUserCfg = CUserCfgMgr::GetUserCfg(mesGrpChat.stoid());
			bool msgNoInterruption = pUserCfg->IsGlobalNoInterruption();
			DbgLog("user %s globalNoInterupt = %d and hideMsgSoundOn = %d", mesGrpChat.stoid().c_str(), pUserCfg->IsGlobalNoInterruption(), pUserCfg->IsHidenMsgSoundOn());
			if (!msgNoInterruption)					//会话是否设置了消息免打搅
			{
				std::shared_ptr<CUserGrpCfg> pUserGrpCfg = CUserGrpCfgMgr::GetUserGrpCfg(mesGrpChat.stoid(), mesGrpChat.sgrpid());
				DbgLog("grp %s user %s msg noInterupt = %d ,hideModel = %d", 
					mesGrpChat.sgrpid().c_str(), mesGrpChat.stoid().c_str(),  pUserGrpCfg->IsMsgNoInterruption(), pUserGrpCfg->IsInHidenModel());
				msgNoInterruption = pUserGrpCfg->IsMsgNoInterruption();
				if (!msgNoInterruption && pUserGrpCfg->IsInHidenModel()) //隐藏消息的免打搅
				{
					msgNoInterruption = !pUserCfg->IsHidenMsgSoundOn();
				}
			}

			if (!msgNoInterruption)
			{
				sendPush(pLogin, mesGrpChat.sfromid(), mesGrpChat.stoid(), mesGrpChat.smsgid(), GROUP_TALK, NewMsgStr);
			}
			
			}while(0);
		}
	}
	
	bool bInsertSuccess = m_offlineMsgMgr.InsertOfflineMsg(offlineMsgs);
	sendGrpMsgChatAck(msg, (bInsertSuccess ? NON_ERR : EXCEPT_ERR), sessionId);
	uint64_t grpChatAckSendTime = elspsedTimer.elapsed();

	if (!bInsertSuccess)
	{
		WarnLog("insert delever msgs for grp msg %s failed", msg.smsgid().c_str());
	}
	
	
	log("_MESGrpInterChat %s finish,_MESGrpInterChatAck(0xb072)@%llu, dispatch to %d mems@ %llu\r\n",
		msg.smsgid().c_str(), grpChatAckSendTime, msg.stoid_size(), elspsedTimer.elapsed());
	
}

//long selectMfromN(int n, int m, int * selected, int arrLen)
//{
//	long count = 0;
//	for (int i = m; i <= n; ++i)
//	{
//		selected[m - 1] = i;
//		if (m > 1)
//			count += selectMfromN(i - 1, m - 1, selected, arrLen);
//		else
//		{
//			/*			for(int j = 0; j < arrLen; j++)
//			{
//			cout << selected[j] << " ";
//			}
//			cout << endl;*/
//			++count;
//		}
//	}
//	return count;
//}
//
//long selectMn(int n, int r)
//{
//	int * selected = new int[r];
//	return selectMfromN(n, r, selected, r);
//	delete[] selected;
//}

void CGrpMsgHandler::sendGrpMsgChatAck(const _MESGrpInterChat& msg, im::ErrCode retCode, const UidCode_t& sessionId)
{
	_MESGrpInterChatAck msgChatAck;
	msgChatAck.set_smsgid(msg.smsgid());
	msgChatAck.set_suserid(msg.sfromid());
	msgChatAck.set_sgrpid(msg.sgrpid());
	msgChatAck.set_errcode(retCode);
	sendAck(&msgChatAck, MES_GRPINTERCHAT_ACK, sessionId);
}

void CGrpMsgHandler::OnGrpMsgChat(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	std::shared_ptr<im::_MESGrpInterChat> pMsg(new im::_MESGrpInterChat);
	if (!pMsg) return;
	_MESGrpInterChat& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	static int msgChatCount = 0;
	static CLock lock;
	int currentCount;

	{
		CAutoLock autoLock(&lock);
		currentCount =( ++msgChatCount);
	}
	
	log("_MESGrpInterChat-%d (0x%x) %s received , %s(%s), content len:%d", currentCount, pPdu->GetCommandId()/*MES_GRPINTERCHAT*/, msg.smsgid().c_str(),
		msg.sgrpid().c_str(), msg.sfromid().c_str(), msg.scontent().size());

	bool IsCreateTaskSuccess = 
		CCommonTaskMgr::InsertCommonTask(pMsg, OnGrpMsgChatStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()), BKDRHash(msg.sgrpid().c_str()));
	if (!IsCreateTaskSuccess)
	{
		sendGrpMsgChatAck(msg, EXCEPT_ERR, pPdu->GetSessionId());
	}
}

void CGrpMsgHandler::OnGrpMsgChatDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESGrpChatDeliveredAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("MESGrpChatDeliveredAck(0x%x) %s received,dir %s --> %s",
		pPdu->GetCommandId()/*MES_GRPCHAT_DELIVER_ACK*/, msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sgrpid().c_str());

	//放到跟插入任务的同一个任务队列中，避免执行先删除，再插入
	m_offlineMsgMgr.DelOfflineMsg(msg.sfromid(), MES_GRPCHAT_DELIVER, msg.smsgid(), NULL, NULL);
	//m_offlineMsgMgr.DelOfflineMsg(msg.sfromid(), MES_GRPCHAT_DELIVER, msg.smsgid());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//消息处理线程,将数据进行分发
void OnGrpMsgChatCancleStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pChatCancelTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CGrpMsgHandler* pHandle = (CGrpMsgHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::_MESGrpInterChatCancel> pChatCancel = dynamic_pointer_cast<im::_MESGrpInterChatCancel>(pChatCancelTask);
	//const im::_MESGrpInterChat& msg = *pChat;

	pHandle->HandleMsgGrpChatCancleTask(*pChatCancel, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}


void CGrpMsgHandler::HandleMsgGrpChatCancleTask(const im::_MESGrpInterChatCancel& msg, const UidCode_t& sessionId)
{ 
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

	vector<im::MESChatCancel>	deliverToGrpMemMsgs;
	vector<COfflineMsg>			offlineMsgs;
	vector<string>				toMemIds;

	im::MESChatCancel deliverMsg;
	deliverMsg.set_sfromid(msg.sfromid());
	deliverMsg.set_sgroupid(msg.sgrpid());
	deliverMsg.set_smsgid(msg.smsgid());
	deliverMsg.set_msgtime(msg.msgtime());
	deliverMsg.set_ncanceltype(1);

	for (int i = 0; i < msg.stoid_size(); ++i)
	{
		const string& toId = msg.stoid(i);
		deliverMsg.set_stoid(toId);

		deliverToGrpMemMsgs.push_back(deliverMsg);
		offlineMsgs.push_back(COfflineMsg(deliverMsg));
		toMemIds.push_back(toId);
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
	
	m_offlineMsgMgr.DelOfflineMsg(toMemIds, MES_GRPCHAT_DELIVER, msg.smsgid());
	bool isInsertSuccess = m_offlineMsgMgr.InsertOfflineMsg(offlineMsgs);
	sendGrpMsgChatCancelAck(msg, (isInsertSuccess ? NON_ERR : EXCEPT_ERR), sessionId);
	uint64_t grpChatCancelAckSendTime = elspsedTimer.elapsed();


	
	log("_MESGrpInterChatCancel %s finish,_MESGrpInterChatCancelAck(0xb076)@%llu, dispatch to %d mems@ %llu\r\n",
		msg.smsgid().c_str(), grpChatCancelAckSendTime, msg.stoid_size(), elspsedTimer.elapsed());
}


void CGrpMsgHandler::sendGrpMsgChatCancelAck(const _MESGrpInterChatCancel& msg, im::ErrCode retCode, const UidCode_t& sessionId)
{
	_MESGrpInterChatAck msgChatCancelAck;
	msgChatCancelAck.set_smsgid(msg.smsgid());
	msgChatCancelAck.set_suserid(msg.sfromid());
	msgChatCancelAck.set_sgrpid(msg.sgrpid());
	msgChatCancelAck.set_errcode(retCode);
	sendAck(&msgChatCancelAck, MES_GRPINTER_CHATCANCLE_ACK, sessionId);
}

void CGrpMsgHandler::OnGrpMsgChatCancle(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	std::shared_ptr<im::_MESGrpInterChatCancel> pMsg(new im::_MESGrpInterChatCancel);
	if (!pMsg) return;
	_MESGrpInterChatCancel& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

	log("msgGrpChatCancel (0x%x) %s received , %s(%s)", pPdu->GetCommandId()/*MES_GRPINTERCHAT*/, msg.smsgid().c_str(),
		msg.sgrpid().c_str(), msg.sfromid().c_str());

	bool IsCreateTaskSuccess = CCommonTaskMgr::InsertCommonTask(pMsg, OnGrpMsgChatCancleStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));
	if (!IsCreateTaskSuccess)
	{
		sendGrpMsgChatCancelAck(msg, EXCEPT_ERR, pPdu->GetSessionId());
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CGrpMsgHandler::OnGrpMsgChatCancleDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESChatCancelAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("MESGrpChatCancleDeliveredAck(0x%x) %s received,dir %s",
		pPdu->GetCommandId()/*MES_GRPCHAT_DELIVER_ACK*/, msg.smsgid().c_str(), msg.suserid().c_str());

	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_CHATCANCEL_DELIVER, msg.smsgid(), NULL, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GrpNotifyDeliverInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, bool bInsertSuccess, void* paras)
{
	if (NULL == paras) return;
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CGrpMsgHandler* pHandle = (CGrpMsgHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}
	const std::shared_ptr<COfflineMsg> pMsg = dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
	pHandle->OnGrpNotifyDeliverInserted(*pMsg, bInsertSuccess, pCallBackPara->m_bNeedSendPush);

	delete pCallBackPara;		//回调函数负责释放资源
}


void CGrpMsgHandler::HandleMsgGrpNotifyTask(const im::_MESGrpInterNotify& msg, const UidCode_t& sessionId /*= UidCode_t()*/)
{
	im::MESGrpNotify deliverMsg;
	deliverMsg.set_sgrpid(msg.sgrpid());
	deliverMsg.set_smsgid(msg.smsgid());
	deliverMsg.set_msgtime(msg.msgtime());

	if (!msg.sopruserid().empty())
	{
		deliverMsg.set_sopruserid(msg.sopruserid());
	}
	deliverMsg.set_scontent(msg.scontent());
	deliverMsg.set_notifytype(msg.notifytype());
	if (msg.errcode())
	{
		deliverMsg.set_errcode(msg.errcode());
	}
	deliverMsg.set_extend(msg.extend());
	
	for (int i = 0; i < msg.smnpleduserid_size(); ++i)
	{
		deliverMsg.add_smnpleduserid(msg.smnpleduserid(i));
	}

	for (int i = 0; i < msg.stoid_size(); ++i)
	{
		deliverMsg.set_stoid(msg.stoid(i));

		//创建插入任务，放到任务队列中
		m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(deliverMsg), GrpNotifyDeliverInsertedCallBack,
			new OfflineMsgInsertCallBackParas_t(this, sessionId, false /*bneedSendPush*/));

		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(deliverMsg.stoid());
		if (pLogin )	//用户在线直接尝试推送消息
		{
			if (pLogin->IsLogin())
			{
				sendReq(&deliverMsg, MES_GRPNOTIFY_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
				log("****send MES_GRPNOTIFY_DELIVER(0x%x) %s to %s", MES_GRPNOTIFY_DELIVER, deliverMsg.smsgid().c_str(), deliverMsg.stoid().c_str());
			}
		}		
	}
}


//消息处理线程,将数据进行分发
void OnGrpMsgNotifyStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pChatTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CGrpMsgHandler* pHandle = (CGrpMsgHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::_MESGrpInterNotify> pChat = dynamic_pointer_cast<im::_MESGrpInterNotify>(pChatTask);

	pHandle->HandleMsgGrpNotifyTask(*pChat, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}

void CGrpMsgHandler::OnGrpNotify(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	assert(NULL != pPdu);
	std::shared_ptr<im::_MESGrpInterNotify> pMsg(new im::_MESGrpInterNotify);
	if (!pMsg) return;
	_MESGrpInterNotify& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	static int notifyCount = 0;
	++notifyCount;

	log("MESGrpNotify-%d (0x%x) %s received , %s",notifyCount, pPdu->GetCommandId()/*MES_GRPNOTIFY*/, 
		msg.smsgid().c_str(), msg.sgrpid().c_str());

	bool IsCreateTaskSuccess = CCommonTaskMgr::InsertCommonTask(pMsg, OnGrpMsgNotifyStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));

	_MESGrpInterNotifyAck grpNotifyAck;
	grpNotifyAck.set_smsgid(msg.smsgid());
	grpNotifyAck.set_sgrpid(msg.sgrpid());
	grpNotifyAck.set_errcode(IsCreateTaskSuccess ? NON_ERR : EXCEPT_ERR);
	sendAck(&grpNotifyAck, MES_GRPINTERNOTIFY_ACK, pPdu->GetSessionId());
}


void CGrpMsgHandler::OnGrpNotifyDeliverInserted(const COfflineMsg& Msg, bool bInsertSuccess, bool bNeedSendPush)
{
	if (!bInsertSuccess || !bNeedSendPush) return;

	std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(Msg.GetToId());
	if (!pLogin) return;

	const std::shared_ptr<CUserCfg> pUserCfg = CUserCfgMgr::GetUserCfg(Msg.GetToId());
	if (!pUserCfg->IsGlobalNoInterruption())
	{
		sendPush(pLogin, Msg.GetFromId(), Msg.GetToId(), Msg.GetMsgId(), CONTACTS, NewMsgStr);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//收到notifyDeliverAck,则删除之前插入的数据，将任务放到相同的队列中

void CGrpMsgHandler::OnGrpNotifyDeliverAck(std::shared_ptr<CImPdu> pPdu)
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

bool CGrpMsgHandler::RegistPacketExecutor(void)
{
	CmdRegist(MES_GRPINTERCHAT,			m_nNumberOfInst,  CommandProc(&CGrpMsgHandler::OnGrpMsgChat));
	CmdRegist(MES_GRPCHAT_DELIVER_ACK,	m_nNumberOfInst,  CommandProc(&CGrpMsgHandler::OnGrpMsgChatDeliverAck));
	CmdRegist(MES_GRPINTER_CHATCANCLE, m_nNumberOfInst, CommandProc(&CGrpMsgHandler::OnGrpMsgChatCancle));
	CmdRegist(MES_CHATCANCEL_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CGrpMsgHandler::OnGrpMsgChatCancleDeliverAck));
	CmdRegist(MES_GRPINTERNOTIFY,		m_nNumberOfInst, CommandProc(&CGrpMsgHandler::OnGrpNotify));
	CmdRegist(MES_GRPNOTIFY_DELIVER_ACK, m_nNumberOfInst,  CommandProc(&CGrpMsgHandler::OnGrpNotifyDeliverAck));
	return true;
}

