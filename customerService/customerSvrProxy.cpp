#include "customerSvrProxy.h"
#include "im.mes.pb.h"
#include <json/json.h>
#include "redisMsgMgr.h"

string protoMsgToJsonStr(const im::MESChat& msg)
{
	Json::Value 		root;
	root[MSG_FIELD_CMD] = MES_CHAT_DELIVER;
	root[MSG_FIELD_SFROMID] = msg.sfromid();
	root[MSG_FIELD_STOID] = msg.stoid();
	root[MSG_FIELD_SMSGID] = msg.smsgid();
	root[MSG_FIELD_MSGTYPE] = msg.msgtype();
	if (msg.msgtime())
		root[MSG_FIELD_MSGTIME] = Json::Value::UInt64(msg.msgtime());
	if (msg.encrypt())
		root[MSG_FIELD_ENCRYPT] = (uint32_t)msg.encrypt();
	root[MSG_FIELD_SCONTENT] = msg.scontent();
	root[MSG_FIELD_SSERVICEID] = msg.sserviceid();
	root[MSG_FIELD_SQUESTIONID] = msg.squestionid();
	
	return root.toStyledString();
}

string protoMsgToJsonStr(const im::MESChatCancel& msg)
{
	Json::Value 		root;
	root[MSG_FIELD_CMD] = MES_CHATCANCEL_DELIVER;
	root[MSG_FIELD_SFROMID] = msg.sfromid();
	root[MSG_FIELD_STOID] = msg.stoid();
	root[MSG_FIELD_SMSGID] = msg.smsgid();
	root[MSG_FIELD_NCANCELTYPE] = msg.ncanceltype();
	if (msg.ncanceltype())
	{
		root[MSG_FIELD_SGROUPID] = msg.sgroupid();
	}
	if (msg.msgtime())
		root[MSG_FIELD_MSGTIME] = Json::Value::UInt64(msg.msgtime());
	if (msg.canceltime())
		root[MSG_FIELD_CANCELTIME] = Json::Value::UInt64(msg.canceltime());
	return root.toStyledString();
}

CCustomerSvrProxy::CCustomerSvrProxy(CConfigFileReader* pConfigReader, int nNumOfInst)
:CBaseHandle(pConfigReader), m_nNumberOfInst(nNumOfInst)
{

}

CCustomerSvrProxy::~CCustomerSvrProxy()
{

}

void CCustomerSvrProxy::OnMsgChatDeliver(std::shared_ptr<CImPdu> pPdu)
{
	MESChat msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

	if (msg.sfromid().empty())
	{
		ErrLog("no fromId specified");
		return;
	}

	if (msg.stoid().empty() | msg.smsgid().empty())
	{
		ErrLog("no toId or msgId specified");
		return;
	}

	im::MESChatDeliveredAck chatDeliverAck;
	chatDeliverAck.set_sfromid(msg.stoid());
	chatDeliverAck.set_stoid(msg.sfromid());
	chatDeliverAck.set_smsgid(msg.smsgid());
	sendAck(&chatDeliverAck, MES_CHAT_DELIVER_ACK, pPdu->GetSessionId());
	log("****%s send MESChatDeliveredAck(0x%x) %s to %s",
		chatDeliverAck.sfromid().c_str(), MES_CHAT_DELIVER_ACK, chatDeliverAck.smsgid().c_str(), chatDeliverAck.stoid().c_str());
	postClientMsg(protoMsgToJsonStr(msg));

}

void CCustomerSvrProxy::OnMsgChatCancelDeliver(std::shared_ptr<CImPdu> pPdu)
{
	MESChatCancel msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

	if (msg.sfromid().empty())
	{
		ErrLog("no fromId specified");
		return;
	}

	if (msg.stoid().empty() | msg.smsgid().empty())
	{
		ErrLog("no toId or msgId specified");
		return;
	}
	
	im::MESChatCancelAck chatCancelDeliverAck;
	chatCancelDeliverAck.set_suserid(msg.stoid());
	chatCancelDeliverAck.set_smsgid(msg.smsgid());
	chatCancelDeliverAck.set_errcode(im::NON_ERR);
	sendAck(&chatCancelDeliverAck, MES_CHATCANCEL_DELIVER_ACK, pPdu->GetSessionId());
	log("****%s send MESChatCancelDeliveredAck(0x%x) %s", MES_CHATCANCEL_DELIVER_ACK,
		chatCancelDeliverAck.suserid().c_str(), chatCancelDeliverAck.smsgid().c_str());
	postClientMsg(protoMsgToJsonStr(msg));
}

