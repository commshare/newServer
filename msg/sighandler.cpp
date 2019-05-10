#include "sighandler.h"
#include "configfilereader.h"
#include "im_loginInfo.h"
#include "im.sig.pb.h"
#include "im.pub.pb.h"
#include "im.pushSvrAPNsMsg.pb.h"
#include "redisLoginInfoMgr.h"
#include "redisUserCfgMgr.h"
#include "util.h"
#include "commonTaskMgr.h"
#include "im_friend.h"

using namespace im;
using namespace std;
im::ErrCode GetErrorCodeForChatToFriend(const string& userId, const string& friendId);

CSigHandler::CSigHandler(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader), m_nNumberOfInst(nNumOfInst)
{

}

CSigHandler::~CSigHandler()
{

}

void OnSponsorCallStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pChatTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CSigHandler* pHandle = (CSigHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::SIGSponsorCall> pChat = dynamic_pointer_cast<im::SIGSponsorCall>(pChatTask);

	pHandle->HandleSponsorCallTask(*pChat, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}

void CSigHandler::sendSponsorCallAck(const im::SIGSponsorCall& msg, im::ErrCode retCode, const UidCode_t& sessionId)
{
	SIGSponsorCallAck sponsorCallAck;
	sponsorCallAck.set_smsgid(msg.smsgid());
	sponsorCallAck.set_stoid(msg.sfromid());
	sponsorCallAck.set_errcode(retCode);
	sendAck(&sponsorCallAck, SIG_SPONSORP2PCALL_ACK, sessionId);
	

	if (NON_ERR == retCode)
	{
		DbgLog("****send SIGSponsorCallAck(0x%x) %s to %s, scallId=%s, errCode = 0x%X", SIG_SPONSORP2PCALL_ACK,
			sponsorCallAck.smsgid().c_str(), sponsorCallAck.stoid().c_str(), msg.scallid().c_str(), sponsorCallAck.errcode());
	}
	else
	{
		log("****send SIGSponsorCallAck(0x%x) %s to %s, ,scallId=%s, errCode = 0x%X", SIG_SPONSORP2PCALL_ACK,
			sponsorCallAck.smsgid().c_str(), sponsorCallAck.stoid().c_str(), msg.scallid().c_str(), sponsorCallAck.errcode());
	}
    }

void CSigHandler::HandleSponsorCallTask(const im::SIGSponsorCall& msg, const UidCode_t& sessionId)
{
	DbgLog("process handle begin msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

	FRIEND_INFO_ friendInfo;
	im::ErrCode retCode = checkFriendShip(msg.sfromid(), msg.sinviteid(), friendInfo);
	
	uint64_t checkUserRightTime = elspsedTimer.elapsed();
	if (retCode != NON_ERR)
	{
		sendSponsorCallAck(msg, retCode, sessionId);
		WarnLog("SponsorCall %s illegal ,Task handle finish, scallId=%s, at checkRight use %llu usecond ,total use use %llu usecond \n",
			msg.smsgid().c_str(), msg.scallid().c_str(), checkUserRightTime, elspsedTimer.elapsed());
		return;
	}

	//查找对端的呼叫状态并判断
	//std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.sinviteid());
	//if (pLogin && pLogin->IsCallBusy())
	//{
	//	sendSponsorCallAck(msg, ERR_CALL_BUSY, sessionId);
	//	log("SponsorCall %s finish,%llu right chked ,SIGSponsorCallAck(0x%x)@%llu\r\n",
	//		msg.smsgid().c_str(), SIG_SPONSORP2PCALL_ACK, checkUserRightTime, elspsedTimer.elapsed());
	//	return;
	//}

	COfflineMsg offmsg(msg);
	unsigned short bInsertSuccess = m_offlineMsgMgr.InsertOfflineMsg(offmsg, 0xffff);
	sendSponsorCallAck(msg, ((bInsertSuccess & MONGO_OPERATION_SUCCESS) ? NON_ERR : EXCEPT_ERR), sessionId);
	uint64_t chatAckSendTime = elspsedTimer.elapsed();

	uint64_t chatDeliverSendTime = 0;
	uint64_t pushSendTime = 0;

	if (bInsertSuccess)
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.sinviteid());
		//如果对端在线则直接推到对方
		if (pLogin)
		{
			if (pLogin->IsLogin())
			{
				sendReq(&msg, SIG_SPONSORP2PCALL_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
				chatDeliverSendTime = elspsedTimer.elapsed();
				log("****send SIG_SPONSORP2PCALL_DELIVER(0x%x) %s to %s sent, scallId=%s", SIG_SPONSORP2PCALL_DELIVER,
					msg.smsgid().c_str(), msg.sinviteid().c_str(), msg.scallid().c_str());
			}
			else //如果不在线则直接发推送
			{
				if (!(bInsertSuccess & MONGO_OPERATION_REPLACE_ONE)) 
				{
//					uint64_t nowTick = getCurrentTime_usec();
//					USER_INFO_ usrInfo;
//					string strCode = "";
//					CGetDataInterfaceManager dataInterface;
//					if(!dataInterface.getUserInfo(m_usrInfoUrl, msg.sinviteid(), usrInfo, strCode))
//					{
//						ErrLog("get user info from http fail usr_id=%s", msg.sinviteid().c_str());
//						return;
//					}
//					DbgLog("call msg off status %d time %llu", usrInfo.newCall, getCurrentTime_usec() - nowTick);
					DbgLog("call msg off status %d user_id %s", friendInfo.newCall, msg.sinviteid().c_str());
					if(friendInfo.newCall && checkDeviceLastUser(msg.sinviteid(), pLogin->GetDeviceToken()))		// 对电话通知的开关判断
					{
						string NewMsgStr = ""; 
						if(0 == msg.calltype())
							NewMsgStr = "\344\275\240\346\234\211\344\270\200\344\270\252\350\247\206\351\242\221\346\235\245\347\224\265";		//你有一个视频来电
						else
							NewMsgStr = "\344\275\240\346\234\211\344\270\200\344\270\252\346\226\260\346\235\245\347\224\265";		//你有一个新来电！
						if(friendInfo.pushType != APNS && friendInfo.pushType != APNS_DEV)
						{
							sendAndroidPush(msg.sfromid(), msg.sinviteid(), msg.smsgid(), P2P_CALL, NewMsgStr, friendInfo.pushToken, friendInfo.pushType);
						}
						else
						{
							sendiOSPush(msg.sfromid(), msg.sinviteid(), msg.smsgid(), P2P_CALL, NewMsgStr, friendInfo.voipToken, friendInfo.pushType, msg.calltype(), msg.scallid());
						}
//						sendPush(msg.sfromid(), msg.sinviteid(), msg.smsgid(), P2P_CALL, NewMsgStr, friendInfo.pushType, friendInfo.pushToken, friendInfo.voipToken, friendInfo.versionCode);
						pushSendTime = elspsedTimer.elapsed();
					}
				}
			}
		}
	}

	log("SponsorCall %s finish, scallId=%s, %llu right chked ,SIGSponsorCallAck(0x%x)@%llu, SIGSponsorCallDeliver(0xb004)@%llu, push@ %llu\r\n",
		msg.smsgid().c_str(), msg.scallid().c_str(), SIG_SPONSORP2PCALL_ACK, checkUserRightTime, chatAckSendTime, chatDeliverSendTime, pushSendTime);
}


    void CSigHandler::OnSponsorCall(std::shared_ptr<CImPdu> pPdu)
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

	assert(NULL != pPdu);
	std::shared_ptr<im::SIGSponsorCall> pMsg(new im::SIGSponsorCall);
	if (!pMsg) return;
	SIGSponsorCall& msg = *pMsg;

	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
    log("SponsorCall(0x%x) %s received %s:%s, scallId=%s", pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sinviteid().c_str(), msg.scallid().c_str());

	if (msg.sfromid().empty() || msg.sinviteid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}

    msg.set_msgtime(getCurrentTime());//set server time
    
	DbgLog("process add task msgId=%s",msg.smsgid().c_str());
	CCommonTaskMgr::InsertCommonTask(pMsg, OnSponsorCallStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()), BKDRHash(msg.sinviteid().c_str()));
	log("SponsorCall (0x%x) %s prehandled , %s-->%s, scallId=%s,  use %llu usecond \n", pPdu->GetCommandId()/*MES_CHAT*/, msg.smsgid().c_str(),
		msg.sfromid().c_str(), msg.sinviteid().c_str(), msg.scallid().c_str(), elspsedTimer.elapsed());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void CSigHandler::OnSponsorCallDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SIGSponsorCallDeliverAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("nSponsorCallDeliverAck(0x%x) %s received", pPdu->GetCommandId()/*SIG_SPONSORP2PCALL_DELIVER_ACK*/, msg.smsgid().c_str());
	if (msg.suserid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
	CLoginInfoMgr::UpdateCallBusyState(msg.suserid(), USER_CALL_STATE_BEING_CALLED);	//正在被呼叫
	//delete SponsorCallDeliver msg from mongo 
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), SIG_SPONSORP2PCALL_DELIVER, msg.smsgid());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void CSigHandler::OnSponsorCallAns(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SIGSponsorCallAns msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
    log("SponsorCallAns(0x%x) %s received, dir(%s-->%s), scallId=%s, code = 0x%x\r\n", pPdu->GetCommandId()/*SIG_SPONSORP2PCALL_ANS*/,
		msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), msg.scallid().c_str(),  msg.errcode());

	if (msg.sfromid().empty() || msg.stoid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
//	if (!msg.msgtime())
//	{
//		msg.set_msgtime(getCurrentTime());
//	}
    msg.set_msgtime(getCurrentTime());//set server time
    
	FRIEND_INFO_ friendInfo;
	im::ErrCode retCode = checkFriendShip(msg.stoid(), msg.sfromid(), friendInfo);
    m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(msg));

	if (NON_ERR == retCode)
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
		if (pLogin && pLogin->IsLogin())	//if receiver was online, send SponsorCallDeliver to receiver
		{
			sendReq(&msg, SIG_SPONSORP2PCALL_ANS_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			log("****send SIG_SPONSORP2PCALL_ANS_DELIVER(0x%x) %s to %s sent, scallId=%s",
				SIG_SPONSORP2PCALL_ANS_DELIVER, msg.smsgid().c_str(), msg.stoid().c_str(), msg.scallid().c_str());
			if (NON_ERR == msg.errcode())	//如果用户接听
			{
				CLoginInfoMgr::UpdateCallBusyState(msg.sfromid(), USER_CALL_STATE_BUSY);
				CLoginInfoMgr::UpdateCallBusyState(msg.stoid(), USER_CALL_STATE_BUSY);
			}
			else if (EXCEPT_ERR == msg.errcode())
			{
				CLoginInfoMgr::UpdateCallBusyState(msg.sfromid(), USER_CALL_STATE_IDEL);
				CLoginInfoMgr::UpdateCallBusyState(msg.stoid(), USER_CALL_STATE_IDEL);
			}
		}
		else
		{
			retCode = ERR_CALL_PEER_OFFLINE;
		}
	}

	SIGSponsorCallAnsAck sponsorCallAnsAck;
	sponsorCallAnsAck.set_smsgid(msg.smsgid());
	sponsorCallAnsAck.set_stoid(msg.sfromid());
	sponsorCallAnsAck.set_errcode(retCode);
	sendAck(&sponsorCallAnsAck, SIG_SPONSORP2PCALL_ANS_ACK, pPdu->GetSessionId());
	log("****send SIGSponsorCallAnsAck(0x%x) %s to %s sent, scallId=%s", SIG_SPONSORP2PCALL_ANS_ACK,
		sponsorCallAnsAck.smsgid().c_str(), sponsorCallAnsAck.stoid().c_str(), msg.scallid().c_str());


}

