#include <algorithm>
#include <list>
#include "groupchat.h"
#include "im.mes.pb.h"
#include "im.group.pb.h"
#include "im_grpmem.h"
#include "redisGrpMgr.h"
#include "grpOfflineMsg.h"
#include "im_time.h"
#include "groupnotify.h"
#include "mongoDbManager.h"
#include "mysqlPool.h"
#include "redisPool.h"
#include "im_grpmem.h"
#include "mysqlGrpmemMgr.h"
#include "pdusender.h"
#include "commonTaskMgr.h"

using namespace im;
using namespace std;

CGroupChat::CGroupChat(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader), m_grpOfflineMsgMgr(), m_grpOfflineMsgMgrForLegalChat(), m_nNumberOfInst(nNumOfInst)
{

}

CGroupChat::~CGroupChat()
{

}


void GrpChatToMsg(std::map<std::string, CGrpMem>& grpmems, const im::GroupChat& msg, void* paras)
{
	//CGroupChat* pHandle = (CGroupChat*)paras;
	//if (NULL == pHandle)
	//{
	//	return;
	//}

	_MESGrpInterChat grpChat;
	grpChat.set_sfromid(msg.sfromid());
	grpChat.set_sgrpid(msg.sgroupid());
	grpChat.set_smsgid(msg.smsgid());
	grpChat.set_msgtime(msg.msgtime());
	grpChat.set_encrypt(msg.encrypt());
	grpChat.set_scontent(msg.scontent());
	grpChat.set_nnotifycount(msg.nnotifycount());

	int notifySize = msg.snotifyusers_size();
	int index = 0;
	while (index < notifySize)
	{
		string* pNotifyUserId = grpChat.add_snotifyusers();
		*pNotifyUserId = msg.snotifyusers(index);
		++index;
	}

	std::map<std::string, CGrpMem>::const_iterator itSendUser = grpmems.find(msg.sfromid());
	std::map<std::string, CGrpMem>::const_iterator iter = grpmems.begin();

	while (iter != grpmems.end())
	{
		if (iter == itSendUser || (int)CGrpMem::GRP_MEM_STATE_QUIT_MEMBER == iter->second.GetState())
		{
			++iter;
			continue;
		}

		grpChat.add_stoid(iter->second.GetMemId());

		if (grpChat.stoid_size() >= 500)
		{
			int retCode = CPduSender::getInstance()->sendReq(&grpChat, MES_GRPINTERCHAT, imsvr::MSG);
			if (retCode < 0)
			{
				//添加任务，重发
				ErrLog("!!!!!!grp %s dispatch grpChat(0x%x) %s failed", grpChat.sgrpid().c_str(),
					MES_GRPINTERCHAT, grpChat.smsgid().c_str());
			}
			grpChat.clear_stoid();
			DbgLog("comtask grp %s dispatch grpChat(0x%x) %s ", grpChat.sgrpid().c_str(),
				MES_GRPINTERCHAT, grpChat.smsgid().c_str());
		}
		++iter;
	}

	if (grpChat.stoid_size() > 0)
	{
		int retCode = CPduSender::getInstance()->sendReq(&grpChat, MES_GRPINTERCHAT, imsvr::MSG);
		if (retCode < 0)
		{
			//添加任务，重发
			ErrLog("!!!!!!grp %s dispatch grpChat(0x%x) %s failed", grpChat.sgrpid().c_str(),
				MES_GRPINTERCHAT, grpChat.smsgid().c_str());
		}
		DbgLog("comtask grp %s dispatch grpChat(0x%x) %s ", grpChat.sgrpid().c_str(),
			MES_GRPINTERCHAT, grpChat.smsgid().c_str());
	}
}

