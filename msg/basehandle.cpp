/******************************************************************************
Filename: basehandle.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description: 
******************************************************************************/
#include "basehandle.h"
#include "configfilereader.h"
#include "im_loginInfo.h"
#include "im.push.android.pb.h"
#include "im.pushSvrAPNsMsg.pb.h"
#include "im.mes.pb.h"
//#include "mysqlUserMgr.h"
#include "redisLoginInfoMgr.h"
#include "serverinfo.h"
#include "util.h"

using namespace im;
using namespace std;

//你有一条新消息！
const string NewMsgStr("\344\275\240\346\234\211\344\270\200\346\235\241\346\226\260\346\266\210\346\201\257");
//你有一条新请求！
const string NewRequestStr("\344\275\240\346\234\211\344\270\200\346\235\241\346\226\260\350\257\267\346\261\202");

CBaseHandle::CBaseHandle(CConfigFileReader* pConfigReader)
	: m_pConfigReader(pConfigReader)
	, m_offlineMsgMgr(OFFLINEMSG_MONGO_DB_NAME, OFFLINEMSG_MONGO_COLL_NAME)
	, m_grpInfoUrl("")
	, m_usrInfoUrl("")
	, m_grpMemberInfoUrl("")
	, m_friendInfoUrl("")
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
	//static int msgChatAckCount = 0;
	//if (MES_CHAT_ACK == command_id)
	//{
	//	++msgChatAckCount;
	//	DbgLog("sendMsgChatAck %d times", msgChatAckCount);
	//}

	if (retCode <= 0)
	{
		WarnLog("!!!send command 0x%x failed, return %d, session=%x%x%x%x%x%x%x%x%x%x%x%x", command_id, retCode,
			sessionId.Uid_Item.code[0], sessionId.Uid_Item.code[1],
			sessionId.Uid_Item.code[2], sessionId.Uid_Item.code[3], sessionId.Uid_Item.code[4],
			sessionId.Uid_Item.code[5], sessionId.Uid_Item.code[6], sessionId.Uid_Item.code[7],
			sessionId.Uid_Item.code[8], sessionId.Uid_Item.code[9], sessionId.Uid_Item.code[10],
			sessionId.Uid_Item.code[11]);
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
		iPush.set_nunreadnotifymsgcount(m_offlineMsgMgr.GetUserUnreadPushMsgCount(toId));
		iPush.set_versioncode(vesionCode);
		iPush.set_edivece_type(im::DiveceType(pushType));
		sendReq(&iPush, APNS_PUSH, imsvr::IPUSH);

		DbgLog("****send request to ipush for use %s to %s, devToken = %s, msgId = %s, msgType = %d appVersion=%d",
			fromId.c_str(), toId.c_str(), strPushToken.c_str(), msgId.c_str(), msgType, vesionCode);
	}
}

void CBaseHandle::sendAndroidPush(const string& fromId, const string& toId, const string& msgId, im::MsgType msgType, 
																const string& content, const string& pushToken, int pushType)
{
	if(pushToken.empty())
	{
		WarnLog("!!!get user %s token failed, can't send push\r\n", toId.c_str());
		return;
	}
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

void CBaseHandle::sendiOSPush(const string& fromId, const string& toId, const string& msgId, im::MsgType msgType, const string& content, 
															const string& pushToken, int pushType, int callType, const std::string& callId)
{
	if(pushToken.empty())
	{
		WarnLog("!!!get user %s token failed, can't send push\r\n", toId.c_str());
		return;
	}
	PSvrMsg iPush;
	iPush.set_emsgtype((MsgType)msgType);
	iPush.set_smsgid(msgId);
	iPush.set_sbody(content);
	iPush.set_sfromid(fromId);
	iPush.set_stoid(toId);
	iPush.set_stotoken(pushToken);
	iPush.set_nunreadnotifymsgcount(0);
	iPush.set_edivece_type(im::DiveceType(pushType));
	iPush.set_calltype(callType);
	iPush.set_scallid(callId);
	sendReq(&iPush, APNS_PUSH, imsvr::IPUSH);

	DbgLog("****send request to ipush for use %s to %s, devToken = %s, msgId = %s, msgType = %d",
		fromId.c_str(), toId.c_str(), pushToken.c_str(), msgId.c_str(), msgType);
}


string CBaseHandle::getAppName() const
{
	static string appName("");
	if (appName.empty())
	{
		if (NULL == m_pConfigReader) return "unknown";
		char* str_userId = m_pConfigReader->GetConfigName("appName");
		if (NULL == str_userId)
		{
			return "notDefined";
		}
		appName = string(str_userId);
	}
	return  appName;
}

bool CBaseHandle::Initialize(void)
{
	bool retcode = RegistPacketExecutor();
	if(retcode)
		StartThread();
	getConfigData();
	return retcode;
}
const char* getofflineTypeStr(unsigned int cmdId)
{
	switch (cmdId)
	{
	case 0xb004:
		return "MESChatDelivered";
	case 0xb006:
		return "MESChatDeliveredNotification";
	case 0xb00a:
		return "MESChatReadDelivere";
	case 0xb034:
		return "MESAddFriendDeliver";
	case 0xb035:
		return "MESAddFriendAns";
	case 0xb038:
		return "MESAddFriendAnsDeliver";
	case 0xb054:
		return "MESJoinGrpDeliver";
	case 0xb062:
		return "MESExchangeKeyDeliver";
	case 0xb064:
		return "MESExchangeKeyDeliverNotify";
	case 0xb068:
		return "MESGrpNotify";
	case 0xb074:
		return "MESGrpChat";
	case 0xb081:
		return "MESChatCancelDeliver";
	}
	return "no specified";
}

void CBaseHandle::getConfigData()
{
	if(nullptr == m_pConfigReader)
		return;
	char* pData = m_pConfigReader->GetConfigName("grpInfoUrl");
	if(pData)
		m_grpInfoUrl = pData;
	pData = m_pConfigReader->GetConfigName("usrInfoUrl");
	if(pData)
		m_usrInfoUrl = pData;
	pData = m_pConfigReader->GetConfigName("grpMemberInfoUrl");
	if(pData)
		m_grpMemberInfoUrl = pData;
	pData = m_pConfigReader->GetConfigName("friendInfoUrl");
	if(pData)
		m_friendInfoUrl = pData;
	pData = m_pConfigReader->GetConfigName("check_appsecret");
	if(pData)
		m_sAppSecret = pData;
}

bool CBaseHandle::checkDeviceLastUser(const std::string& strUserId, const std::string& strDeviceToken)
{
	std::string lastUserId = CLoginInfoMgr::getDeviceLastUserID(strDeviceToken);
	DbgLog("check device last user. userid=%s lastUserId=%s", strUserId.c_str(), lastUserId.c_str());
	return strUserId.compare(lastUserId) == 0;
}

