/// 
///file jpush_protoc.cpp 
///add by liulang 2017-09-27 
/// 

#include "jsoncpp/json/json.h"
#include "jpush_protoc.h"
#include "protobuf_phase.h"
#include "jpush_client.h"

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

//'{
//	"platform":"android",
//	"audience":
//	{
//		"alias" : [ "liulang1"]
//	},
//	"notification": 
//	{
// 	 	"alert":"Hi,JPush 111!",
// 	 	"android":
//		{
//			"extras":
//			{
//				"android-key1":"android-value1"
//			}
//		}
//	}
//}


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

// TODO: Uncomment the copy constructor when you need it.
//inline CJPushProtoc::CJPushProtoc(const CJPushProtoc& src)
//{
//   // TODO: copy
//}

// TODO: Uncomment the assignment operator when you need it.
//inline CJPushProtoc& CJPushProtoc::operator=(const CJPushProtoc& rhs)
//{
//   if (this == &rhs) {
//      return *this;
//   }
//
//   // TODO: assignment
//
//   return *this;
//}
