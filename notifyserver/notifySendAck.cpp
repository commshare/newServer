#include"notifySendAck.h"

#include "util.h"
#include "im.mes.pb.h"
#include "im.cm.pb.h"
#include "im.pub.pb.h"
#include "im.inner.pb.h"

CNotifySendAck::CNotifySendAck()
	: m_nActualServiceInst(0)
{

}

CNotifySendAck::~CNotifySendAck()
{

}

bool CNotifySendAck::Initialize()
{
	bool retcode = RegistPacketExecutor();
	if(retcode)
		StartThread();
	return retcode;
}

bool CNotifySendAck::RegistPacketExecutor(void)
{
	CmdRegist(im::CM_PHP_LOGIN_NOTIFY_ACK, m_nActualServiceInst,  CommandProc(&CNotifySendAck::OnLoginNotifyAck));
	CmdRegist(im::SVR_GROUP_RELATIN_NOTIFY_ACK, m_nActualServiceInst,  CommandProc(&CNotifySendAck::OnGroupRelationAck));
	CmdRegist(im::SVR_FRIEND_RELATION_NOTIFY_ACK, m_nActualServiceInst,  CommandProc(&CNotifySendAck::OnFriendRelationAck));
	CmdRegist(im::SVR_COMMON_MSG_NOTIFY_ACK, m_nActualServiceInst,  CommandProc(&CNotifySendAck::OnCommonMsgNotifyAck));
	CmdRegist(im::SVR_RADIO_RELATIN_NOTIFY_ACK, m_nActualServiceInst,  CommandProc(&CNotifySendAck::OnRadioMsgNotifyAck));
	CmdRegist(im::SVR_USER_PUSHSET_NOTIFY_ACK, m_nActualServiceInst,  CommandProc(&CNotifySendAck::OnUserPushSetNotifyAck));
	return true;
}

void CNotifySendAck::OnLoginNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	im::CMPHPLoginNotifyACK loginAck;
	if (!pPdu->GetBodyData() || !loginAck.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("%s" , loginAck.suserid().c_str());
	return;
}

void CNotifySendAck::OnGroupRelationAck(std::shared_ptr<CImPdu> pPdu)
{
	im::SVRMSGNotifyACK notifyAck;
	if (!pPdu->GetBodyData() || !notifyAck.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("%s" , notifyAck.smsgid().c_str());
	return;
}

void CNotifySendAck:: OnFriendRelationAck(std::shared_ptr<CImPdu> pPdu)
{
	im::SVRMSGNotifyACK notifyAck;
	if (!pPdu->GetBodyData() || !notifyAck.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("%s" , notifyAck.smsgid().c_str());
	return;
}

void CNotifySendAck::OnCommonMsgNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	im::SVRMSGNotifyACK notifyAck;
	if (!pPdu->GetBodyData() || !notifyAck.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("%s" , notifyAck.smsgid().c_str());
	return;
}

void CNotifySendAck::OnRadioMsgNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	im::SVRMSGNotifyACK notifyAck;
	if (!pPdu->GetBodyData() || !notifyAck.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("%s" , notifyAck.smsgid().c_str());
	return;
}

void CNotifySendAck::OnUserPushSetNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	im::SVRMSGNotifyACK notifyAck;
	if (!pPdu->GetBodyData() || !notifyAck.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		return;
	}
	DbgLog("%s" , notifyAck.smsgid().c_str());
	return;
}



