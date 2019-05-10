#include "mi_protoc.h"
#include "utility.hpp"
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


string CMiProtoc::GetHttpUrl() {
    return "https://api.xmpush.xiaomi.com/v2/message/alias";
}

void  CMiProtoc::GetHttpHeaders(vector<string>& vstr) {

		vstr.push_back("authorization:key=2QuNEZ7GxNAlxSHnl1LSZA==");
}

string CMiProtoc::GetHttpPostData(bool phoneCall) {
        
	if (!m_pPbMsg)
	{
		ErrLog("m_pPbMsg is nullptr!");
		return "";
	}

	CAppOpenJson appData;
	appData.utype = (int)m_pPbMsg->emsgtype();
	appData.sid = m_pPbMsg->stoid();

	string payLoad = appData.GetJsonValue().toStyledString();
    static int notifyId = 0;
	char strSendBody[4096];
    if(!phoneCall) {
        sprintf(strSendBody, mi_push_common_post_form_data.c_str(), m_pPbMsg->sdivece_token().c_str(), notifyId, 
                m_pPbMsg->stitle().c_str(), m_pPbMsg->sbody().c_str(), payLoad.c_str());

    } else {
        sprintf(strSendBody, mi_push_post_form_data.c_str(), m_pPbMsg->sdivece_token().c_str(), notifyId, 
                m_pPbMsg->stitle().c_str(), m_pPbMsg->sbody().c_str(), payLoad.c_str());
    }
   if(notifyId > NOTIFY_ID_MAX) {
       notifyId = 0;
   }
   notifyId++;//整数溢出应该也没问题吧？ 
   
  //  if(!phoneCall) {
  //      sprintf(strSendBody, mi_push_common_post_form_data.c_str(), m_pPbMsg->sdivece_token().c_str(), 
  //              m_pPbMsg->stitle().c_str(), m_pPbMsg->sbody().c_str(), payLoad.c_str());

  //  } else {
  //      sprintf(strSendBody, mi_push_post_form_data.c_str(), m_pPbMsg->sdivece_token().c_str(), 
  //              m_pPbMsg->stitle().c_str(), m_pPbMsg->sbody().c_str(), payLoad.c_str());
  //  }
    return strSendBody;                      
}                                            
                                             
                                             
                                             
                                             
                                             