void CSigHandler::OnSponsorCallAnsDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SIGSponsorCallAnsDeliverACK msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
    log("nSponsorCallAnsDeliverAck(0x%x) %s received", pPdu->GetCommandId(), msg.smsgid().c_str());

	if (msg.suserid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
	//delete SponsorCallDeliver msg from mongo 
	//COfflineMsgMgr offlineMsgMgr;
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), SIG_SPONSORP2PCALL_ANS_DELIVER, msg.smsgid());
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OnHangUpStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CSigHandler* pHandle = (CSigHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::SIGHangUp> pHangUp = dynamic_pointer_cast<im::SIGHangUp>(pTask);
	//const im::SIGSponsorCall& msg = *pChat;

	pHandle->HandleHangUpTask(*pHangUp, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}

    
 void CSigHandler::sendHangUpAck(const im::SIGHangUp& msg, im::ErrCode retCode, const UidCode_t& sessionId)
{
	SIGHangUpAck hangUpAck;
	hangUpAck.set_smsgid(msg.smsgid());
	hangUpAck.set_stoid(msg.sfromid());
	hangUpAck.set_errcode(retCode);
	sendAck(&hangUpAck, SIG_P2PCALLHANGUP_ACK, sessionId);

	if (NON_ERR == retCode)
	{
		DbgLog("****send SIGHangUpAck(0x%x) %s to %s sent, scallId=%s", SIG_P2PCALLHANGUP_ACK,
			hangUpAck.smsgid().c_str(), hangUpAck.stoid().c_str(), msg.scallid().c_str());
	}
	else
	{
		log("****send SIGHangUpAck(0x%x) %s to %s sent, scallId=%s", SIG_P2PCALLHANGUP_ACK,
			hangUpAck.smsgid().c_str(), hangUpAck.stoid().c_str(), msg.scallid().c_str());
	}
}


void CSigHandler::HandleHangUpTask(const im::SIGHangUp& msg, const UidCode_t& sessionId)
{
	DbgLog("process handle begin msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

	FRIEND_INFO_ friendInfo;
	im::ErrCode retCode = checkFriendShip(msg.sfromid(), msg.stoid(), friendInfo);
	
	uint64_t checkUserRightTime = elspsedTimer.elapsed();

	if (retCode != NON_ERR)
	{
		sendHangUpAck(msg, retCode, sessionId);
		WarnLog("SIGHangUp %s illegal ,scallId=%s, Task handle finish, at checkRight use %llu usecond ,total use use %llu usecond \n",
			msg.smsgid().c_str(), msg.scallid().c_str(), checkUserRightTime, elspsedTimer.elapsed());
		return;
	}

	// 若找到callid的消息，将其状态置为1（取消），否则保存这条取消消息
	bool bHangUpStatus = false;
	if(!msg.scallid().empty())
		bHangUpStatus = m_offlineMsgMgr.updateCallOfflineMsgStatus(msg.stoid(), SIG_SPONSORP2PCALL_DELIVER, msg.scallid(), 1);
	bool bInsertDbStatus = false;
	if(!bHangUpStatus)
	{	
		COfflineMsg offmsg(msg);
		bInsertDbStatus  = m_offlineMsgMgr.InsertOfflineMsg(offmsg);
		if(!bInsertDbStatus)
			WarnLog("insert signal hang up msg  fail.from %s to %s msgid %s, scallId=%s", msg.sfromid().c_str(), msg.stoid().c_str(), msg.smsgid().c_str(),
                    msg.scallid().c_str());
	}


	//这里暂时没有做判断，不确定是不是实际存在的通话
	CLoginInfoMgr::UpdateCallBusyState(msg.sfromid(), USER_CALL_STATE_IDEL);
	CLoginInfoMgr::UpdateCallBusyState(msg.stoid(), USER_CALL_STATE_IDEL);
	

	sendHangUpAck(msg, ((bHangUpStatus || bInsertDbStatus) ? NON_ERR : EXCEPT_ERR), sessionId);
	uint64_t chatAckSendTime = elspsedTimer.elapsed();
	uint64_t chatDeliverSendTime = 0;

	if (bHangUpStatus || bInsertDbStatus) 
	{
		//在线则直接推送给对端，但不在线目前不发推送
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
		if (pLogin && pLogin->IsLogin())
		{
			sendReq(&msg, SIG_P2PCALLHANGUPDElIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			chatDeliverSendTime = elspsedTimer.elapsed();
			log("****send SIG_P2PCALLHANGUPDElIVER(0x%x) %s to %s sent, scallId=%s", SIG_P2PCALLHANGUPDElIVER,
				msg.smsgid().c_str(), msg.stoid().c_str(), msg.scallid().c_str());
		}
		else
		{
#if 0				
			std::shared_ptr<CUserCfg> pUserCfg = CUserCfgMgr::GetUserCfg(msg.stoid());
			bool bCallMsgOff = pUserCfg->IsCallMsgOff();
			DbgLog("call msg off status %d", bCallMsgOff);
			if(!bCallMsgOff)		// 对电话通知的开关判断
			{
				if(pLogin && (pLogin->GetDevType() == APNS || pLogin->GetDevType() == APNS_DEV))
				{
					// 定义一个特殊的标识符“cancel_p2p_call”，表示p2p通话取消
					sendPush(pLogin, msg.sfromid(), msg.stoid(), msg.smsgid(), P2P_CALL, "cancel_p2p_call");
				}
			}
#endif
			if(!bInsertDbStatus)
			{
				COfflineMsg offmsg(msg);
				bInsertDbStatus  = m_offlineMsgMgr.InsertOfflineMsg(offmsg);
				if(!bInsertDbStatus)
					WarnLog("insert signal hang up msg  fail.from %s to %s msgid %s, scallId=%s", msg.sfromid().c_str(), msg.stoid().c_str(), msg.smsgid().c_str(),
                            msg.scallid().c_str());
			}
		}
	}

	log("Hangup %s finish, scallId=%s, %llu right chked ,SIGHangUpAck(0x%x)@%llu, SIGSponsorCallDeliver(0xb004)@%llu\r\n",
		msg.smsgid().c_str(), msg.scallid().c_str(), SIG_P2PCALLHANGUP_ACK, checkUserRightTime, chatAckSendTime, chatDeliverSendTime);
}


void CSigHandler::OnHangUp(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);

	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();
	std::shared_ptr<im::SIGHangUp> pMsg(new im::SIGHangUp);
	if (!pMsg) return;
	SIGHangUp& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
    log("SIGHangUp(0x%x) %s received, scallId=%s, %s delfriend %s\r\n", pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.scallid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());

	if (msg.sfromid().empty() || msg.stoid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
    msg.set_msgtime(getCurrentTime());//set server time

	DbgLog("process add task msgId=%s",msg.smsgid().c_str());
	CCommonTaskMgr::InsertCommonTask(pMsg, OnHangUpStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()), BKDRHash(msg.stoid().c_str()));
    log("HangUp(0x%x) %s prehandled ,scallId=%s, %s-->%s, use %llu usecond \n", pPdu->GetCommandId(), msg.smsgid().c_str(),
		msg.scallid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), elspsedTimer.elapsed());
	
}


void CSigHandler::OnHangupDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SIGHangUpDeliverAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
    log("SIGHangUpDeliverAck(0x%x) %s received", pPdu->GetCommandId(), msg.smsgid().c_str());

	if (msg.suserid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
	//delete SponsorCallDeliver msg from mongo 
	//COfflineMsgMgr offlineMsgMgr;
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), SIG_P2PCALLHANGUPDElIVER, msg.smsgid());
}