void CCustomerSvrProxy::OnMsgChatReadDeliver(std::shared_ptr<CImPdu> pPdu)
{
	im::MESChatRead msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

	if (msg.sfromid().empty() | msg.stoid().empty() | msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
	MESChatReadDelivereAck chatReadDeliverAck;
	chatReadDeliverAck.set_smsgid(msg.smsgid());
	chatReadDeliverAck.set_suserid(msg.stoid());

	sendAck(&chatReadDeliverAck, MES_CHAT_READ_DELIVER_ACK, pPdu->GetSessionId());
	log("****%s send MESChatReadDelivereAck(0x%x) %s",
		MES_CHAT_READ_DELIVER_ACK, chatReadDeliverAck.suserid().c_str(), chatReadDeliverAck.smsgid().c_str());
}

void CCustomerSvrProxy::OnMsgChatDeliverNotify(std::shared_ptr<CImPdu> pPdu)
{
	im::MESChatDeliveredAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}

	if (msg.sfromid().empty() | msg.stoid().empty() | msg.smsgid().empty())
	{
		ErrLog("!!!lack of required parameter");
		return;
	}
	im::MESChatDeliveredNotificationAck chatDeliverNotifyAck;
	chatDeliverNotifyAck.set_smsgid(msg.smsgid());
	chatDeliverNotifyAck.set_suserid(msg.stoid());

	sendAck(&chatDeliverNotifyAck, MES_CHAT_DELIVERED_NOTIFICATION_ACK, pPdu->GetSessionId());
	/*log("****%s send MESChatDeliveredNotificationAck(0x%x) %s",
		MES_CHAT_DELIVERED_NOTIFICATION_ACK, chatDeliverNotifyAck.suserid().c_str(), chatDeliverNotifyAck.smsgid().c_str());*/
}


