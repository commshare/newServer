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

using namespace im;
using namespace std;

CBaseHandle::CBaseHandle(CConfigFileReader* pConfigReader)
	: m_pConfigReader(pConfigReader)
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

int CBaseHandle::sendAck(const string& pMsg, uint16_t command_id, const UidCode_t& sessionId, int nDirection)
{
	CImPdu pdu;
//	pdu.SetPBMsg(pMsg);
    pdu.SetPBMsg(pMsg.c_str(), pMsg.length());
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

int CBaseHandle::sendReq(const string& pMsg, uint16_t command_id, const string& ip, int port)
{
	CImPdu pdu;
//	pdu.SetPBMsg(pMsg);
    pdu.SetPBMsg(pMsg.c_str(), pMsg.length());
	pdu.SetCommandId(command_id);
	int retCode = SendPdu(ip, port, &pdu);
	if (retCode <= 0)
	{
		WarnLog("!!!send command 0x%x failed, return %d", command_id, retCode);
	}
	return retCode;
}

int CBaseHandle::sendReq(const string& pMsg, uint16_t command_id, int16_t nServiceId)
{
	CUsecElspsedTimer myTimer;
	myTimer.start();
	CImPdu pdu;
//	pdu.SetPBMsg(pMsg);
    pdu.SetPBMsg(pMsg.c_str(), pMsg.length());
	pdu.SetCommandId(command_id);
	int retCode = SendPdu(nServiceId, &pdu);
	if (retCode <= 0)
	{
		WarnLog("!!!send command 0x%x to svr %d failed, return %d", command_id, nServiceId, retCode);
	}
	
    return retCode;
}

void CBaseHandle::sendPush(const string& fromId, const string& toId, const string& msgId, im::MsgType msgType, 
							const string& content, int pushType, const string& pushToken, const string& voipToken, int vesionCode)
{
	string strPushToken = pushToken;
	if(pushType == APNS || pushType == APNS_DEV)
		strPushToken = voipToken;
	if(strPushToken.empty())
	{
		WarnLog("!!!get user %s token failed, can't send push\r\n", toId.c_str());
		return;
	}

	if (pushType != APNS && pushType != APNS_DEV)		//安卓推送
	{
#if 0
		ANDPushMsg aPush;
		aPush.set_emsgtype(msgType);
		aPush.set_smsgid(msgId);
		aPush.set_stitle(getAppName());
		aPush.set_sbody(content);
		aPush.set_stoid(toId);
		aPush.set_sdivece_token(strPushToken);
		aPush.set_edivece_type(im::DiveceType(pushType));

		DbgLog("****send request 0x%x to andpush for use %s to %s, devToken = %s, msgId = %s, msgType = %d",
			ANDROID_PUSH, fromId.c_str(), toId.c_str(), strPushToken.c_str(), msgId.c_str(), msgType);
		sendReq(&aPush, ANDROID_PUSH, imsvr::APUSH);
#endif
	}
	else						//苹果推送
	{
		PSvrMsg iPush;
		iPush.set_emsgtype((MsgType)msgType);
		iPush.set_smsgid(msgId);
		iPush.set_sbody(content);
		iPush.set_sfromid(fromId);
		iPush.set_stoid(toId);
		iPush.set_stotoken(strPushToken);
	//	iPush.set_nunreadnotifymsgcount(m_offlineMsgMgr.GetUserUnreadPushMsgCount(toId));
		iPush.set_versioncode(vesionCode);
		iPush.set_edivece_type(im::DiveceType(pushType));
		sendReq(&iPush, APNS_PUSH, imsvr::IPUSH);

		DbgLog("****send request to ipush for use %s to %s, devToken = %s, msgId = %s, msgType = %d appVersion=%d",
			fromId.c_str(), toId.c_str(), strPushToken.c_str(), msgId.c_str(), msgType, vesionCode);
	}
}

void CBaseHandle::sendPush(std::shared_ptr<CLoginInfo>& pLogin, const string& fromId, const string& toId, const string& msgId, MsgType msgType, const string& content)
{
	if (pLogin && pLogin->IsLogin()) return;

	uint16_t devType = 0;
	string devToken = "";
	if (!pLogin)
	{
		return;
	}
	else
	{
	}

	if (devToken.empty())
	{
		WarnLog("!!!get user %s token failed, can't send push\r\n", toId.c_str());
		return;
	}

	if (devType != APNS && devType != APNS_DEV)		//安卓推送
	{
#if 0
		ANDPushMsg aPush;
		aPush.set_emsgtype(msgType);
		aPush.set_smsgid(msgId);
		aPush.set_stitle(getAppName());
		aPush.set_sbody(content);
		aPush.set_stoid(toId);
		aPush.set_sdivece_token(devToken);
		aPush.set_edivece_type(im::DiveceType(devType));
		DbgLog("****send request 0x%x to andpush for use %s to %s, devToken = %s, msgId = %s, msgType = %d",
			ANDROID_PUSH, fromId.c_str(), toId.c_str(), devToken.c_str(), msgId.c_str(), msgType);
		sendReq(&aPush, ANDROID_PUSH, imsvr::APUSH);
#endif
	}
	else						//苹果推送
	{
		PSvrMsg iPush;
		iPush.set_emsgtype((MsgType)msgType);
		iPush.set_smsgid(msgId);
		iPush.set_sbody(content);
		iPush.set_sfromid(fromId);
		iPush.set_stoid(toId);
		iPush.set_stotoken(devToken);
	//	iPush.set_nunreadnotifymsgcount(m_offlineMsgMgr.GetUserUnreadPushMsgCount(toId));
		iPush.set_edivece_type(im::DiveceType(devType));
		sendReq(&iPush, APNS_PUSH, imsvr::IPUSH);
	}
}


bool CBaseHandle::Initialize(void)
{
	bool retcode = RegistPacketExecutor();
	if (retcode)
		StartThread();

	return retcode;
}