bool CSigHandler::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(SIG_SPONSORP2PCALL, m_nNumberOfInst, CommandProc(&CSigHandler::OnSponsorCall));
	CmdRegist(SIG_SPONSORP2PCALL_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CSigHandler::OnSponsorCallDeliverAck));
	CmdRegist(SIG_SPONSORP2PCALL_ANS, m_nNumberOfInst, CommandProc(&CSigHandler::OnSponsorCallAns));
	CmdRegist(SIG_SPONSORP2PCALL_ANS_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CSigHandler::OnSponsorCallAnsDeliverAck));
	CmdRegist(SIG_P2PCALLHANGUP, m_nNumberOfInst, CommandProc(&CSigHandler::OnHangUp));
	CmdRegist(SIG_P2PCALLHANGUPDElIVER_ACK, m_nNumberOfInst, CommandProc(&CSigHandler::OnHangupDeliverAck));

	CmdRegist(SIG_P2PCALL_EXCHANGE_NATINFO, m_nNumberOfInst, CommandProc(&CSigHandler::OnExchangeNatInfo));
	CmdRegist(SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CSigHandler::OnExchangeNatInfoDeliverAck));

	CmdRegist(SIG_P2PCALLSTATENOTIFY, m_nNumberOfInst, CommandProc(&CSigHandler::OnCallStateNotify));
	CmdRegist(SIG_P2PCALLSTATENOTIFYDElIVER_ACK, m_nNumberOfInst, CommandProc(&CSigHandler::OnCallStateNotifyDeliverAck));
	return true;
}