void CCustomerSvrProxy::OnGetOfflineMsgAck(std::shared_ptr<CImPdu> pPdu)
{
	im::MESOfflineMsgAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		printf("para getofflineMsgAck failed");
		return;
	}
	printf("list size = %d\r\n", msg.msglist_size());
	if (msg.msglist_size())
	{
		//解析收到的数据,发送给真实的客服
		//发送消息已经送的的响应
		MESOfflineMsgDelivered offlineMsgDelivered;
		offlineMsgDelivered.set_sfromid(msg.stoid());
		offlineMsgDelivered.set_smsgid(msg.smsgid());
		
		for (int index = 0; index < msg.msglist_size(); ++index)
		{
			const OfflineMsgData& msgData = msg.msglist(index);
			switch (msgData.cmdid())
			{
			case im::MES_CHAT_DELIVER:
			{
				MESChat deliverMsg;
				if (!deliverMsg.ParseFromString(msgData.smsgdata()))
				{
					break;
				}

				if (deliverMsg.sfromid().empty())
				{
					ErrLog("0x%x no fromId specified", msgData.cmdid());
					break;
				}

				if (deliverMsg.stoid().empty() | deliverMsg.smsgid().empty())
				{
					ErrLog("0x%x no toId or msgId specified", msgData.cmdid());
					break;
				}
				postClientMsg(protoMsgToJsonStr(deliverMsg));

				OfflineDeliveredMsg* pOfflineDeliveredMsg = offlineMsgDelivered.add_lsmsgs();
				pOfflineDeliveredMsg->set_cmdid(msgData.cmdid());
				pOfflineDeliveredMsg->set_sfromid(deliverMsg.sfromid());
				pOfflineDeliveredMsg->set_stoid(deliverMsg.stoid());
				pOfflineDeliveredMsg->set_smsgid(deliverMsg.smsgid());
			}
				break;
			case im::MES_CHAT_DELIVERED_NOTIFICATION:
			{
				MESChatDeliveredAck deliverMsg;
				if (!deliverMsg.ParseFromString(msgData.smsgdata()))
				{
					break;
				}

				if (deliverMsg.sfromid().empty())
				{
					ErrLog("0x%x no fromId specified", msgData.cmdid());
					break;
				}

				if (deliverMsg.stoid().empty() | deliverMsg.smsgid().empty())
				{
					ErrLog("0x%x no toId or msgId specified", msgData.cmdid());
					break;
				}

				OfflineDeliveredMsg* pOfflineDeliveredMsg = offlineMsgDelivered.add_lsmsgs();
				pOfflineDeliveredMsg->set_cmdid(msgData.cmdid());
				pOfflineDeliveredMsg->set_sfromid(deliverMsg.sfromid());
				pOfflineDeliveredMsg->set_stoid(deliverMsg.stoid());
				pOfflineDeliveredMsg->set_smsgid(deliverMsg.smsgid());
			}
				break;
			case im::MES_CHAT_READ_DELIVER:
			{
				MESChatRead deliverMsg;
				if (!deliverMsg.ParseFromString(msgData.smsgdata()))
				{
					break;
				}

				if (deliverMsg.sfromid().empty())
				{
					ErrLog("0x%x no fromId specified", msgData.cmdid());
					break;
				}

				if (deliverMsg.stoid().empty() | deliverMsg.smsgid().empty())
				{
					ErrLog("0x%x no toId or msgId specified", msgData.cmdid());
					break;
				}

				OfflineDeliveredMsg* pOfflineDeliveredMsg = offlineMsgDelivered.add_lsmsgs();
				pOfflineDeliveredMsg->set_cmdid(msgData.cmdid());
				pOfflineDeliveredMsg->set_sfromid(deliverMsg.sfromid());
				pOfflineDeliveredMsg->set_stoid(deliverMsg.stoid());
				pOfflineDeliveredMsg->set_smsgid(deliverMsg.smsgid());
			}
				break;
			case im::MES_CHATCANCEL_DELIVER:
			{
				MESChatCancel deliverMsg;
				if (!deliverMsg.ParseFromString(msgData.smsgdata()))
				{
					break;
				}

				if (deliverMsg.sfromid().empty())
				{
					ErrLog("0x%x no fromId specified", msgData.cmdid());
					break;
				}

				if (deliverMsg.stoid().empty() | deliverMsg.smsgid().empty())
				{
					ErrLog("0x%x no toId or msgId specified", msgData.cmdid());
					break;
				}
				postClientMsg(protoMsgToJsonStr(deliverMsg));

				OfflineDeliveredMsg* pOfflineDeliveredMsg = offlineMsgDelivered.add_lsmsgs();
				pOfflineDeliveredMsg->set_cmdid(msgData.cmdid());
				pOfflineDeliveredMsg->set_sfromid(deliverMsg.sfromid());
				pOfflineDeliveredMsg->set_stoid(deliverMsg.stoid());
				pOfflineDeliveredMsg->set_smsgid(deliverMsg.smsgid());
			}
				break;
			default:
				WarnLog("%x offlinemsg not handle", msgData.cmdid());
				break;
			}
		}
		sendReq(&offlineMsgDelivered, MES_OFFLINEMSG_DELIVERED, imsvr::MSG);
		usleep(1 * 1000 * 1000);		//延迟几秒启动，使服务有时间注册zookeeper并连接到关联服务器,
		//再次发送请求

		if (msg.msglist_size())
			sendGetOfflineMsgRequest();
	}
}



bool CCustomerSvrProxy::RegistPacketExecutor(void)
{
	CmdRegist(MES_CHAT_DELIVER, m_nNumberOfInst, CommandProc(&CCustomerSvrProxy::OnMsgChatDeliver));
	CmdRegist(MES_CHATCANCEL_DELIVER, m_nNumberOfInst, CommandProc(&CCustomerSvrProxy::OnMsgChatCancelDeliver));
	CmdRegist(MES_CHAT_READ_DELIVER, m_nNumberOfInst, CommandProc(&CCustomerSvrProxy::OnMsgChatReadDeliver));
	CmdRegist(MES_CHAT_DELIVERED_NOTIFICATION, m_nNumberOfInst, CommandProc(&CCustomerSvrProxy::OnMsgChatDeliverNotify));
	CmdRegist(MES_OFFLINEMSG_ACK, m_nNumberOfInst, CommandProc(&CCustomerSvrProxy::OnGetOfflineMsgAck));
	

	CmdRegist(MES_CHAT_ACK, m_nNumberOfInst, CommandProc(&CCustomerSvrProxy::ignoreMsg));
	CmdRegist(MES_CHATCANCEL_ACK, m_nNumberOfInst, CommandProc(&CCustomerSvrProxy::ignoreMsg));
	CmdRegist(MES_OFFLINEMSG_DELIVERED_ACK, m_nNumberOfInst, CommandProc(&CCustomerSvrProxy::ignoreMsg));
	return true;
}

void CCustomerSvrProxy::ignoreMsg(std::shared_ptr<CImPdu> pPdu)
{
	NOTUSED_ARG(pPdu);
}


