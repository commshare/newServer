/// 
///file jpush_protoc.cpp 
///add by liulang 2017-09-27 
/// 

#include "jsoncpp/json/json.h"
#include "jpush_protoc.h"
#include "protobuf_phase.h"
#include "jpush_client.h"
#include "utility.hpp"
CJsonJPushNotification::CJsonJPushNotification(string body)
{
	m_sBody = body;
}

Json::Value CJsonJPushNotification::GetJsonNotify(im::ANDPushMsg *pPbMsg)
{
	Json::Value jRoot;

	CAppOpenJson appData;
	appData.utype = (int)pPbMsg->emsgtype();
	appData.sid = pPbMsg->stoid();

	jRoot["alert"] = m_sBody;
	//jRoot["title"] = pPbMsg->stitle();

	Json::Value jextras;
	jextras["extras"] = appData.GetJsonValue();

	jRoot["android"] = jextras;

	return jRoot;
} 


CJsonAudience::CJsonAudience(string alias)
{
	m_sAlias = alias;
}

Json::Value CJsonAudience::GetJsonAudience()
{
	Json::Value jRoot;

	jRoot["alias"].append(m_sAlias);

	return jRoot;
}

// Constructor implementation
string CJPushProtoc::base64Auth;
char CJPushProtoc::m_sendBuf[4096];
CJPushProtoc::CJPushProtoc(im::ANDPushMsg *pPbMsg)
{
	m_pPbMsg = pPbMsg;
}

// Destructor implementation
CJPushProtoc::~CJPushProtoc()
{
}
string CJPushProtoc::GetSendBuf()
{
	if (!m_pPbMsg)
	{
		ErrLog("m_pPbMsg is nullptr!");
		return "";
	}

	Json::Value jroot;
	CJsonJPushNotification notify(m_pPbMsg->sbody());
	CJsonAudience audience(m_pPbMsg->sdivece_token());

	jroot["platform"] = "android";
	jroot["audience"] = audience.GetJsonAudience();
	jroot["notification"] = notify.GetJsonNotify(m_pPbMsg);

	string payload = jroot.toStyledString();

	if (CJPushProtoc::base64Auth.empty())
	{
		string authorization = CJPushClient::appKey + ":" + CJPushClient::masterSecret;
		CJPushProtoc::base64Auth = base64_encode(authorization);
	}

	memset(m_sendBuf, 0, 4096);
	sprintf(m_sendBuf, jpush_head.c_str(), CJPushProtoc::base64Auth.c_str(),
			 payload.size(), payload.c_str());

	return m_sendBuf;
}
string CJPushProtoc::GetHttpUrl() {
    return "https://api.jpush.cn/v3/push";
}

void CJPushProtoc::GetHttpHeaders(vector<string>& vstr) {

	if (CJPushProtoc::base64Auth.empty())
	{
		string authorization = CJPushClient::appKey + ":" + CJPushClient::masterSecret;
		CJPushProtoc::base64Auth = base64_encode(authorization);
	}
//只添加一个简单的授权的头部
    vstr.push_back(string("authorization: Basic ") + CJPushProtoc::base64Auth);
}

string CJPushProtoc::GetHttpPostData() {
   
	if (!m_pPbMsg)
	{
		ErrLog("m_pPbMsg is nullptr!");
		return "";
	}

	Json::Value jroot;
	CJsonJPushNotification notify(m_pPbMsg->sbody());
	CJsonAudience audience(m_pPbMsg->sdivece_token());

	jroot["platform"] = "android";
	jroot["audience"] = audience.GetJsonAudience();
	jroot["notification"] = notify.GetJsonNotify(m_pPbMsg);

	return jroot.toStyledString();
}