bool CGroupChat::OnGrpChatInserted(const CGrpOfflineMsg& offlineMsg, bool bInsertSuccess, const UidCode_t& sessionID)
{
	CUsecElspsedTimer timer;
	timer.start();

	std::map<std::string, CGrpMem> grpmems = GetGrpMems(offlineMsg.GetGrpId());
	uint64_t getGrpMemTime = timer.elapsed();

	//check user right for chat
	std::map<std::string, CGrpMem>::const_iterator itSendUser = grpmems.find(offlineMsg.GetFromId());
	if (itSendUser == grpmems.end())
	{
		std::shared_ptr<CGrpMem> pGrpMem = CMysqlGrpmemMgr::GetGrpmem(offlineMsg.GetGrpId(), offlineMsg.GetFromId());
		if (pGrpMem)
		{
			CReidsGrpMgr::InsertGrpMem(*pGrpMem);
			grpmems.insert(std::pair<std::string, CGrpMem>(offlineMsg.GetFromId(), *pGrpMem));
			itSendUser = grpmems.find(offlineMsg.GetFromId());
		}
		else
		{
			sendChatAck(offlineMsg, sessionID, ERR_CHAT_FORBIDDEN, "not grp mem");
			//删除插入的数据
			m_grpOfflineMsgMgr.DelOfflineMsg(offlineMsg.GetGrpId(), offlineMsg.GetMsgId(), NULL, NULL);
			//m_grpOfflineMsgMgr.InsertGrpOfflineMsg(CGrpOfflineMsg(msg), GrpChatInsertedCallBack, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));
			return true;
		}
	}

	if (itSendUser->second.IsFlagSet(CGrpMem::GRP_MEM_STATE_QUIT_MEMBER))
	{
		sendChatAck(offlineMsg, sessionID, ERR_CHAT_FORBIDDEN, "has no right send grp msg");
		m_grpOfflineMsgMgr.DelOfflineMsg(offlineMsg.GetGrpId(), offlineMsg.GetMsgId(), NULL, NULL);
		return true;
	}

	if (!bInsertSuccess)
	{
		sendChatAck(offlineMsg, sessionID, EXCEPT_ERR, "insert failed");
		return true;
	}
	sendChatAck(offlineMsg, sessionID, NON_ERR, "");
	uint64_t sendAckTime = timer.elapsed();
	//发送0xb071到消息服务器
	GroupChat msg;
	msg.ParseFromString(offlineMsg.GetMsgData());
	GrpChatToMsg(grpmems, msg, this);

	log("thread %lu get grpMem use %llu useconds, and send ack at %llu usec , send 0xb071 to msg at %llu usec",pthread_self(), getGrpMemTime, sendAckTime, timer.elapsed());
	//return CCommonTaskMgr::InsertCommonTask(std::shared_ptr<::google::protobuf::MessageLite>(new im::GroupChat(msg)), GrpChatCheckedLegalCallBack, this);
	DbgLog("process msg[%s] end",msg.smsgid().c_str());
	return true;
}

void GrpChatInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, bool bInsertSuccess, void* paras)
{
	if (NULL == paras) return;
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CGroupChat* pHandle = (CGroupChat*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}
	const std::shared_ptr<CGrpOfflineMsg> pMsg = dynamic_pointer_cast<CGrpOfflineMsg>(pOfflineMsg);
	pHandle->OnGrpChatInserted(*pMsg, bInsertSuccess, pCallBackPara->m_sessionID);

	delete pCallBackPara;		//回调函数负责释放资源
}

bool CGroupChat::OnChat(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);

	CUsecElspsedTimer timer;
	timer.start();

	GroupChat msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return false;
	}
	

	//暂时以客户端时间为准
    //if (!msg.msgtime())
    //{
    //	const uint64_t msgTime = getCurrentTime();
    //	msg.set_msgtime(msgTime);
    //}
    msg.set_msgtime(getCurrentTime());//set server time

	static int msgChatCount = 0;
	++msgChatCount;
	

	if (msg.sgroupid().empty() || msg.sfromid().empty())
	{
		WarnLog("grpChat-%d (0x%x) %s, %s-->%s, grpId or fromId not specified\r\n", msgChatCount, pPdu->GetCommandId(), msg.smsgid().c_str(),
			msg.sfromid().c_str(), msg.sgroupid().c_str());
		return false;
	}
	uint64_t paraTime = timer.elapsed();

	if (!IsGrpIdValid(msg.sgroupid()))
	{
		WarnLog("grpChat-%d (0x%x) %s, %s-->%s, invalid grpId specified\r\n", msgChatCount, pPdu->GetCommandId(), msg.smsgid().c_str(),
			msg.sfromid().c_str(), msg.sgroupid().c_str());
		return false;
	}
#if 0
	//send addFriendAck to sender
	sendChatAck(msg, pPdu->GetSessionId(), NON_ERR, "content is illegal");
	return true;
#endif
	uint64_t checkTime = timer.elapsed();
	im::ErrCode retCode = msg.scontent().empty() ? ERR_CHAT_UNHEALTHY : NON_ERR;
	if (retCode != NON_ERR)
	{
		//send addFriendAck to sender
		sendChatAck(msg, pPdu->GetSessionId(), retCode, "content is illegal");
		return true;
	}

	DbgLog("process msg[%s] begin",msg.smsgid().c_str());
	m_grpOfflineMsgMgr.InsertGrpOfflineMsg(CGrpOfflineMsg(msg), GrpChatInsertedCallBack, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));

	log("grpChat-%d (0x%x) %s pre handled, %s-->%s, content %s, para at %llu, ckeckfinish at %llu, use %llu useconds\r\n", msgChatCount, pPdu->GetCommandId(), msg.smsgid().c_str(),
		msg.sfromid().c_str(), msg.sgroupid().c_str(), msg.scontent().c_str(), paraTime, checkTime, timer.elapsed());
	return true;
}

