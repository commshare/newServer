/******************************************************************************
Filename: msghandle.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/07
Description: 
******************************************************************************/
#include "configfilereader.h"
#include "msghandle.h"
#include "im_time.h"
#include "im_loginInfo.h"
#include "im.mes.pb.h"
#include "im.pub.pb.h"
#include "im.push.android.pb.h"
#include "im.pushSvrAPNsMsg.pb.h"
#include "mysqlFriendMgr.h"
#include "mysqlUserMgr.h"
#include "redisFriendMgr.h"
#include "redisLoginInfoMgr.h"
#include "redisUserCfgMgr.h"
#include "serverinfo.h"
#include "util.h"
#include "commonTaskMgr.h"
#include"getDataInterfaceManager.h"

using namespace im;
using namespace std;

bool isCustomerService(const string& toId)
{
	return toId == string("10000");
}

CMsgHandler::CMsgHandler(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader), m_nNumberOfInst(nNumOfInst)
{

}

CMsgHandler::~CMsgHandler()
{

}

std::vector<CFriendRelation>  GetFriendShips(const string& userId, const string& friendId);

struct MsgSessionSetting
{
	bool m_nHidenFlag = false;			//会话是否是隐藏模式
	bool m_nSoundOff = false;		//会话是否设置了免打搅
};

im::ErrCode GetErrorCodeForChatToFriend(const string& userId, const string& friendId, MsgSessionSetting& sessionSetting)
{
	static const string ServerID("10000");
	if (userId == ServerID || friendId == ServerID ) return NON_ERR;

	//static const string testID("1387044");
	//if (userId == testID || friendId == testID) return NON_ERR;

	std::vector<CFriendRelation> relationShips = GetFriendShips(userId, friendId);

	//no relationship create
	if (relationShips.size() < 2)
		return ERR_CHAT_FORBIDDEN;

	//check whether can send Msg
	std::vector<CFriendRelation>::iterator it = relationShips.begin();
	for (; it != relationShips.end(); ++it)
	{
		//对方的好友状态
		if (it->GetUserId() == friendId)
		{
			sessionSetting.m_nHidenFlag = it->IsHidenModel();
			sessionSetting.m_nSoundOff = it->IsNoInterruption();
			if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_BLOCKED))
			{
				return ERR_CHAT_FRIEND_BLOCK;
			}
			else if (it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_DELETED))
			{
				return ERR_CHAT_FRIEND_DEL;
			}
		}
		if (!it->IsFlagSet(CFriendRelation::FRIEND_RELATION_STATE_NORMAL))	//if either side is unNormal, forbidden that
			return ERR_CHAT_FORBIDDEN;

	}
	return NON_ERR;
}

im::ErrCode GetErrorCodeForChatToFriend(const string& userId, const string& friendId)
{
	MsgSessionSetting sessionSetting; 
	return GetErrorCodeForChatToFriend(userId, friendId, sessionSetting);
}