int CCustomerSvrProxy::postCustMsg(const std::string& jsonContent)
{
	Json::Reader *pJsonParser = new Json::Reader(Json::Features::strictMode());
	Json::Value tempVal;
	if (!pJsonParser->parse(jsonContent, tempVal))
	{
		ErrLog("parase jsonMsg %s failed", jsonContent.c_str());
		return -1;
	}
	int cmdId = tempVal[MSG_FIELD_CMD].asInt();
	switch (cmdId)
	{
#ifdef DEBUG
	case im::MES_CHAT_DELIVER:
	{
		cmdId = im::MES_CHAT;
	}
#endif
	
	case im::MES_CHAT:
	{
		im::MESChat msg;
//#ifdef DEBUG
//		msg.set_sfromid(tempVal[MSG_FIELD_STOID].asString());
//		msg.set_stoid(tempVal[MSG_FIELD_SFROMID].asString());
//#else
		msg.set_sfromid(tempVal[MSG_FIELD_SFROMID].asString());
		msg.set_stoid(tempVal[MSG_FIELD_STOID].asString());
//#endif
		msg.set_smsgid(tempVal[MSG_FIELD_SMSGID].asString());
		msg.set_msgtype(tempVal[MSG_FIELD_MSGTYPE].asUInt());
		msg.set_msgtime(tempVal[MSG_FIELD_MSGTIME].asUInt64());
		msg.set_encrypt(tempVal[MSG_FIELD_ENCRYPT].asUInt());
		msg.set_scontent(tempVal[MSG_FIELD_SCONTENT].asString());
		sendReq(&msg, cmdId, imsvr::MSG);
	}
		break;
#ifdef DEBUG
	case im::MES_CHATCANCEL_DELIVER:
	{
		cmdId = im::MES_CHATCANCEL;
	}
#endif
	case im::MES_CHATCANCEL:
	{
		im::MESChatCancel msg;
		msg.set_sfromid(tempVal[MSG_FIELD_SFROMID].asString());
		msg.set_stoid(tempVal[MSG_FIELD_STOID].asString());
		msg.set_ncanceltype(tempVal[MSG_FIELD_NCANCELTYPE].asUInt());
		msg.set_sgroupid(tempVal[MSG_FIELD_SGROUPID].asString());
		msg.set_smsgid(tempVal[MSG_FIELD_SMSGID].asString());
		msg.set_msgtime(tempVal[MSG_FIELD_MSGTIME].asUInt64());
		msg.set_canceltime(tempVal[MSG_FIELD_CANCELTIME].asUInt());
		sendReq(&msg, cmdId, imsvr::MSG);
	}
	break;
	default:
		ErrLog("unknown cmdId %d", cmdId);
		break;
	}
	log("send request to msg");
	return 0;
}

void CCustomerSvrProxy::OnInitialize()
{
	static bool bInit = false;
	if (!bInit)
	{
#ifdef _WIN32
		(void)CreateThread(NULL, 0, StartRoutine, this, 0, &m_thread_id);
#else
		pthread_t tid;
		(void)pthread_create(&tid, NULL, GetOffLineMsgRoutine, this);
#endif
		bInit = true;
	}
}


int CCustomerSvrProxy::sendGetOfflineMsgRequest()
{
	MESOfflineMsg offlineMsgRequestMsg;
	offlineMsgRequestMsg.set_stoid("10000");
	offlineMsgRequestMsg.set_count(1000);

	string msgId = getuuid();
	transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);
	offlineMsgRequestMsg.set_smsgid(msgId);
	return sendReq(&offlineMsgRequestMsg, im::MES_OFFLINEMSG, imsvr::MSG);	

}

//负责发送第一个获取离线数据请求，后续的通过响应来触发
void* CCustomerSvrProxy::GetOffLineMsgRoutine(void* arg)
{
	usleep(5 * 1000 * 1000);		//延迟几秒启动，使服务有时间注册zookeeper并连接到关联服务器,
	CCustomerSvrProxy* pCustomerSvrProxy = (CCustomerSvrProxy*)arg;
	int retCode = pCustomerSvrProxy->sendGetOfflineMsgRequest();
	while (retCode <= 0)
	{
		usleep(5 * 1000 * 1000);
		retCode = pCustomerSvrProxy->sendGetOfflineMsgRequest();
	}
	return NULL;
}

int CCustomerSvrProxy::postClientMsg(const string& msg)
{
	CIMAppFrame::GetInstance()->HandleClientMsg(msg);
	return 0;
}