void CSigHandler::OnExchangeNatInfo(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SIGP2PCallExchangeNatInfo msg;

	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
    log("exchangeNatInfo(0x%x) %s , scallId=%s, received %s-->%s", pPdu->GetCommandId(), msg.smsgid().c_str(), msg.scallid().c_str(),  msg.sfromid().c_str(), msg.stoid().c_str());
	if (!msg.msgtime())
	{
		msg.set_msgtime(getCurrentTime());
	}

	msg.set_msgtime(getCurrentTime());

	//FRIEND_INFO_ friendInfo;
	//im::ErrCode retCode = checkFriendShip(msg.sfromid(), msg.stoid(), friendInfo);

	//if receiver was online, send SponsorCallDeliver to receiver
	//bool bneedSendPush = true;
	im::ErrCode retCode = NON_ERR; 
	//if (NON_ERR == retCode)
	//{
		// 存储net信息
		bool bRes = m_offlineMsgMgr.InsertOne(COfflineMsg(msg));
		DbgLog("insert nat info result %d", bRes);
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
		if (pLogin && pLogin->IsLogin())
		{
			sendReq(&msg, SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			//bneedSendPush = false;
			log("****send SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER(0x%x) %s to %s sent, scallId=%s", SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER,
				msg.smsgid().c_str(), msg.stoid().c_str(), msg.scallid().c_str());
		}
		else
		{
			retCode = ERR_CALL_PEER_OFFLINE;
		}

		//交换信息的时候双方必须在线
		//save SponsorCallDeliver msg to mongodb, if insert failed ,can try to send deliver
		//m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(msg), ExchangeNatInfoInsertedCallBack,
		//	new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId(), bneedSendPush));
	//}

	SIGP2PCallExchangeNatInfoAck msgAck;
	msgAck.set_smsgid(msg.smsgid());
	msgAck.set_stoid(msg.sfromid());
	msgAck.set_errcode(retCode);
	sendAck(&msgAck, SIG_P2PCALL_EXCHANGE_NATINFO_ACK, pPdu->GetSessionId());
	log("****send SIGP2PCallExchangeNatInfoAck(0x%x) %s to %s sent, scallId=%s", SIG_P2PCALL_EXCHANGE_NATINFO_ACK,
		msgAck.smsgid().c_str(), msgAck.stoid().c_str(), msg.scallid().c_str());
}


void CSigHandler::OnExchangeNatInfoDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SIGP2PCallExchangeNatInfoDeliverAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

    log("SIGP2PCallExchangeNatInfoDeliverAck(0x%x) %s received",
		pPdu->GetCommandId(), msg.smsgid().c_str());

	//delete SponsorCallDeliver msg from mongo 
	//COfflineMsgMgr offlineMsgMgr;
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER, msg.smsgid()); // // 删除net信息

}