void GrpChatCancelToMsg(std::map<std::string, CGrpMem>& grpmems, const im::GroupChatCancel& msg, void* paras)
{
	_MESGrpInterChatCancel grpChatCancel;
	grpChatCancel.set_sfromid(msg.sfromid());
	grpChatCancel.set_sgrpid(msg.sgroupid());
	grpChatCancel.set_smsgid(msg.smsgid());
	grpChatCancel.set_msgtime(msg.msgtime());		//主要用在对方离线并且原始消息没收到的时候,因为原始消息被删除了，故无法直接在原始消息那里替换


	std::map<std::string, CGrpMem>::const_iterator itSendUser = grpmems.find(msg.sfromid());
	std::map<std::string, CGrpMem>::const_iterator iter = grpmems.begin();

	while (iter != grpmems.end())
	{
		if (iter == itSendUser || (int)CGrpMem::GRP_MEM_STATE_QUIT_MEMBER == iter->second.GetState())
		{
			++iter;
			continue;
		}

		grpChatCancel.add_stoid(iter->second.GetMemId());

		if (grpChatCancel.stoid_size() >= 500)
		{
			int retCode = CPduSender::getInstance()->sendReq(&grpChatCancel, MES_GRPINTER_CHATCANCLE, imsvr::MSG);
			if (retCode < 0)
			{
				//添加任务，重发
				ErrLog("!!!!!!grp %s dispatch grpChatCancel(0x%x) %s failed", grpChatCancel.sgrpid().c_str(),
					MES_GRPINTER_CHATCANCLE, grpChatCancel.smsgid().c_str());
			}
			grpChatCancel.clear_stoid();
			DbgLog("comtask grp %s dispatch grpChatCancel(0x%x) %s ", grpChatCancel.sgrpid().c_str(),
				MES_GRPINTER_CHATCANCLE, grpChatCancel.smsgid().c_str());
		}
		++iter;
	}

	if (grpChatCancel.stoid_size() > 0)
	{
		int retCode = CPduSender::getInstance()->sendReq(&grpChatCancel, MES_GRPINTER_CHATCANCLE, imsvr::MSG);
		if (retCode < 0)
		{
			//添加任务，重发
			ErrLog("!!!!!!grp %s dispatch grpChatCancel(0x%x) %s failed", grpChatCancel.sgrpid().c_str(),
				MES_GRPINTER_CHATCANCLE, grpChatCancel.smsgid().c_str());
		}
		DbgLog("comtask grp %s dispatch grpChatCancel(0x%x) %s ", grpChatCancel.sgrpid().c_str(),
			MES_GRPINTER_CHATCANCLE, grpChatCancel.smsgid().c_str());
	}
}

void OnMsgChatCancelStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pChatTask, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
	CGroupChat* pHandle = (CGroupChat*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	const std::shared_ptr<im::GroupChatCancel> pChat = dynamic_pointer_cast<im::GroupChatCancel>(pChatTask);
	//const im::GroupChatCancel& msg = *pChat;

	pHandle->HandleMsgChatCancelTask(*pChat, pCallBackPara->m_sessionID);
	delete pCallBackPara;
}

bool CGroupChat::OnChatCancle(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);

	CUsecElspsedTimer timer;
	timer.start();

	std::shared_ptr<im::GroupChatCancel> pMsg(new im::GroupChatCancel);
	GroupChatCancel& msg = *pMsg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return false;
	}

	if (msg.sgroupid().empty() || msg.sfromid().empty() || msg.smsgid().empty())
	{
		WarnLog("\r\n\r\n\r\ngrpChatCancel (0x%x) %s, %s-->%s, grpId or fromId not specified\r\n", pPdu->GetCommandId(), msg.smsgid().c_str(),
			msg.sfromid().c_str(), msg.sgroupid().c_str());
		return false;
	}
	uint64_t paraTime = timer.elapsed();

	if (!IsGrpIdValid(msg.sgroupid()))
	{
		WarnLog("\r\n\r\n\r\ngrpChatCancle (0x%x) %s, %s-->%s, invalid grpId specified\r\n", pPdu->GetCommandId(), msg.smsgid().c_str(),
			msg.sfromid().c_str(), msg.sgroupid().c_str());
		return false;
	}

	uint64_t checkTime = timer.elapsed();

	CCommonTaskMgr::InsertCommonTask(pMsg, OnMsgChatCancelStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));

	log("\r\n\r\n\r\ngrpChatCancel (0x%x) %s pre handled, %s-->%s, para at %llu, ckeckfinish at %llu, use %llu useconds\r\n", pPdu->GetCommandId(), msg.smsgid().c_str(),
		msg.sfromid().c_str(), msg.sgroupid().c_str(), paraTime, checkTime, timer.elapsed());
	return true;
}

