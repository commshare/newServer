/******************************************************************************
Filename: basehandle.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description: 
******************************************************************************/
#include "basehandle.h"
#include "configfilereader.h"
#include "serverinfo.h"
#include "util.h"
#include "im.pub.pb.h"
#include <google/protobuf/message_lite.h>

using namespace im;
using namespace std;

//class fakeProtoMsg : public im::SYSAssocSvrRegist
//{
//public:
//	fakeProtoMsg(const string& content) :m_content(content){}
//	int ByteSize() const{return m_content.length();}
//	bool SerializeToArray(unsigned char* data, int len){ memcpy(data, m_content.c_str(), len); return true; }
//private:
//	string m_content;
//};

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
	int retCode = SendPdu(&pdu, nDirection);
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

//int CBaseHandle::sendReq(const string& msg, uint16_t command_id, int16_t nServiceId)
//{
//	google::protobuf::MessageLite* pMsg = new fakeProtoMsg(msg);
//	sendReq(pMsg, command_id, nServiceId);
//	return 0;
//}

bool CBaseHandle::Initialize(void)
{
	bool retcode = RegistPacketExecutor();
	if(retcode)
		StartThread();
	OnInitialize();
	return retcode;
}

#ifdef DEBUG
void CBaseHandle::OnThreadRun(void)
{
	printf("CBaseHandle start thread\r\n");
	CPacket::OnThreadRun();
}
#endif



//////////////////////////////////////////////////////////////////////
const char* getCmdStr(unsigned int cmdId)
{
	switch (cmdId)
	{
	case 0xb001:
		return "MESChat";
	case 0xb002:
		return "MESChatAck";
	case 0xb003:
		return "MESChatDeliveredAck";
	case 0xb004:
		return "MESChatDelivered";
	case 0xb005:
		return "MESChatDeliveredNotificationAck";
	case 0xb006:
		return "MESChatDeliveredNotification";
	case 0xb007:
		return "MESChatRead";
	case 0xb008:
		return "MESChatReadAck";
	case 0xb009:
		return "MESChatReadDelivereAck";
	case 0xb00a:
		return "MESChatReadDelivere";
	case 0xb011:
		return "MESOfflineSummary";
	case 0xb012:
		return "MESOfflineSummaryAck";
	case 0xb013:
		return "MESOfflineTotal";
	case 0xb014:
		return "MESOfflineTotalAck";
	case 0xb015:
		return "MESOfflineMsg";
	case 0xb016:
		return "MESOfflineMsgAck";
	case 0xb017:
		return "MESOfflineMsgDelivered";
	case 0xb018:
		return "MESOfflineMsgDeliveredAck";
	case 0xb019:
		return "MESOfflineMsgDeliveredNotifyAck";
	case 0xb01a:
		return "MESOfflineMsgDeliveredNotify";
	case 0xb031:
		return "MESAddFriend";
	case 0xb032:
		return "MESAddFriendAck";
	case 0xb033:
		return "MESAddFriendDeliverAck";
	case 0xb034:
		return "MESAddFriendDeliver";
	case 0xb035:
		return "MESAddFriendAns";
	case 0xb036:
		return "MESAddFriendAnsAck";
	case 0xb037:
		return "MESAddFriendAnsDeliverACK";
	case 0xb038:
		return "MESAddFriendAnsDeliver";
	case 0xb039:
		return "MESDelFriend";
	case 0xb03a:
		return "MESDelFriendAck";
	case 0xb041:
		return "MESIncBlacklist";
	case 0xb042:
		return "MESIncBlacklistAck";
	case 0xb043:
		return "MESDecBlacklist";
	case 0xb044:
		return "MESDecBlacklistAck";
	case 0xb051:
		return "MESJoinGrp";
	case 0xb052:
		return "MESJoinGrpAck";
	case 0xb053:
		return "MESJoinGrpDeliverAck";
	case 0xb054:
		return "MESJoinGrpDeliver";
	case 0xb059:
		return "MESExchangeKey";
	case 0xb060:
		return "MESExchangeKeyAck";
	case 0xb061:
		return "MESExchangeKeyDeliverAck";
	case 0xb062:
		return "MESExchangeKeyDeliver";
	case 0xb063:
		return "MESExchangeKeyDeliverNotifyAck";
	case 0xb064:
		return "MESExchangeKeyDeliverNotify";
	case 0xb065:
		return "_MESGrpInterNotify";
	case 0xb066:
		return "_MESGrpInterNotifyAck";
	case 0xb067:
		return "MESGrpNotifyDeliverAck";
	case 0xb068:
		return "MESGrpNotify";
	case 0xb071:
		return "_MESGrpInterChat";
	case 0xb072:
		return "_MESGrpInterChatAck";
	case 0xb073:
		return "MESGrpChatDeliveredAck";
	case 0xb074:
		return "MESGrpChat";
	case 0xb075:
		return "_MESGrpInterChatCancel";
	case 0xb076:
		return "_MESGrpInterChatCancelAck";
	case 0xb077:
		return "MESChatCancel";
	case 0xb078:
		return "MESChatCancelAck";
	case 0xb081:
		return "MESChatCancelDeliver";
	case 0xb082:
		return "MESChatCancelDeliverAck";
	}
	return "unknown";
}