void CSigHandler::OnCallStateNotify(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SIGP2PCallStateNotify msg;

	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
    log("callStateNotify(0x%x) %s, scallId=%s, received %s-->%s", pPdu->GetCommandId(), msg.smsgid().c_str(),msg.scallid().c_str(),  msg.sfromid().c_str(), msg.stoid().c_str());
//	if (!msg.msgtime())
//	{
//		msg.set_msgtime(getCurrentTime());
//	}

    msg.set_msgtime(getCurrentTime());//set server time

	FRIEND_INFO_ friendInfo;
	im::ErrCode retCode = checkFriendShip(msg.sfromid(), msg.stoid(), friendInfo);

	if (NON_ERR == retCode)
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
		if (pLogin && pLogin->IsLogin())
		{
			sendReq(&msg, SIG_P2PCALLSTATENOTIFYDElIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			//bneedSendPush = false;
			log("****send SIG_P2PCALLSTATENOTIFY_DElIVER(0x%x) %s to %s sent, scallId=%s", SIG_P2PCALLSTATENOTIFYDElIVER,
				msg.smsgid().c_str(), msg.stoid().c_str(), msg.scallid().c_str());
		}
		else
		{
			retCode = ERR_CALL_PEER_OFFLINE;
		}

		//交换信息的时候双方必须在线
		//save SponsorCallDeliver msg to mongodb, if insert failed ,can try to send deliver
		//m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(msg), ExchangeNatInfoInsertedCallBack,
		//	new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId(), bneedSendPush));
	}

    SIGP2PCallStateNotifyACK  msgAck;
	msgAck.set_smsgid(msg.smsgid());
	msgAck.set_stoid(msg.sfromid());
	msgAck.set_errcode(retCode);
	sendAck(&msgAck, SIG_P2PCALLSTATENOTIFY_ACK, pPdu->GetSessionId());
	log("****send SIGP2PCallStateNotifyACK(0x%x) %s to %s sent, scallId=%s", SIG_P2PCALLSTATENOTIFY_ACK,
		msgAck.smsgid().c_str(), msgAck.stoid().c_str(), msg.scallid().c_str());
}