std::shared_ptr<CGrpMem> GetGrpMem(std::map<std::string, CGrpMem>& grpmems, const string& grpId, const string & userId)
{
	std::map<std::string, CGrpMem>::const_iterator itSendUser = grpmems.find(userId);
	if (itSendUser == grpmems.end())
	{
		std::shared_ptr<CGrpMem> pGrpMem = CMysqlGrpmemMgr::GetGrpmem(grpId, userId);
		if (pGrpMem)
		{
			CReidsGrpMgr::InsertGrpMem(*pGrpMem);
			grpmems.insert(std::pair<std::string, CGrpMem>(userId, *pGrpMem));
		}
		return pGrpMem;
	}
	return std::shared_ptr<CGrpMem>(new CGrpMem(itSendUser->second));
}

bool CGroupChat::HandleMsgChatCancelTask(const GroupChatCancel& msg, const UidCode_t& sessionID)
{
	CUsecElspsedTimer timer;
	timer.start();

	std::map<std::string, CGrpMem> grpmems = GetGrpMems(msg.sgroupid());
	std::shared_ptr<CGrpMem> pUserMemInfo = GetGrpMem(grpmems, msg.sgroupid(), msg.sfromid());
	uint64_t getGrpMemTime = timer.elapsed();

	if (pUserMemInfo == NULL)
	{
		sendChatCancleAck(msg, sessionID, ERR_CHAT_FORBIDDEN, "not grp mem");
		return true;
	}

	if (pUserMemInfo->IsFlagSet(CGrpMem::GRP_MEM_STATE_QUIT_MEMBER))
	{
		sendChatCancleAck(msg, sessionID, ERR_CHAT_FORBIDDEN, "has no right send grp msg");
		return true;
	}

	sendChatCancleAck(msg, sessionID, NON_ERR, "");
	uint64_t sendAckTime = timer.elapsed();
	//发送0xb075到消息服务器
	GrpChatCancelToMsg(grpmems, msg, this);

	log("thread %lu handle grpchat %s get grp %s Mem use %llu useconds, and send ack at %llu usec , send 0xb075 to msg at %llu usec",
		pthread_self(), msg.smsgid().c_str(), msg.sgroupid().c_str(), getGrpMemTime, sendAckTime, timer.elapsed());

	return true;
}

