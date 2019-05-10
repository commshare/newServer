#include "customerSvrProxy.h"
#include "im.mes.pb.h"
#include <json/json.h>
#include "redisMgr.h"
#include "common.h"
#include "custInfoMgr.h"

#if 0
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

#endif

CCustomerSvrProxy::CCustomerSvrProxy(CConfigFileReader* pConfigReader, int nNumOfInst)
:CBaseHandle(pConfigReader), m_nNumberOfInst(nNumOfInst)
{

}

CCustomerSvrProxy::~CCustomerSvrProxy()
{

}

void CCustomerSvrProxy::OnMsgChatDeliver(std::shared_ptr<CImPdu> pPdu)
{
	#if 0
	im::MESChat msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		ErrLog("parse pdu failed!");
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

	//消息存入mongodb
	CCustMsg custMsg;
	custMsg.m_command = CUST::MES_CHAT;
	custMsg.m_sFromId = msg.sfromid();
	custMsg.m_sServiceId = msg.stoid();
	custMsg.m_sMsgId = msg.smsgid();
	custMsg.m_encrypt = msg.encrypt();
	custMsg.m_sContent = msg.scontent();
	custMsg.m_msgTime = msg.msgtime();
	custMsg.m_createTime = getCurrentTime();
	custMsg.m_updateTime = getCurrentTime();
	custMsg.m_processed = 0;
	if(false == m_custMsgMgr.insertCustMsg(custMsg))
	{
		ErrLog("insert MongoDb failed [msgid:%s]",msg.smsgid().c_str());
		return;
	}
	
	//回ACK给用户
	im::MESChatDeliveredAck chatDeliverAck;
	chatDeliverAck.set_sfromid(msg.stoid());
	chatDeliverAck.set_stoid(msg.sfromid());
	chatDeliverAck.set_smsgid(msg.smsgid());
	sendAck(&chatDeliverAck, MES_CHAT_DELIVER_ACK, pPdu->GetSessionId());
	log("****%s send MESChatDeliveredAck(0x%x) %s to %s",
		chatDeliverAck.sfromid().c_str(), MES_CHAT_DELIVER_ACK, chatDeliverAck.smsgid().c_str(), chatDeliverAck.stoid().c_str());

	//user正在cust的当前处理会话中，则直接发送
	CUST::USER_PROCESSING_INFO processingInfo;
	bool ret = CRedisMgr::getInstance()->getCurrentCustInfoByUser(msg.sfromid(),processingInfo);
	if( true == ret )
	{
		//更新CustMsg中消息的toId
		m_custMsgMgr.updateToCustId(msg.sfromid(), msg.stoid(), msg.smsgid(), processingInfo.custId);

		//
		return;
	}

	//Json::Reader JsonParser = Json::Reader(Json::Features::strictMode());
	//Json::Value tempVal;
	//JsonParser.parse(msg.scontent(), tempVal);
	
	//string questionId = tempVal[CUST_JSON_FIELD_SERVICEID].asString();
	
	dispatchUserToCust(msg.sfromid(),msg.stoid(),questionId);
	
	//postClientMsg(protoMsgToJsonStr(msg));
	#endif

}

#if 0
bool  CCustomerSvrProxy::userIsProcessing(const string& userId,const string& serviceID)
{
	
	CUST::USER_PROCESSING_INFO processingInfo;
	bool ret = CRedisMgr::getInstance()->getCurrentCustInfoByUser(userId,processingInfo);
	if(true == ret)
	{	
		//更新CustMsg中消息的toId
		//发送消息给对应Cust
			
	}

	return ret;
	
}
#endif

void  CCustomerSvrProxy::dispatchUserToCust(const string& userId,const string& serviceID,const string& questionId)
{	
	#if 0
	CustList custs;
	bool ret = CCustInfoMgr::getInstance()->getCustListByQuestionId(questionId,serviceID,custs);
	
	string custId;
	int32_t queueSize=0;
	
	if(true == ret)
	{
		map<string,map<string,string>> custsInfo;
		CRedisMgr::getInstance()->getCustInfo(custs,custsInfo);

		map<string,map<string,string>>::iterator pCustInfo = custsInfo.begin();
		for(; pCustInfo != custsInfo.end(); pCustInfo++)
		{
			if(STATUS_ONLINE != string2int(pCustInfo->second[VALUE_CUST_STATUS]))
			{
				continue;
			}

			if( CRedisMgr::getInstance()->getSessionListNum()  <= string2int(pCustInfo->second[VALUE_CUST_CURRENT_SIZE] ))
			{
				if( CRedisMgr::getInstance()->getSessionListNum()  <= 
				string2int(pCustInfo->second[VALUE_CUST_QUEUE_SIZE]))
				{
					continue;
				}
				else
				{
					custId = pCustInfo->first;
					queueSize = atoi(pCustInfo->second[VALUE_CUST_QUEUE_SIZE].c_str());
				}
			}
			else
			{
				CRedisMgr::getInstance()->userToCustCurrent(userId,pCustInfo->first);
				break;
			}
		}

		if(!custId.empty())
		{
			CRedisMgr::getInstance()->userToCustQueue(userId,custId);
		}
	}
	else
	{
		//doNothing
	}

	#endif

}



bool CCustomerSvrProxy::RegistPacketExecutor(void)
{
	CmdRegist(MES_CHAT_DELIVER, m_nNumberOfInst, CommandProc(&CCustomerSvrProxy::OnMsgChatDeliver));
	//CmdRegist(MES_OFFLINEMSG_DELIVERED_ACK, m_nNumberOfInst, CommandProc(&CCustomerSvrProxy::ignoreMsg));
	return true;
}

void CCustomerSvrProxy::ignoreMsg(std::shared_ptr<CImPdu> pPdu)
{
	NOTUSED_ARG(pPdu);
}

void CCustomerSvrProxy::OnInitialize()
{
	static bool bInit = false;
	if (!bInit)
	{
#ifdef _WIN32
		(void)CreateThread(NULL, 0, StartRoutine, this, 0, &m_thread_id);
#else
		//pthread_t tid;
		//(void)pthread_create(&tid, NULL, GetOffLineMsgRoutine, this);
#endif
		bInit = true;
	}
}