void CSigHandler::OnCallStateNotifyDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	SIGP2PCallStateNotifyDeliverAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

	log("\r\n\r\n\r\nSIGP2PCallStateNotifyDeliverAck(0x%x) %s received",
		pPdu->GetCommandId(), msg.smsgid().c_str());
}

im::ErrCode CSigHandler::checkFriendShip(const string& userId, const string& friendId, FRIEND_INFO_& friendInfo)
{
	im::ErrCode retCode = NON_ERR;
	CGetDataInterfaceManager dataInterface;
	string strCode = "";
	uint64_t nowTick = getCurrentTime_usec();
	if(!dataInterface.getFriendInfo(m_friendInfoUrl, m_sAppSecret, userId, friendId, friendInfo, strCode))
	{
		retCode = EXCEPT_ERR;
		ErrLog("get friend info fail");
		return retCode;
	}
	DbgLog("Thread[%lu] get friend info time %llu",pthread_self(), getCurrentTime_usec() - nowTick);

	if(NO_RELA == friendInfo.friendStatus || NO_RELA == friendInfo.userStatus)
		retCode = ERR_CHAT_FORBIDDEN;
	else if(IS_BLACK == friendInfo.friendStatus || NO_RELA == friendInfo.userStatus)
		retCode = ERR_CHAT_FRIEND_BLOCK;
	else if(IS_DEL == friendInfo.friendStatus || IS_DEL == friendInfo.userStatus)
		retCode = ERR_CHAT_FRIEND_DEL;
	return retCode;
}

