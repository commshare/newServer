/**
 * creator by liulang
 */

#include "xmpp_google_fcm_protoc.h"
#include "utility.hpp"
#include "protobuf_phase.h"

CXmppFcmPayLoad::CXmppFcmPayLoad()
{
	m_bSet = false;
	Init();
}

void CXmppFcmPayLoad::Init()
{
	m_jroot[fcmPayLoad_delivery_receipt_requested] = true;
	m_jroot[fcmPayLoad_priority] = "high";
	m_jroot[fcmPayLoad_content_available] = true;
}

bool CXmppFcmPayLoad::SetValue(const char *to, const char *msgId, const char *title, 
							   const char *body, const string &toId, int iNotify_type/* = 0*/)
{
	if(!to || !msgId || !title || !body)
	{
		ErrLog("!to || !msgId || !title || !body");
		return false;
	}

	m_jroot[fcmPayLoad_to] = to;
	m_jroot[fcmPayLoad_message_id] = msgId;

	Json::Value jnotification;
	jnotification[fcmPayLoad_title] = title;
	jnotification[fcmPayLoad_body] = body;

	m_jroot[fcmPayLoad_notification] = jnotification;


	CAppOpenJson appData;
	appData.utype = iNotify_type;
	appData.sid = toId;

	m_jroot[fcmPayLoad_data] = appData.GetJsonValue();

	m_bSet = true;

	return true;
}

const string CXmppFcmPayLoad::GetMsgIncJson()
{
	if (m_bSet)
	{
		return m_jroot.toStyledString();
	}
	else
	{
		ErrLog("m_jroot not set GetMsgIncJson");
		return "";
	}
}

CXmppFcmProtoc::CXmppFcmProtoc(im::ANDPushMsg *pPbMsg)
{
	m_pPbMsg = pPbMsg;
}

std::string CXmppFcmProtoc::GetSendBuf()
{
	if(!m_pPbMsg)
	{
		ErrLog("m_pPbMsg is null");
		return "";
	}

	CXmppFcmPayLoad cxmppFcmPayLoad;

	if(!cxmppFcmPayLoad.SetValue(m_pPbMsg->sdivece_token().c_str(), m_pPbMsg->smsgid().c_str(), 
								 m_pPbMsg->stitle().c_str(), m_pPbMsg->sbody().c_str(), m_pPbMsg->stoid(), m_pPbMsg->emsgtype()))
	{
		ErrLog("cxmppFcmPayLoad.SetValue");
		return "";
	}

	char sendBuf[4096];
	string tmp = cxmppFcmPayLoad.GetMsgIncJson();

	if(tmp.empty())
	{
		ErrLog("cxmppFcmPayLoad.GetMsgIncJson");
		return "";
	}

	sprintf(sendBuf, xmpp_handshake_MsgTemplate.c_str(), tmp.c_str());
	return sendBuf;
}


