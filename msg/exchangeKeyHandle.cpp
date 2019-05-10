#include "exchangeKeyHandle.h"

#include "configfilereader.h"
#include "im_loginInfo.h"
#include "im.mes.pb.h"
#include "im.pub.pb.h"
#include "serverinfo.h"
#include "util.h"
#include "redisLoginInfoMgr.h"

using namespace im;
using namespace std;

CExchangeKeyHandle::CExchangeKeyHandle(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader), m_nNumberOfInst(nNumOfInst)
{

}

CExchangeKeyHandle::~CExchangeKeyHandle()
{
	
}

bool CExchangeKeyHandle::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(MES_EXCHANGE_KEY,				m_nNumberOfInst,  CommandProc(&CExchangeKeyHandle::OnExchangeKey));
	CmdRegist(MES_EXCHANGE_KEY_DELIVER_ACK, m_nNumberOfInst,  CommandProc(&CExchangeKeyHandle::OnExchangeKeyDeliverAck));
	CmdRegist(MES_EXCHANGE_KEY_DELIVERD_NOTIFY_ACK, m_nNumberOfInst,  CommandProc(&CExchangeKeyHandle::OnExchangeKeyDeliverNotifyAck));
	return true;
}

void ExchangeKeyInsertedCallBack(const std::vector<std::shared_ptr<IMongoDataEntry> >& msgs, bool result, void* paras)
{
	OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;

	CExchangeKeyHandle* pHandle = (CExchangeKeyHandle*)pCallBackPara->m_handle;
	if (NULL == pHandle)
	{
		delete pCallBackPara;
		return;
	}

	std::vector<std::shared_ptr<COfflineMsg> > offMsgs;
	for (unsigned int i = 0; i < msgs.size(); ++i)
	{
		offMsgs.push_back(dynamic_pointer_cast<COfflineMsg>(msgs[i]));
	}
	pHandle->OnExchangeKeyInserted(offMsgs, result, pCallBackPara->m_sessionID);

	delete pCallBackPara;		//回调函数负责释放资源
}

#define CHANGE_GRP_KEY 0x00
void CExchangeKeyHandle::OnExchangeKey(std::shared_ptr<CImPdu> pPdu)
{
	MESExchangeKey msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("exchangeKey(0x%x) %s received %s-->grp:%s", pPdu->GetCommandId()/*MES_EXCHANGE_KEY*/, msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sgrpid().c_str());
	msg.set_msgtime(getCurrentTime());//set server time

	int userNum = msg.lsuserkeys_size();
	if (0 == userNum)
	{
		MESExchangeKeyAck exchangeKeyAck;
		exchangeKeyAck.set_smsgid(msg.smsgid());
		exchangeKeyAck.set_suserid(msg.sfromid());
		exchangeKeyAck.set_sgrpid(msg.sgrpid());
		exchangeKeyAck.set_errcode(ERR_GROUP_PARAMETER);
		sendAck(&exchangeKeyAck, MES_EXCHANGE_KEY_ACK, pPdu->GetSessionId());
		WarnLog("!!xkey userKey size = 0");
		return;
	}

	MESExchangeKeyDeliver exchangeKeyDeliver;
	exchangeKeyDeliver.set_sfromid(msg.sfromid());
	exchangeKeyDeliver.set_sgrpid(msg.sgrpid());
	exchangeKeyDeliver.set_smsgid(msg.smsgid());
	exchangeKeyDeliver.set_exchangemode(msg.exchangemode());
	exchangeKeyDeliver.set_encrypt(msg.encrypt());
	exchangeKeyDeliver.set_msgtime(msg.msgtime());
	if (!msg.sopruserid().empty())
	{
		exchangeKeyDeliver.set_sopruserid(msg.sopruserid());
	}

	std::vector<COfflineMsg> msgs;
	for (int index = 0; index < userNum; ++index)
	{
		const UserKey& userKey = msg.lsuserkeys(index);
		exchangeKeyDeliver.set_stoid(userKey.suserid());
		exchangeKeyDeliver.set_skey(userKey.skey());
		msgs.push_back(COfflineMsg(exchangeKeyDeliver));
	}
	m_offlineMsgMgr.InsertOfflineMsg(msgs, ExchangeKeyInsertedCallBack, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));
}

