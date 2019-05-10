/******************************************************************************
Filename: basehandle.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description: 
******************************************************************************/
#include "basehandle.h"
#include "configfilereader.h"
#include "im.mes.pb.h"
#include "util.h"
#include "im_time.h"
#include "thread_pool_manager.h"
#include "im.push.android.pb.h"
#include "im.pushSvrAPNsMsg.pb.h"

using namespace im;
using namespace std;

CBaseHandle::CBaseHandle(CConfigFileReader* pConfigReader)
	: m_pConfigReader(pConfigReader)
	, m_nChnnMsgSendPoolSize(60)
	, m_nInsertMsgSendPoolSize(20)
	, m_chnnMemListUrl("")
	, m_sAppSecret("")
{

}

CBaseHandle::~CBaseHandle()
{
	
}


int CBaseHandle::sendAck(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const UidCode_t& sessionId, int nDirection)
{
	CImPdu pdu;
	pdu.SetPBMsg(pMsg);
	pdu.SetSessionId(sessionId);
	pdu.SetCommandId(command_id);
	int retCode = SendPdu(&pdu,nDirection);
	static int msgChatAckCount = 0;
	if (MES_CHAT_ACK == command_id)
	{
		++msgChatAckCount;
		DbgLog("sendMsgChatAck %d times", msgChatAckCount);
	}

	if (retCode <= 0)
	{
		WarnLog("!!!send command 0x%x failed, return %d", command_id, retCode);
	}
	return retCode;
}

int CBaseHandle::sendReq(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const string& ip, int port)
{
	DbgLog("send request to client ip:%s port:%d", ip.c_str(), port);
	CImPdu pdu;
	pdu.SetPBMsg(pMsg);
	pdu.SetCommandId(command_id);
	int retCode = SendPdu(ip, port, &pdu);
	if (retCode <= 0)
	{
		WarnLog("!!!send command 0x%x failed, return %d", command_id, retCode);
	}
	return retCode;
}

int CBaseHandle::sendReq(const google::protobuf::MessageLite* pMsg, uint16_t command_id, int16_t nServiceId)
{
	CUsecElspsedTimer myTimer;
	myTimer.start();
	CImPdu pdu;
	pdu.SetPBMsg(pMsg);
	pdu.SetCommandId(command_id);
	int retCode = SendPdu(nServiceId, &pdu);
	if (retCode <= 0)
	{
		WarnLog("!!!send command 0x%x to svr %d failed, return %d", command_id, nServiceId, retCode);
	}
	return retCode;
}

void CBaseHandle::sendPush(const string& fromId, const string& toId, const string& msgId, 
							im::MsgType msgType, const string& content, int pushType, const string& pushToken, int vesionCode)
{
	if(pushToken.empty())
	{
		WarnLog("!!!get user %s token failed, can't send push\r\n", toId.c_str());
		return;
	}

	if (pushType != APNS && pushType != APNS_DEV)		//安卓推送
	{
		ANDPushMsg aPush;
		aPush.set_emsgtype(msgType);
		aPush.set_smsgid(msgId);
		aPush.set_stitle(getAppName());
		aPush.set_sbody(content);
		aPush.set_stoid(toId);
		aPush.set_sdivece_token(pushToken);
		aPush.set_edivece_type(im::DiveceType(pushType));

		DbgLog("****send request 0x%x to andpush for use %s to %s, devToken = %s, msgId = %s, msgType = %d",
			ANDROID_PUSH, fromId.c_str(), toId.c_str(), pushToken.c_str(), msgId.c_str(), msgType);
		sendReq(&aPush, ANDROID_PUSH, imsvr::APUSH);
	}
	else						//苹果推送
	{
		PSvrMsg iPush;
		iPush.set_emsgtype((MsgType)msgType);
		iPush.set_smsgid(msgId);
		iPush.set_sbody(content);
		iPush.set_sfromid(fromId);
		iPush.set_stoid(toId);
		iPush.set_stotoken(pushToken);
		iPush.set_nunreadnotifymsgcount(0);
		iPush.set_versioncode(vesionCode);
		iPush.set_edivece_type(im::DiveceType(pushType));
		sendReq(&iPush, APNS_PUSH, imsvr::IPUSH);

		DbgLog("****send request to ipush for use %s to %s, devToken = %s, msgId = %s, msgType = %d appVersion=%d",
			fromId.c_str(), toId.c_str(), pushToken.c_str(), msgId.c_str(), msgType, vesionCode);
	}
}

std::string CBaseHandle::getAppName() const
{
	static string appName("");
	if (appName.empty())
	{
		if (NULL == m_pConfigReader) 
			return "unknown";
		char* str_userId = m_pConfigReader->GetConfigName("appName");
		if (NULL == str_userId)
			return "notDefined";
		appName = string(str_userId);
	}
	return  appName;
}


bool CBaseHandle::Initialize(void)
{
	char* cache_size = m_pConfigReader->GetConfigName("chnn_msg_pool_count");
	if(cache_size)
		m_nChnnMsgSendPoolSize = atoi(cache_size);
	CThreadPoolManager::getInstance()->initSendChannelMsgPool(m_nChnnMsgSendPoolSize, "sendChannelMsg");

	cache_size = m_pConfigReader->GetConfigName("insert_msg_pool_count");
	if(cache_size)
		m_nInsertMsgSendPoolSize = atoi(cache_size);
	CThreadPoolManager::getInstance()->initInsertChannelMsgPool(m_nInsertMsgSendPoolSize, "insertChannelMsg");

	char* pData = m_pConfigReader->GetConfigName("chnnMemberListUrl");
	if(pData)
		m_chnnMemListUrl = pData;
	pData = m_pConfigReader->GetConfigName("check_appsecret");
	if(pData)
		m_sAppSecret = pData;
	
	bool retcode = RegistPacketExecutor();
	if (retcode)
		StartThread();

	return retcode;
}