//void GrpChatCheckedLegalCallBack(const std::shared_ptr<::google::protobuf::MessageLite>& pGrpChatTask, void* paras)
//{
//	CGroupChat* pHandle = (CGroupChat*)paras;
//	if (NULL == pHandle)
//	{
//		return;
//	}
//	const std::shared_ptr<im::GroupChat> pGrpChat = dynamic_pointer_cast<im::GroupChat>(pGrpChatTask);
//	const im::GroupChat& msg = *pGrpChat;
//
//	_MESGrpInterChat grpChat;
//	grpChat.set_sfromid(msg.sfromid());
//	grpChat.set_sgrpid(msg.sgroupid());
//	grpChat.set_smsgid(msg.smsgid());
//	grpChat.set_msgtime(msg.msgtime());
//	grpChat.set_encrypt(msg.encrypt());
//	grpChat.set_scontent(msg.scontent());
//	grpChat.set_nnotifycount(msg.nnotifycount());
//
//	int notifySize = msg.snotifyusers_size();
//	int index = 0;
//	while (index < notifySize)
//	{
//		string* pNotifyUserId = grpChat.add_snotifyusers();
//		*pNotifyUserId = msg.snotifyusers(index);
//		++index;
//	}
//
//	std::map<std::string, CGrpMem> grpmems = GetGrpMems(msg.sgroupid());
//	std::map<std::string, CGrpMem>::const_iterator itSendUser = grpmems.find(msg.sfromid());
//	std::map<std::string, CGrpMem>::const_iterator iter = grpmems.begin();
//
//	while (iter != grpmems.end())
//	{
//		if (iter == itSendUser || (int)CGrpMem::GRP_MEM_STATE_QUIT_MEMBER == iter->second.GetState())
//		{
//			++iter;
//			continue;
//		}
//		//const string& toId = iter->second.GetMemId();
//		//grpChat.set_stoid(toId);
//
//		grpChat.add_stoid(iter->second.GetMemId());
//
//		if (grpChat.stoid_size() >= 500)
//		{
//			int retCode = CPduSender::getInstance()->sendReq(&grpChat, MES_GRPINTERCHAT, imsvr::MSG);
//			if (retCode < 0)
//			{
//				//添加任务，重发
//				ErrLog("!!!!!!grp %s dispatch grpChat(0x%x) %s failed", grpChat.sgrpid().c_str(),
//					MES_GRPINTERCHAT, grpChat.smsgid().c_str());
//			}
//			grpChat.clear_stoid();
//			DbgLog("comtask grp %s dispatch grpChat(0x%x) %s ", grpChat.sgrpid().c_str(),
//				MES_GRPINTERCHAT, grpChat.smsgid().c_str());
//		}		
//		++iter;
//	}
//
//	if (grpChat.stoid_size() > 0)
//	{
//		int retCode = CPduSender::getInstance()->sendReq(&grpChat, MES_GRPINTERCHAT, imsvr::MSG);
//		if (retCode < 0)
//		{
//			//添加任务，重发
//			ErrLog("!!!!!!grp %s dispatch grpChat(0x%x) %s failed", grpChat.sgrpid().c_str(),
//				MES_GRPINTERCHAT, grpChat.smsgid().c_str());
//		}
//		DbgLog("comtask grp %s dispatch grpChat(0x%x) %s ", grpChat.sgrpid().c_str(),
//			MES_GRPINTERCHAT, grpChat.smsgid().c_str());
//	}
//}



bool CGroupChat::OngrpChatAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	_MESGrpInterChatAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return false;
	}
	if (msg.errcode() != NON_ERR)
	{
		WarnLog("****receive MESGrpChatAck(0x%x) %s to %s, code = 0X%X", MES_GRPINTERCHAT_ACK,
			msg.smsgid().c_str(), msg.suserid().c_str(), msg.errcode());
	}
	return true;
}

bool CGroupChat::OngrpChatCancelAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	_MESGrpInterChatCancelAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return false;
	}
	if (msg.errcode() != NON_ERR)
	{
		WarnLog("****receive MESGrpChatCancelAck(0x%x) %s to %s, code = 0X%X", MES_GRPINTER_CHATCANCLE_ACK,
			msg.smsgid().c_str(), msg.suserid().c_str(), msg.errcode());
	}
	return true;
}


void CGroupChat::sendChatCancleAck(const im::GroupChatCancel& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc /*= string("")*/)
{
	GroupChatCancelAck grpChatCancelAck;
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


void CGroupChat::sendChatAck(const GroupChat& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc)
{
	GroupChatAck grpChatAck;
	grpChatAck.set_smsgid(msg.smsgid());
	grpChatAck.set_sfromid(msg.sfromid());
	grpChatAck.set_sgroupid(msg.sgroupid());
	grpChatAck.set_sendtime(msg.msgtime());
	grpChatAck.set_errcode(retCode);
	sendAck(&grpChatAck, GROUP_CHAT_ACK, sessionId);
	if (retCode != NON_ERR)
	{
		WarnLog("****send GroupChatAck(0x%x) %s to %s, code = 0X%X,time %zd, %s", GROUP_CHAT_ACK,
			grpChatAck.smsgid().c_str(), grpChatAck.sfromid().c_str(), retCode, msg.msgtime(), desc.c_str());
	}
}

void CGroupChat::sendChatAck(const CGrpOfflineMsg& msg, const UidCode_t& sessionId, im::ErrCode retCode, const string& desc /*= string("")*/)
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



bool CGroupChat::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(GROUP_CHAT, m_nNumberOfInst, CommandProc(&CGroupChat::OnChat));
	CmdRegist(MES_GRPINTERCHAT_ACK, m_nNumberOfInst, CommandProc(&CGroupChat::OngrpChatAck));
	CmdRegist(GROUP_CHATCANCEL, m_nNumberOfInst, CommandProc(&CGroupChat::OnChatCancle));
	CmdRegist(MES_GRPINTER_CHATCANCLE_ACK, m_nNumberOfInst, CommandProc(&CGroupChat::OngrpChatCancelAck));
	return true;
}