void CExchangeKeyHandle::OnExchangeKeyInserted(const std::vector<std::shared_ptr<COfflineMsg> >& msgs, bool bInsertSuccess, const UidCode_t& sessionID)
{
	//send ACK first
	const std::shared_ptr<COfflineMsg>& firstUser = msgs.front();
	MESExchangeKeyAck exchangeKeyAck;
	exchangeKeyAck.set_smsgid(firstUser->GetMsgId());
	exchangeKeyAck.set_suserid(firstUser->GetFromGrpUserId());
	exchangeKeyAck.set_sgrpid(firstUser->GetFromId());
	exchangeKeyAck.set_errcode(bInsertSuccess ? NON_ERR : EXCEPT_ERR);
	sendAck(&exchangeKeyAck, MES_EXCHANGE_KEY_ACK, sessionID);

	DbgLog("****send MESExchangeKeyAck(0x%x) %s to %s , errCode = 0x%x", MES_EXCHANGE_KEY_ACK, exchangeKeyAck.smsgid().c_str(), exchangeKeyAck.suserid().c_str(), exchangeKeyAck.errcode());
	if (!bInsertSuccess)
	{
		return;
	}

	//send exchangeKeyDeliver to grp members which is online
	std::vector<std::shared_ptr<COfflineMsg> >::const_iterator  iter= msgs.begin();
	std::vector<std::shared_ptr<COfflineMsg> >::const_iterator  endIter = msgs.end();
	for (; iter != endIter; ++iter)
	{
		MESExchangeKeyDeliver exchangeKeyDeliver;
		if (!exchangeKeyDeliver.ParseFromString((*iter)->GetMsgData()))
		{
			WarnLog("get exchangeKeyDeliver failed,msgId = %s, from %s:%s-->%s",
				(*iter)->GetMsgId().c_str(), (*iter)->GetFromId().c_str(), (*iter)->GetFromGrpUserId().c_str(), (*iter)->GetToId().c_str());
			continue;
		}
		std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo((*iter)->GetToId());
		if (pLogin && pLogin->IsLogin())
		{
			sendReq(&exchangeKeyDeliver, MES_EXCHANGE_KEY_DELIVER, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
			DbgLog("****send MES_EXCHANGE_KEY_DELIVER(0x%x) %s from %s:%s to %s", MES_EXCHANGE_KEY_DELIVER,
				exchangeKeyDeliver.smsgid().c_str(), exchangeKeyDeliver.sgrpid().c_str(), exchangeKeyDeliver.sfromid().c_str(), exchangeKeyDeliver.stoid().c_str());
		}
	}
	
}

void ExchangeKeyDeliverNotifyInsertedCallBack(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, bool bInsertSuccess, void* paras)
{
	CExchangeKeyHandle* pHandle = (CExchangeKeyHandle*)paras;
	if (pHandle)
	{
		const std::shared_ptr<COfflineMsg> pMsg = dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
		pHandle->OnExchangeKeyDeliverNotifyInserted(*pMsg, bInsertSuccess);
	}
}

void CExchangeKeyHandle::OnExchangeKeyDeliverAck(std::shared_ptr<CImPdu> pPdu)
{
	assert(NULL != pPdu);
	MESExchangeKeyDeliverAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	msg.set_msgtime(getCurrentTime());//set server time
	DbgLog("MESExchangeKeyDeliverACK(0x%x) %s received,dir %s:%s --> %s", pPdu->GetCommandId()/*MES_EXCHANGE_KEY_DELIVER_ACK*/, msg.smsgid().c_str(), msg.sgrpid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());

	//if receiver was online, send addFriendDeliver to receiver
	std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(msg.stoid());
	if (pLogin && pLogin->IsLogin())
	{
		sendReq(&msg, MES_EXCHANGE_KEY_DELIVERD_NOTIFY, pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
		DbgLog("****send MES_EXCHANGE_KEY_DELIVERD_NOTIFY(0x%x) %s to %s:%s", MES_EXCHANGE_KEY_DELIVERD_NOTIFY, msg.smsgid().c_str(), msg.sgrpid().c_str(), msg.stoid().c_str());
	}

	//!!!!!!!!!!!!!!!!!这里后续通过添加exchangeMode字段判断
	if (msg.sgrpid().empty() && msg.skey().empty())
		m_offlineMsgMgr.DelOfflineMsg(msg.sfromid(), MES_EXCHANGE_KEY_DELIVER, msg.smsgid(),NULL,NULL);
	else
		m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(msg), ExchangeKeyDeliverNotifyInsertedCallBack, this);
}

void CExchangeKeyHandle::OnExchangeKeyDeliverNotifyInserted(const COfflineMsg& offlineMsg, bool bInsertSuccess)
{
	//if ExchangeKeyDeliverNotify Inserted success, remove ExchangeKeyDeliver
	if (bInsertSuccess)
	{
		m_offlineMsgMgr.DelOfflineMsg(offlineMsg.GetFromGrpUserId(), MES_EXCHANGE_KEY_DELIVER, offlineMsg.GetMsgId());
	}
}

void CExchangeKeyHandle::OnExchangeKeyDeliverNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	MESExchangeKeyDeliverNotifyAck msg;
	if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	log("MESExchangeKeyDeliverNotifyAck(0x%x) %s received", pPdu->GetCommandId()/*MES_EXCHANGE_KEY_DELIVERD_NOTIFY_ACK*/, msg.smsgid().c_str());

	m_offlineMsgMgr.DelOfflineMsg(msg.suserid(), MES_EXCHANGE_KEY_DELIVERD_NOTIFY, msg.smsgid());
}



