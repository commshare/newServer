#include "mi_protoc.h"



CMiProtoc::CMiProtoc(im::ANDPushMsg *pPbMsg)
{
	m_pPbMsg = pPbMsg;
}

string CMiProtoc::GetSendBuf()
{
	if (!m_pPbMsg)
	{
		ErrLog("m_pPbMsg is nullptr!");
		return "";
	}

	CAppOpenJson appData;
	appData.utype = (int)m_pPbMsg->emsgtype();
	appData.sid = m_pPbMsg->stoid();

	string payLoad = appData.GetJsonValue().toStyledString();

	char strSendBody[4096];
	sprintf(strSendBody, mi_push_post_form_data.c_str(), m_pPbMsg->sdivece_token().c_str(), 
			m_pPbMsg->stitle().c_str(), m_pPbMsg->sbody().c_str(), payLoad.c_str());

	char strSendHead[4096];
	sprintf(strSendHead, mi_push_post_head.c_str(), strlen(strSendBody), strSendBody);

	return strSendHead;
}




