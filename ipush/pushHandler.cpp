/*****************************************************************************************
Filename: apnspushserver.h
Author: jack			Version: im-1.0 		Date:2017/06/14
Description: 	用户登录/登出、权限检查、用户信息管理类实现
 *****************************************************************************************/

#include "pushHandler.h"
#include "protohandle.h"
#include "CApnsPostData.h"
#include "ssl_post_mgr.h"

CLock *CPushHandler::m_pLockSendResp =  new CLock();
//CLock *CPushHandler::m_pLockPostMutex = new CLock();

CPushHandler::CPushHandler(CConfigFileReader* pConfigReader, int nNumOfInst) :
	m_pConfigReader(pConfigReader), m_nNumberOfInst(nNumOfInst)
{
	//m_pRecvProtoBuf = nullptr;
	//m_pPostPduCacheMgr = new CPostPduCacheMgr(OnApnsPushCacheCallBack, this);
}

CPushHandler::~CPushHandler()
{
	//if (m_pRecvProtoBuf)
	//{
	//	delete m_pRecvProtoBuf;
	//	m_pRecvProtoBuf = nullptr;
	//}
	
}

bool CPushHandler::Initialize(void)  
{
	RegistPacketExecutor();

	m_pManage =  CAppPushserverManager::GetInstance();
	if (!m_pManage)
	{
		ErrLog("CPushHandler Initialize");
		return false;
	}
//!!!###这里有问题，多个线程调用，后面的值把前面的值覆盖了
	if (!m_pManage->RegistSvr(this))
	{
		ErrLog("CPushHandler StartApp");
		return false;
	}

	StartThread();

	return true;
}

bool CPushHandler::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{

	CmdRegist(APNS_PUSH, m_nNumberOfInst, CommandProc(&CPushHandler::OnApnsPush));
	CmdRegist(APNS_NOTIFY_ACK, m_nNumberOfInst, CommandProc(&CPushHandler::OnApnsNotifyAck));
	return true;
}

//debuf
static int testcountErr = 0;
bool CPushHandler::OnApnsPush(std::shared_ptr<CImPdu> pPdu)
{
	static int tt=0;
	bool bRet = false;

	PSvrMsg msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return bRet;
	}

	if (msg.smsgid().empty() || msg.stotoken().empty() || msg.sbody().empty())
	{
		WarnLog("apns Request lack of necessary field,msgId = %s, stotoken= %s, sbody = %s",
			msg.smsgid().c_str(), msg.stotoken().c_str(), msg.sbody().c_str());
		return bRet;
	}
	InfoLog("recv ApnsPush:%s %d times, stotoken= %s, stoId = %s",
		msg.smsgid().c_str(), ++tt, msg.stotoken().c_str(), msg.stoid().c_str());
	
	ErrCode eCode = NON_ERR;

	DbgLog("ios push vesion_code=%d, devType=%u", msg.versioncode(), msg.edivece_type());
    if(msg.edivece_type() == APNS) {
		if(!m_pManage->GetClient(PUSH_CLIENT_TYPE_VOIP_PRODUCTION) || !m_pManage->GetClient(PUSH_CLIENT_TYPE_VOIP_PRODUCTION)->AddTask(msg))
        {
            WarnLog("###post push to voip_pro_client false");
            eCode = EXCEPT_ERR;
            goto toAck;
        } 
   } else if(msg.edivece_type() == APNS_DEV) {
        if (!m_pManage->GetClient(PUSH_CLIENT_TYPE_VOIP_DEV) || !m_pManage->GetClient(PUSH_CLIENT_TYPE_VOIP_DEV)->AddTask(msg)) {
            WarnLog("###post push to voip_dev_client false");
            eCode = EXCEPT_ERR;
            goto toAck;
        }
    }   
	bRet = true;
toAck:
	if (eCode == EXCEPT_ERR)
	{
		testcountErr++;
		InfoLog("testcountErr:%d\n", testcountErr);
	}

	OnApnsPushAck(msg.smsgid(), pPdu->GetSessionId(), eCode);

	return bRet;
}

void CPushHandler::OnApnsPushAck(string sMsgId,UidCode_t sSessionId,ErrCode bCode)
{
	if (!CPushHandler::m_pLockSendResp)
	{
		CPushHandler::m_pLockSendResp  = new CLock;
	}
	
    PSvrMsgAck 	apnsPushAck;
	apnsPushAck.set_smsgid(sMsgId);
	apnsPushAck.set_nerr(bCode);


	CImPdu 		apnsPushAckPdu;
	apnsPushAckPdu.SetPBMsg(&apnsPushAck);
	apnsPushAckPdu.SetCommandId(APNS_PUSH_ACK);
	apnsPushAckPdu.SetSessionId(sSessionId);

	if(SendPdu(&apnsPushAckPdu, 0) < 0)
	{
		ErrLog("OnApnsPushAck:%d\n", testcountErr);
	}
	
}

bool CPushHandler::OnApnsNotify(shared_ptr<CApnsPostData> pData)
{

	if (!pData)
	{
		ErrLog("pData is nullptr");
		return false;
	}

	//err code
	int errCode = EXCEPT_ERR;

	//200 success
	if (pData->apnsRetStatus == "200")
	{
		errCode = NON_ERR;
	}
	
	/*
	if (!CPushHandler::m_pLockSendResp)
	{
		CPushHandler::m_pLockSendResp  = new CLock;
	}
	*/
	//CAutoLock autolock(CPushHandler::m_pLockSendResp);

	PSvrAPNsRespone 	apnsNotify;
	CImPdu 				apnsNotifyPdu;

	apnsNotify.set_smsgid(pData->sMsgId);
	apnsNotify.set_stoid(pData->sToId);
	apnsNotify.set_nerr((ErrCode)errCode);

	apnsNotifyPdu.SetPBMsg(&apnsNotify);
	apnsNotifyPdu.SetCommandId(APNS_NOTIFY);
	//apnsNotifyPdu.SetSessionId(pData->sessionId);

	if (SendPdu(imsvr::MSG, &apnsNotifyPdu)< 0)
	{
		ErrLog("send notify %s:%s value %s failed\n", pData->sMsgId.c_str(), pData->sToId.c_str(), pData->apnsRetStatus.c_str());
	}
	else
	{
		InfoLog("send notify %s:%s value %s success\n", pData->sMsgId.c_str(), pData->sToId.c_str(), pData->apnsRetStatus.c_str());
	}
	

/* 
	//success or send time out 
	if (pData->apnsRetStatus == "200" || pData->apnsRetStatus == "600")
	{
		errCode = NON_ERR;	
	}
*/
	return true;
}

void CPushHandler::OnApnsNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	//do nothing 
	NOTUSED_ARG(pPdu);
	return;
}

//