//消息处理线程
//做一些权限的检查并尝试直接将消息推送给对方
void OnMsgChatStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pChatTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CMsgHandler* pHandle = (CMsgHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::MESChat> pChat = dynamic_pointer_cast<im::MESChat>(pChatTask);
	//const im::MESChat& msg = *pChat;
	
	pHandle->HandleMsgChatTask(*pChat, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}

void CMsgHandler::sendMsgChatAck(const im::MESChat& msg, im::ErrCode retCode, const UidCode_t& sessionId)
{
	const uint64_t msgTime = getCurrentTime();
	MESChatAck msgChatAck;
	msgChatAck.set_smsgid(msg.smsgid());
	msgChatAck.set_suserid(msg.sfromid());
	msgChatAck.set_sendtime(msgTime);
	msgChatAck.set_errcode(retCode);
	sendAck(&msgChatAck, MES_CHAT_ACK, sessionId);
	if (NON_ERR == retCode)
	{
		DbgLog("****send MESChatAck(0x%x) %s to %s, code = 0X%X,time %zd", MES_CHAT_ACK,
			msgChatAck.smsgid().c_str(), msgChatAck.suserid().c_str(), retCode, msgTime);
	}
	else
	{
		log("****send MESChatAck(0x%x) %s to %s, code = 0X%X,time %zd", MES_CHAT_ACK,
			msgChatAck.smsgid().c_str(), msgChatAck.suserid().c_str(), retCode, msgTime);
	}
}

void CMsgHandler::HandleMsgChatTask(const im::MESChat& msg, const UidCode_t& sessionId)
{
	DbgLog("process handle begin msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
	im::ErrCode retCode = NON_ERR;
	unsigned short bInsertSuccess = 0;
	FRIEND_INFO_ friendInfo;
	do 
	{
		if(msg.scontent().empty())
		{
			retCode = ERR_CHAT_UNHEALTHY;
			break;
		}

		uint64_t nowTick = getCurrentTime_usec();
		CGetDataInterfaceManager dataInterface;
		string strCode = "";
		if(!dataInterface.getFriendInfo(m_friendInfoUrl, m_sAppSecret, msg.sfromid(), msg.stoid(), friendInfo, strCode))
		{
			retCode = im::ERR_GROUP_NETWORKEXCEPTION;
			ErrLog("get friend info fail");
			break;
		}
		DbgLog("Thread[%lu] get friend info time %llu msgId=%s",pthread_self(), getCurrentTime_usec() - nowTick, msg.smsgid().c_str());

		if(NO_RELA == friendInfo.friendStatus || NO_RELA == friendInfo.userStatus)
		{
			retCode = ERR_CHAT_FORBIDDEN;
			break;
		}

		if(IS_BLACK == friendInfo.friendStatus || IS_BLACK == friendInfo.userStatus)
		{
			retCode = ERR_CHAT_FRIEND_BLOCK;
			break;
		}

		if(IS_DEL == friendInfo.friendStatus || IS_DEL == friendInfo.userStatus)
		{
			retCode = ERR_CHAT_FRIEND_DEL;
			break;
		}
		
		COfflineMsg offmsg(msg);
		DbgLog("Thread[%lu] insertOffline begin msg[%s]",pthread_self(),msg.smsgid().c_str());
		bInsertSuccess = m_offlineMsgMgr.InsertOfflineMsg(offmsg, 0xffff);
		DbgLog("Thread[%lu] insertOffline end msg[%s]",pthread_self(),msg.smsgid().c_str());
		retCode = (bInsertSuccess & MONGO_OPERATION_SUCCESS) ? NON_ERR : EXCEPT_ERR;
		if(0 == bInsertSuccess )
			ErrLog("Thread[%lu] insert msg %s to mongodb failed",pthread_self(), msg.smsgid().c_str());
	}while(0);
	sendMsgChatAck(msg, retCode,sessionId);
	if(retCode != NON_ERR)
	{
		WarnLog("Thread[%lu] send msg exception! msgid=%s, from=%s, to=%s code=%d", pthread_self(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), retCode);
		return;
	}

	if (isCustomerService(msg.stoid()))
	{
		sendReq(&msg, MES_CHAT_DELIVER, imsvr::CSR);
	}
	else
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());		
		if (nullptr == pLogin)
		{
			ErrLog("Thread[%lu]  get login info fail  msg=%s, toid=%s",pthread_self(), msg.smsgid().c_str(), msg.stoid().c_str());
			return;
		}
		if(pLogin->IsLogin())	//如果对端在线则直接推到对方
		{
			sendReq(&msg, MES_CHAT_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			DbgLog("****from_id %s send MES_CHAT_DELIVER(0x%x) %s to %s", msg.sfromid().c_str(), MES_CHAT_DELIVER, msg.smsgid().c_str(), msg.stoid().c_str());
		}
		else
		{
			if(bInsertSuccess & MONGO_OPERATION_REPLACE_ONE)
			{
				WarnLog("the msg is repeat! msgid=%s, from=%s, to=%s", msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());
				return;
			}
			bool bPush = friendInfo.newMsg;
			if(friendInfo.newMsg)
			{
				bPush = !friendInfo.undisturb;
				if(!friendInfo.undisturb && IS_STRANGER == friendInfo.friendStatus)
				{
					bPush = friendInfo.hideMsgSoundOn;
				}
			}
			DbgLog("user %s bPush = %d", msg.stoid().c_str(), bPush);	
			if(bPush && checkDeviceLastUser(msg.stoid(), pLogin->GetDeviceToken()))
			{
				sendPush(msg.sfromid(), msg.stoid(), msg.smsgid(), im::PERSONAL_TALK, NewMsgStr, 
								friendInfo.pushType, friendInfo.pushToken, friendInfo.voipToken, friendInfo.versionCode);
				DbgLog("****from_id %s push MES_CHAT_DELIVER(0x%x) %s to %s", msg.sfromid().c_str(), MES_CHAT_DELIVER, msg.smsgid().c_str(), msg.stoid().c_str());
			}	
		}
	}
	DbgLog("process handle end msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
}


//收到消息后，做简单的消息合法性检查，组成任务放到线程池中
void CMsgHandler::OnMsgChat(std::shared_ptr<CImPdu> pPdu)
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

	assert(NULL != pPdu);
	std::shared_ptr<im::MESChat> pMsg(new im::MESChat);
	if (!pMsg) return;
	MESChat& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	static int msgChatCount = 0;
	++msgChatCount;
	
	if (msg.sfromid().empty())
	{
		ErrLog("no fromId specified");
		return;
	}

	if (msg.stoid().empty() || msg.smsgid().empty())
	{
		ErrLog("no toId or msgId specified");
		return;
	}

    msg.set_msgtime(getCurrentTime());//set server time
	DbgLog("process add task msgId=%s",msg.smsgid().c_str());
	CCommonTaskMgr::InsertCommonTask(pMsg, OnMsgChatStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()),BKDRHash(msg.stoid().c_str()));
	log("msgChat-%d (0x%x) %s prehandled , %s-->%s, content Len %d, content = %s, use %llu usecond \n", msgChatCount, pPdu->GetCommandId()/*MES_CHAT*/, msg.smsgid().c_str(),
		msg.sfromid().c_str(), msg.stoid().c_str(), msg.scontent().size(), (msg.scontent().size() > 40 ? "..." : msg.scontent().c_str()), elspsedTimer.elapsed());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//消息处理线程
//做一些权限的检查并尝试直接将消息推送给对方
void OnMsgChatCancelStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pChatCancelTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CMsgHandler* pHandle = (CMsgHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::MESChatCancel> pChatCancel = dynamic_pointer_cast<im::MESChatCancel>(pChatCancelTask);
	//const im::MESChatCancel& msg = *pChatCancel;

	pHandle->HandleMsgChatCancelTask(*pChatCancel, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}

void CMsgHandler::sendMsgChatCancelAck(const im::MESChatCancel& msg, im::ErrCode retCode, const UidCode_t& sessionId)
{
	MESChatCancelAck msgChatCancelAck;
	msgChatCancelAck.set_smsgid(msg.smsgid());
	msgChatCancelAck.set_suserid(msg.sfromid());
	msgChatCancelAck.set_errcode(retCode);
	sendAck(&msgChatCancelAck, MES_CHATCANCEL_ACK, sessionId);
	if (NON_ERR == retCode)
	{
		DbgLog("****send MESChatCancelAck(0x%x) %s to %s, code = 0X%X", MES_CHATCANCEL_ACK,
			msgChatCancelAck.smsgid().c_str(), msgChatCancelAck.suserid().c_str(), retCode);
	}
	else
	{
		log("****send MESChatCancelAck(0x%x) %s to %s, code = 0X%X", MES_CHATCANCEL_ACK,
			msgChatCancelAck.smsgid().c_str(), msgChatCancelAck.suserid().c_str(), retCode);
	}
}

void CMsgHandler::HandleMsgChatCancelTask(im::MESChatCancel& msg, const UidCode_t& sessionId)
{
	DbgLog("process handle begin msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
	im::ErrCode retCode = NON_ERR;
	bool bUpdateSuccess = false;
	FRIEND_INFO_ friendInfo;
	do 
	{
		uint64_t nowTick = getCurrentTime_usec();
		CGetDataInterfaceManager dataInterface;
		string strCode = "";
		if(!dataInterface.getFriendInfo(m_friendInfoUrl, m_sAppSecret, msg.sfromid(), msg.stoid(), friendInfo, strCode))
		{
			retCode = im::ERR_GROUP_NETWORKEXCEPTION;
			ErrLog("get friend info fail");
			break;
		}
		DbgLog("Thread[%lu] get friend info time %llu",pthread_self(), getCurrentTime_usec() - nowTick);

		if(NO_RELA == friendInfo.friendStatus || NO_RELA == friendInfo.userStatus)
		{
			retCode = ERR_CHAT_FORBIDDEN;
			break;
		}
		if(IS_BLACK == friendInfo.friendStatus || IS_BLACK == friendInfo.userStatus)
		{
			retCode = ERR_CHAT_FRIEND_BLOCK;
			break;
		}
		if(IS_DEL == friendInfo.friendStatus || IS_DEL == friendInfo.userStatus)
		{
			retCode = ERR_CHAT_FRIEND_DEL;
			break;
		}

		DbgLog("Thread[%lu] cancel chat msg update offline msg begin! msg[%s]",pthread_self(), msg.smsgid().c_str());
		msg.set_sendstate(0);
		if(m_offlineMsgMgr.getOfflineMsgPullStatus(msg.stoid(), MES_CHAT_DELIVER, msg.smsgid()))
			msg.set_sendstate(1);
		DbgLog("Thread[%lu] read offline msg status! msg[%s] status=%d",pthread_self(), msg.smsgid().c_str(), msg.sendstate());
		bUpdateSuccess = m_offlineMsgMgr.UpdateOfflineMsg(msg.stoid(), MES_CHAT_DELIVER, msg.smsgid(), COfflineMsg(msg));
		DbgLog("Thread[%lu] cancel chat msg update offline msg end! msg[%s]",pthread_self(), msg.smsgid().c_str());
		retCode = bUpdateSuccess ? NON_ERR : EXCEPT_ERR;
		if(!bUpdateSuccess )
			ErrLog("Thread[%lu] cancel chat msg update fail! msg %s to mongodb failed",pthread_self(), msg.smsgid().c_str());
	}while(0);

	sendMsgChatCancelAck(msg, retCode,sessionId);
	if(retCode != NON_ERR)
	{
		WarnLog("send cancel msg exception! msgid=%s, from=%s, to=%s code=%d", msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), retCode);
		return;
	}
	
	if (isCustomerService(msg.stoid()))
	{
		sendReq(&msg, MES_CHATCANCEL_DELIVER, imsvr::CSR);
	}
	else
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
		if (pLogin && pLogin->IsLogin())
		{
			sendReq(&msg, MES_CHATCANCEL_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			log("\t****send MES_CHATCANCEL_DELIVER(0x%x) %s to %s", MES_CHATCANCEL_DELIVER, msg.smsgid().c_str(), msg.stoid().c_str());
		}
	}
	DbgLog("process handle end msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
}


//收到消息后，做简单的消息合法性检查，组成任务放到线程池中
void CMsgHandler::OnMsgChatCancel(std::shared_ptr<CImPdu> pPdu)
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

	assert(NULL != pPdu);
	std::shared_ptr<im::MESChatCancel> pMsg(new im::MESChatCancel);
	if (!pMsg) return;
	MESChatCancel& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

	if (msg.sfromid().empty())
	{
		ErrLog("no fromId specified");
		return;
	}

	if (msg.stoid().empty() || msg.smsgid().empty())
	{
		ErrLog("no toId or msgId specified");
		return;
	}
	DbgLog("process add task msgId=%s",msg.smsgid().c_str());
	CCommonTaskMgr::InsertCommonTask(pMsg, OnMsgChatCancelStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));
	log("msgChatCancel (0x%x) %s prehandled , %s-->%s, use %llu usecond \n", pPdu->GetCommandId(), msg.smsgid().c_str(),
		msg.sfromid().c_str(), msg.stoid().c_str(), elspsedTimer.elapsed());
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMsgHandler::HandleMsgChatDeliverAckTask(const im::MESChatDeliveredAck& msg)
{
	DbgLog("process handle begin msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
	CUsecElspsedTimer timer;
	timer.start();	

	//尽量保证在发送MES_CHAT_DELIVERED_NOTIFICATION之前，mongo消息已经存入
	bool bInsertSuccess = m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(msg));

	if (isCustomerService(msg.stoid()))
	{
		sendReq(&msg, MES_CHAT_DELIVERED_NOTIFICATION, imsvr::CSR);
	} 
	else
	{
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
		if (pLogin && pLogin->IsLogin())
		{
			sendReq(&msg, MES_CHAT_DELIVERED_NOTIFICATION, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			log("****send MES_CHAT_DELIVERED_NOTIFICATION(0x%x) %s to %s",
				MES_CHAT_DELIVERED_NOTIFICATION, msg.smsgid().c_str(), msg.stoid().c_str());
		}
	}

	//bool bInsertSuccess = m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(msg));
	if (bInsertSuccess)
	{
		m_offlineMsgMgr.DelOfflineMsg(msg.sfromid(), MES_CHAT_DELIVER, msg.smsgid());
	}
	log("MESChatDeliveredAck %s handle in thread %llu finish,dir %s --> %s, use %llu usecond\r\n",
		pthread_self(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), timer.elapsed());
	DbgLog("process handle end msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
}

void OnMsgChatDeliverAckStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pChatTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CMsgHandler* pHandle = (CMsgHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::MESChatDeliveredAck> pChat = dynamic_pointer_cast<im::MESChatDeliveredAck>(pChatTask);
	//const im::MESChatDeliveredAck& msg = *pChat;

	pHandle->HandleMsgChatDeliverAckTask(*pChat);
	delete pCallBackPara;
}

void CMsgHandler::onMsgChatDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	std::shared_ptr<im::MESChatDeliveredAck> pMsg(new im::MESChatDeliveredAck);
	if (!pMsg) return;
	MESChatDeliveredAck& msg = *pMsg;
	//MESChatDeliveredAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

	if (msg.sfromid().empty() || msg.stoid().empty() || msg.smsgid().empty() )
	{
		ErrLog("!!!MESChatDeliveredAck(0x%x) %s lack of required parameter,dir %s --> %s\r\n",
			pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());
		return;
	}
	DbgLog("process add task msgId=%s",msg.smsgid().c_str());
	CCommonTaskMgr::InsertCommonTask(pMsg, OnMsgChatDeliverAckStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()), BKDRHash(msg.stoid().c_str()));
	log("MESChatDeliveredAck(0x%x) %s prehandled ,dir %s --> %s",
		pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//void OnMsgChatDeliverNotifAckStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pTask, void* paras)
//{
//	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
//	CMsgHandler* pHandle = (CMsgHandler*)pCallBackPara->m_handle;
//	if (NULL == pHandle)
//	{
//		delete pCallBackPara;
//		return;
//	}
//
//	const std::shared_ptr<im::MESChatDeliveredNotificationAck> pMsg = dynamic_pointer_cast<im::MESChatDeliveredNotificationAck>(pTask);
//
//	pHandle->HandleMsgChatDeliverNotifAckTask(*pMsg);
//	delete pCallBackPara;
//}
//
//void CMsgHandler::HandleMsgChatDeliverNotifAckTask(const im::MESChatDeliveredNotificationAck& msg)
//{
//	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_CHAT_DELIVERED_NOTIFICATION, msg.smsgid());
//}

void CMsgHandler::onMsgChatDeliverNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	//MESChatDeliveredNotificationAck msg;
	std::shared_ptr<im::MESChatDeliveredNotificationAck> pMsg(new im::MESChatDeliveredNotificationAck);
	if (!pMsg) return;
	MESChatDeliveredNotificationAck& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	

	if (msg.suserid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}

	//CCommonTaskMgr::InsertCommonTask(pMsg, OnMsgChatDeliverNotifAckStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()), BKDRHash(msg.suserid().c_str()));
	log("MESChatDeliveredNotificationAck(0x%x) %s received",	pPdu->GetCommandId(), msg.smsgid().c_str());

	//delete addfriendDeliver msg from mongo 
	//COfflineMsgMgr offlineMsgMgr;
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_CHAT_DELIVERED_NOTIFICATION, msg.smsgid(),NULL,NULL);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void OnMsgReadStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CMsgHandler* pHandle = (CMsgHandler*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::MESChatRead> pChatRead = dynamic_pointer_cast<im::MESChatRead>(pTask);
	pHandle->HandleMsgChatReadTask(*pChatRead,pCallBackPara->m_sessionID);
	delete pCallBackPara;
}

void CMsgHandler::HandleMsgChatReadTask(const im::MESChatRead& msg, const UidCode_t& sessionId)
{
	DbgLog("process handle begin msgId=%s thread=%lu", msg.smsgid().c_str(), pthread_self());
	COfflineMsg offmsg(msg);
	bool bInsertSuccess = m_offlineMsgMgr.InsertOfflineMsg(offmsg);

	/*send addFriendAnsAck to sender*/
	OnMsgReadDeliverInserted(msg,bInsertSuccess, sessionId);

	//if receiver was online, send addFriendDeliver to receiver
	if (!bInsertSuccess) return;

	std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
	if (pLogin && pLogin->IsLogin())
	{
		sendReq(&msg, MES_CHAT_READ_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
		log("****send MES_CHAT_READ_DELIVER(0x%x) %s to %s",
			MES_CHAT_READ_DELIVER, msg.smsgid().c_str(), msg.stoid().c_str());
	}
}

void CMsgHandler::OnMsgRead(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	//MESChatRead msg;
	std::shared_ptr<im::MESChatRead> pMsg(new im::MESChatRead);
	if (!pMsg) return;
	MESChatRead& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

	if (msg.sfromid().empty() || msg.stoid().empty() || msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
	DbgLog("process add task msgId=%s",msg.smsgid().c_str());
	CCommonTaskMgr::InsertCommonTask(pMsg, OnMsgReadStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()), BKDRHash(msg.stoid().c_str()));
	log("msgChatRead(0x%x) %s prehandled , %s-->%s", pPdu->GetCommandId(), msg.smsgid().c_str(),
		msg.sfromid().c_str(), msg.stoid().c_str());
}


void CMsgHandler::OnMsgReadDeliverInserted(const COfflineMsg& Msg, bool bInsertSuccess, const UidCode_t& sessionID)
{
	MESChatReadAck chatReadAck;
	chatReadAck.set_smsgid(Msg.GetMsgId());
	chatReadAck.set_suserid(Msg.GetFromId());
	chatReadAck.set_errcode(bInsertSuccess ? NON_ERR : EXCEPT_ERR);

	sendAck(&chatReadAck, MES_CHAT_READ_ACK, sessionID);
	log("****send MESChatReadAck(0x%x) %s to %s",
		MES_CHAT_READ_ACK, chatReadAck.smsgid().c_str(), chatReadAck.suserid().c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//void OnMsgChatReadDeliverAckStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pTask, void* paras)
//{
//	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
//	CMsgHandler* pHandle = (CMsgHandler*)pCallBackPara->m_handle;
//	if (NULL == pHandle)
//	{
//		delete pCallBackPara;
//		return;
//	}
//
//	const std::shared_ptr<im::MESChatReadDelivereAck> pChatReadDeliverAck = dynamic_pointer_cast<im::MESChatReadDelivereAck>(pTask);
//
//	pHandle->HandleMsgChatReadDeliverAckTask(*pChatReadDeliverAck);
//	delete pCallBackPara;
//}
//
//void CMsgHandler::HandleMsgChatReadDeliverAckTask(const im::MESChatReadDelivereAck& msg)
//{
//	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_CHAT_READ_DELIVER, msg.smsgid());
//}

void CMsgHandler::OnMsgReadDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	//MESChatReadDelivereAck msg;
	std::shared_ptr<im::MESChatReadDelivereAck> pMsg(new im::MESChatReadDelivereAck);
	if (!pMsg) return;
	MESChatReadDelivereAck& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("MESChatReadDelivereAck(0x%x) %s preHanded", pPdu->GetCommandId()/*MES_CHAT_READ_DELIVER_ACK*/, msg.smsgid().c_str());

	if (msg.suserid().empty() || msg.smsgid().empty() )
	{
		ErrLog("!!!lack of required parameter");
		return;
	}

	//CCommonTaskMgr::InsertCommonTask(pMsg, OnMsgChatReadDeliverAckStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()), BKDRHash(msg.suserid().c_str()));

	//delete addfriendDeliver msg from mongo 
	//COfflineMsgMgr offlineMsgMgr;
	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_CHAT_READ_DELIVER, msg.smsgid(),NULL,NULL);
}

bool CMsgHandler::RegistPacketExecutor(void)
{
	CmdRegist(MES_CHAT,								m_nNumberOfInst,  CommandProc(&CMsgHandler::OnMsgChat));
	CmdRegist(MES_CHATCANCEL,						m_nNumberOfInst,  CommandProc(&CMsgHandler::OnMsgChatCancel));
	CmdRegist(MES_CHAT_DELIVER_ACK,					m_nNumberOfInst,  CommandProc(&CMsgHandler::onMsgChatDeliverAck));
	CmdRegist(MES_CHAT_DELIVERED_NOTIFICATION_ACK,	m_nNumberOfInst,  CommandProc(&CMsgHandler::onMsgChatDeliverNotifyAck));
	CmdRegist(MES_CHAT_READ,						m_nNumberOfInst,  CommandProc(&CMsgHandler::OnMsgRead));
	CmdRegist(MES_CHAT_READ_DELIVER_ACK,			m_nNumberOfInst,  CommandProc(&CMsgHandler::OnMsgReadDeliverAck));
	CmdRegist(ANDROID_PUSH_ACK,						m_nNumberOfInst,  CommandProc(&CMsgHandler::OnAndPushAck));
	CmdRegist(ANDROID_NOTIFY,						m_nNumberOfInst,  CommandProc(&CMsgHandler::OnAndNotify));
	CmdRegist(APNS_PUSH_ACK,						m_nNumberOfInst, CommandProc(&CMsgHandler::OnIPushAck));
	CmdRegist(APNS_NOTIFY,							m_nNumberOfInst, CommandProc(&CMsgHandler::OnINotify));
	// 消息通知 by Abner
//	CmdRegist(MS_COMMONNOTIFY,						m_nNumberOfInst, CommandProc(&CMsgHandler::OnCommonNotifyMsg));
//	CmdRegist(MS_COMMONNOTIFY_DELIVER_ACK,			m_nNumberOfInst, CommandProc(&CMsgHandler::OnCommonNotifyMsgDeliverAck));
	
	return true;
}

void CMsgHandler::OnAndPushAck(std::shared_ptr<CImPdu> pPdu)
{
	NOTUSED_ARG(pPdu);
}

void CMsgHandler::OnAndNotify(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	ANDNotify msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("ANDNotify(0x%x) %s received, to %s, code = %x", 
		pPdu->GetCommandId()/*ANDROID_NOTIFY*/, msg.smsgid().c_str(), msg.stoid().c_str(), msg.nerr());

	ANDNotifyAck andNotifyAck;
	andNotifyAck.set_smsgid(msg.smsgid());
	andNotifyAck.set_stoid(msg.stoid());
	andNotifyAck.set_nerr(NON_ERR);
	sendAck(&andNotifyAck, ANDROID_NOTIFY_ACK, pPdu->GetSessionId(),1);
	//sendReq(&andNotifyAck, ANDROID_NOTIFY_ACK, imsvr::APUSH);
	DbgLog("****send ANDNotifyAck(0x%x) %s", ANDROID_NOTIFY_ACK, andNotifyAck.smsgid().c_str());
}

void CMsgHandler::OnIPushAck(std::shared_ptr<CImPdu> pPdu)
{
	NOTUSED_ARG(pPdu);
}

void CMsgHandler::OnINotify(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	PSvrAPNsRespone msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("PSvrAPNsRespone(0x%x) %s received, to %s, code = %x",
		pPdu->GetCommandId(), msg.smsgid().c_str(), msg.stoid().c_str(), msg.nerr());

	PSvrAPNsNotifyAck NotifyAck;
	NotifyAck.set_smsgid(msg.smsgid());
	NotifyAck.set_stoid(msg.stoid());
	NotifyAck.set_nerr(NON_ERR);
	sendAck(&NotifyAck, APNS_NOTIFY_ACK, pPdu->GetSessionId(), 1);
	DbgLog("****send PSvrAPNsNotifyAck(0x%x) %s", APNS_NOTIFY_ACK, NotifyAck.smsgid().c_str());
}

// 通知消息处理 2018-8-16 Abner
/*void OnCommonNotifyMsgStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pCommonNotifyMsgTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CMsgHandler* pHandle = (CMsgHandler*)pCallBackPara->m_handle;
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


void CMsgHandler::OnCommonNotifyMsg(std::shared_ptr<CImPdu> pPdu)
{
	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

	assert(NULL != pPdu);
	std::shared_ptr<im::MSGCommonNotify> pMsg(new im::MSGCommonNotify);
	if (!pMsg) 
		return;
	im::MSGCommonNotify& msg = *pMsg;

	uchar_t* pMsgBody = pPdu->GetBodyData();
	uint16_t nMsgBodyLen = pPdu->GetBodyLength();
	if (nullptr == pMsgBody || !msg.ParseFromArray(pMsgBody, nMsgBodyLen))
	{
		ErrLog("msg content parese fail, msg body length %d", nMsgBodyLen);
		return;
	}

	if (msg.sfromid().empty())
	{
		ErrLog("no fromId specified");
		return;
	}

	if (msg.stoids_size() <= 0 || msg.smsgid().empty())
	{
		ErrLog("no toId or msgId specified");
		return;
	}

	CCommonTaskMgr::InsertCommonTask(pMsg, OnCommonNotifyMsgStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));
	log("commonNotifyMsg (0x%x) %s prehandled , %s-->%s, use %llu usecond \n", pPdu->GetCommandId(), msg.smsgid().c_str(),
		msg.sfromid().c_str(), msg.stoid().c_str(), elspsedTimer.elapsed());
}

void CMsgHandler::HandleCommonNotifyMsgTask(const im::MSGCommonNotify& msg, const UidCode_t& sessionId)
{
	UserInfoModifyNotify(msg, sessionId);
}

void CMsgHandler::UserInfoModifyNotify(const im::MSGCommonNotify& msg, const UidCode_t& sessionId)
{
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
		im::ErrCode retCode = GetErrorCodeForChatToFriend(msg.sfromid(), strToid);
		if (NON_ERR != retCode) 
		{
			WarnLog("common notify msg %s from %s to %s not friend", msg.smsgid().c_str(), msg.sfromid().c_str(), strToid.c_str());
			continue;
		}
		deliverMsg.set_stoid(strToid);
		vecMsgCommNotifyDeliver.push_back(deliverMsg);
		vecOfflineMsg.push_back(COfflineMsg(deliverMsg));
	}

	bool bInsertSuccess = m_offlineMsgMgr.InsertOfflineMsg(vecOfflineMsg);
	sendCommonNotifyMsgAck(msg, (bInsertSuccess ? NON_ERR : EXCEPT_ERR), sessionId);
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
void CMsgHandler::OnCommonNotifyMsgDeliverAck(std::shared_ptr<CImPdu> pPdu)
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

void CMsgHandler::sendCommonNotifyMsgAck(const im::MSGCommonNotify& msg, im::ErrCode retCode, const UidCode_t& sessionId)
{
	im::MSGCommonNotifyACK CommonNotifyACK;
	CommonNotifyACK.set_smsgid(msg.smsgid());
	CommonNotifyACK.set_suserid(msg.sfromid());
	CommonNotifyACK.set_errcode(retCode);
	sendAck(&CommonNotifyACK, MS_COMMONNOTIFY_ACK, sessionId);
	log("****send MSGCommonNotifyACK(0x%x) %s to %s, code = 0X%X", MS_COMMONNOTIFY_ACK,
			CommonNotifyACK.smsgid().c_str(), CommonNotifyACK.suserid().c_str(), retCode);
}*/




