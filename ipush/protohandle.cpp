//
//  protohandle.cpp
//  PushServer
//
//  Created by lang on 14/06/2017.
//  Copyright © 2017 lang. All rights reserved.
//
#include <stdio.h>
#include <string.h>

#include <json/json.h>

#include <iostream>

#include "protohandle.h"
#include "util.h"
#include "utility.hpp"
#include "jwkencodesign.h"
#include "lock.h"
#include "CApnsPostData.h"
#include "ssl_post_mgr.h"
#include "configfilereader.h"
#include "im_time.h"

#define CONFIG_FILE "server.conf"

//static const char 	*topicBunldId = "com.onlyy.kimi.voip";		//topicBunldId
static const string topicBunldId = CConfigFileReader::GetInstance(CONFIG_FILE)->GetConfigName("topic") ? CConfigFileReader::GetInstance(CONFIG_FILE)->GetConfigName("topic") : "com.onlyy.kimi.voip";
//static const string	KID_VALUE = "594632QH7J";	//issauthKeyId classCHead
static const string	KID_VALUE = CConfigFileReader::GetInstance(CONFIG_FILE)->GetConfigName("kid") ? CConfigFileReader::GetInstance(CONFIG_FILE)->GetConfigName("kid") : "594632QH7J";
static const string	ISS_UERVALUE = "PF9HQJWL24";	//teamId classCHeadiss
//static const string JWT_SIGN_KEY = "AuthKey_594632QH7J.p8";
static const string JWT_SIGN_KEY =CConfigFileReader::GetInstance(CONFIG_FILE)->GetConfigName("signkey") ? CConfigFileReader::GetInstance(CONFIG_FILE)->GetConfigName("signkey") : "AuthKey_594632QH7J.p8";
static const string JWT_HEAD = "{ \"alg\": \"ES256\", \"kid\": \"%s\" }";
static const string JWT_CLAIMS = "{ \"iss\": \"%s\", \"iat\": %d }";

static const string HEAD_BEARER_S = "bearer %s.%s";


static_assert(sizeof(CHead) < POST_BUF_SIZE, "sizeof(CHead) > POST_BUF_SIZE");
static_assert(sizeof(CPayLoad) < POST_BUF_SIZE, "sizeof(CPayLoad) > POST_BUF_SIZE");

const char *CHead::m_sMethod = "POST";
const char *CHead::m_sChemme = "https";

using namespace std;


time_t	CHead::m_timestampSecAuth = 0;
CLock	CHead::m_AuthorLock;
char	CHead::m_sAuthorization[u16AUTHJWTLEN] = { 0 };//Providertokenofjwt,https必须设置,ce如果设置,请求将被忽略!!

//pushType 消息类型 0:普通消息   1:p2p音频 2:好友邀请 3:群邀请 4:会议呼叫 5:频道聊天
enum IOS_PUSH_TYPE
{
	PUSH_TYPE_CHAT = 0,
	PUSH_TYPE_P2P_CALL = 1,
	PUSH_TYPE_CONTACTS = 2,
	PUSH_TYPE_GRP_CONTACTS = 3,
	PUSH_TYPE_REFERENCE_CALL = 4,
	PUSH_TYPE_CHNN_CHAT = 5,
    PUSH_TYPE_APPWAKEUP = 6
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
const char CAppOpenJson::type[12] = "notify_type";
const char CAppOpenJson::id[3] = "id";
const char CAppOpenJson::name[5] = "name";
const char CAppOpenJson::url[4] = "url";
const char CAppOpenJson::extra[6] = "extra";
Json::Value CAppOpenJson::GetJsonValue()
{
	Json::Value jroot;

	jroot[type] = utype;

	if (!sid.empty())
	{
		jroot[id] = sid;
	}

	if (!sname.empty())
	{
		jroot[name] = sname;
	}

	if (!surl.empty())
	{
		jroot[url] = surl;
	}

	if (!sextra.empty())
	{
		jroot[extra] = sextra;
	}

	return jroot;
}


void CAppOpenJson::Clear()
{
	utype = -1;
	sid.clear();
	sname.clear();
	surl.clear();
	sextra.clear();
}

bool CAppOpenJson::SetOpenVal(int type, const string& id, const string& str)
{
	Clear();
	utype = type;
	sid = id;

	switch (utype)
	{
	case 0:
		break;
	case 1:
		sname = str;
		break;
	case 2:
		surl = str;
		break;
	case 3:
		sextra = str;
		break;
	default:
		break;
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CPayLoad::SetAppOpenJson(int type, const string& sid, const string& str)
{
	/*
	if (type < 0 || id.empty() || str.empty())
	{
	ErrLog("SetAppOpenJson");
	return false;
	}
	*/

	return m_appOpenJson.SetOpenVal(type, sid, str);
}


CPayLoad::CPayLoad(const char *sTitle, const char *sBody)
{
	//m_vTitle_loc_args.clear();

	if (sTitle != nullptr)
	{
		m_sTitle = sTitle;
	}

	if (sBody != nullptr)
	{
		m_sBody = sBody;
	}
}

void CPayLoad::Clear()
{
	m_nBadge = 0;
	m_sTitle.clear();					//标题
	m_sBody.clear();					//消息内容
	m_appOpenJson.Clear();
}

bool CPayLoad::SetTitle(const string& sTitle)
{
	m_sTitle = sTitle;
	return true;
}

bool	CPayLoad::SetBody(const string& sBody)
{
	m_sBody = sBody;
	return true;

}

//bool	CPayLoad::SetTitle_loc_key(char *sTitle_loc_key)
//{
//	if (nullptr == sTitle_loc_key)
//	{
//		return false;
//	}
//	
//	m_sTitle_loc_key = sTitle_loc_key;
//	return true;
//}

//bool	CPayLoad::SetTitle_loc_args(vector<string> &vec)
//{
//	if (false == vec.empty())
//	{
//		return false;
//	}
//	
//	m_vTitle_loc_args.assign(vec.begin() , vec.end());
//	if (false == m_vTitle_loc_args.empty())
//	{
//		return false;
//	}
//	
//	return true;
//}

//bool	CPayLoad::SetLaunch_image(char *sLaunch_image)
//{
//	if (nullptr == sLaunch_image)
//	{
//		printf("SetLaunch_image err!\n");
//		return false;
//	}
//	
//	m_sLaunch_image = sLaunch_image;
//	
//	return true;
//}


bool CPayLoad::SetToId(const string&strToid)
{
	m_strToId = strToid;
	return true;
}



std::string CPayLoad::GetToId()
{
	return m_strToId;
}

string CPayLoad::GetJson_s()
{
	Json::Value 		root;
	Json::Value			alertValue;

	//标题
	if (!m_sTitle.empty())
	{
		alertValue["title"] = m_sTitle;
	}

	//未知
	//alertValue["subtitle"] = "my subtitle";

	//内容
	if (!m_sBody.empty()) {
		alertValue["body"] = m_sBody;
	}
	else
	{
		InfoLog("the m_sBody is empty!\n");
		return string();
	}

	//alertValue["action"] = m_appOpenJson.GetJsonValue();

	//必须成对出现
	//If a string is specified, the system displays an alert that includes the Close
	//and View buttons. The string is used as a key to get a localized string in the
	//current localization to use for the right button’s title instead of “View”.
	//锁屏界面的关闭打开和查看等国际化操作操作，暂不做实现可参考上面m_sTitle_loc_key
	//if (!m_sTitle_loc_key.empty() && !m_vTitle_loc_args.empty())
	//{
	//	alertValue["loc_key"] = m_sTitle;
	//	
	//	Json::Value locArgArr;
	//	
	//	for (uint32_t i=0; i < m_vTitle_loc_args.size(); i++)
	//	{
	//		locArgArr.append(m_vTitle_loc_args[i]);
	//	}
	//	
	//	//"loc_arg" = ["abc", "efg", "yd9"]
	//	alertValue["loc_arg"] = locArgArr;
	//}

	if (!m_sLaunch_image.empty())
	{
		alertValue["launch-image"] = m_sLaunch_image;
	}


	Json::Value apsValue;
	apsValue["action"] = m_appOpenJson.GetJsonValue();

	apsValue["badge"] = GetBadge();	//1;
	apsValue["sound"] = "chime.aiff";
	//apsValue["category"] =  "mycategory";
	apsValue["alert"] = alertValue;

	//用户自定义数据类型
	//Json::Value customInValue;
	//customInValue["mykey"] =  "myvalue";
	//Json::Value customValue;
	//customValue["custom"] = customInValue;

	root["aps"] = apsValue;
	//root["custom"] = customInValue;

	return root.toStyledString();
}

bool CPayLoad::SetBadge(int badge)
{
	m_nBadge = badge;
	return true;
}

int CPayLoad::GetBadge() const
{
	return m_nBadge;
}

bool CPayLoad::SetData(const PSvrMsg& svrMsg)
{
	if (svrMsg.sbody().empty()) return false;
	Clear();
	SetBody(svrMsg.sbody());
	SetToId(svrMsg.stoid());
	SetBadge(svrMsg.nunreadnotifymsgcount());
	SetAppOpenJson(svrMsg.emsgtype(), svrMsg.stoid(), svrMsg.stoid());
	return true;
}

void CHead::Clear()
{
	memset(m_sDeviceToken, 0, u8PATHLEN);
	memset(m_sTopic, 0, u8TOPICLEN);
	//根据不同的设备版本，确定不同的链路，以及连接方式，
	//指向全局变量
	//m_sHost.clear();

	m_sAPNsId.clear();
}

bool CHead::SetDeviceToken(const char *sDeviceToken)
{
	//printf("%s, %s\n", m_sDeviceToken, sDeviceToken);
	if (sDeviceToken == nullptr)
	{
		ErrLog("Error Device token: nullptr!\n");
		return false;
	}

	if (strlen(sDeviceToken) != u8TOKENLEN/*base 64*/)
	{
		ErrLog("Error Length Device token:%s!\n", sDeviceToken);
		return false;
	}

	if (!memset((void *)m_sDeviceToken, 0, (size_t)u8PATHLEN))
	{
		ErrLog("memset m_sPath false");
		return false;
	}
	sprintf(m_sDeviceToken, "/3/device/%s", sDeviceToken);

	//printf("%s, %s\n", m_sDeviceToken, sDeviceToken);

	return true;
}

/*{
	"alg": "ES256",
	"kid": "ABC123DEFG"
	}
	{
	"iss": "DEF123GHIJ",
	"iat": 1437179036
	}
	*/
// head->base64, body->base64,  sig = sign(head->base64+bodybase64) , sig->base64
bool CHead::SetAuthorization(const char *key)
{

	CAutoLock lock(&m_AuthorLock);

	//InfoLog("SetAuthorization");
	//If the timestamp for token issue is not within the last hour, APNs rejects 
	//subsequent push messages, returning an ExpiredProviderToken (403) error.
	//here set the time is half of a hour: 1800 = 1*60*30
	time_t tm = time(NULL);
	if (tm - CHead::m_timestampSecAuth < 1800 && CHead::m_timestampSecAuth != 0)
	{
		//InfoLog("m_timestampSecAuth  1800 true m_sAuthorization:%s", m_sAuthorization);
		return true;
	}

	CHead::m_timestampSecAuth = tm;

	if (key == nullptr)
	{
		InfoLog("Error provider key!\n");
		return false;
	}

	char head[JWT_HEAD.size() + KID_VALUE.size()];
	sprintf(head, JWT_HEAD.c_str(), KID_VALUE.c_str());

	char claims[JWT_CLAIMS.size() + ISS_UERVALUE.size() + 10]; //time`s must bit is 10 for dec
	sprintf(claims, JWT_CLAIMS.c_str(), ISS_UERVALUE.c_str(), (int)tm);

	string sBase = jwt_base64must((const unsigned char *)head);
	if (sBase.empty())
	{
		InfoLog("sBase jwt_base64must");
		return false;
	}

	string strClaims = jwt_base64must((const unsigned char *)claims);
	if (strClaims.empty())
	{
		InfoLog("sBase jwt_base64must");
		return false;
	}

	sBase = sBase + "." + strClaims;

	string sSigned = jwt_encode_str(key, sBase.c_str());
	if (sSigned.empty())
	{
		InfoLog("sBase jwt_encode_str");
		return false;
	}

	string sSignedBase = jwt_base64must((const unsigned char *)sSigned.c_str());
	if (sSignedBase.empty())
	{
		InfoLog("sSignedBase jwt_base64must");
		return false;
	}

	if (u16AUTHJWTLEN < sBase.size() + sSignedBase.size() + HEAD_BEARER_S.size())
	{
		InfoLog("size is too big, more than u16AUTHJWTLEN!");
		return  false;
	}


	if (!memset((void *)m_sAuthorization, 0, (size_t)u16AUTHJWTLEN))
	{
		InfoLog("memset m_sAuthorization false");
		return false;
	}

	sprintf(m_sAuthorization, HEAD_BEARER_S.c_str(), sBase.c_str(), sSignedBase.c_str());
	//sprintf(m_sAuthorization,"bearer .MEYCIQCAMVPvNN_");

	return true;

}

//bundle ID.ce方式,如果有多个,必须指定一个,否则可为默认,token方式必须且只有一个
bool CHead::SetTopic(const char* sTopic)
{
	if (sTopic == nullptr || strlen(sTopic) == 0)
	{
		InfoLog("SetTopic error! strlen(sTopic) == 0\n");
		return false;
	}

	if (nullptr != memcpy(m_sTopic, sTopic, strlen(sTopic)))
	{
		return true;
	}
	else
	{
		InfoLog("SetTopic memcpy error!\n");
		return false;
	}
}

//设置apns主机地址：测试，沙盒，https，feedback
//bool CHead::SetHost(const char * sHost)
//{
//	m_sHost = sHost;
//	return true;
//}

const char* CHead::GetDeviceToken() const
{
	//printf("GetDeviceToken:%s\n", m_sDeviceToken);
	return m_sDeviceToken;
}

const char* CHead::GetAuthorization() const
{
	return m_sAuthorization;
}

const char* CHead::GetTopic() const
{
	return m_sTopic;
}

//const char* CHead::GetHost() const
//{
//	return m_sHost.c_str();
//}



nghttparr *CHead::GetNghttp2Nv(int &len)
{
	//need free
	nghttparr *pNva = new nghttparr;
	if (!pNva)
	{
		ErrLog("new nghttparr[sizeof(nva)/ sizeof(nghttparr)] is null");
		return nullptr;
	}

	len = 5;
	pNva->method = str_post;
	pNva->path = m_sDeviceToken;
	pNva->apns_id = m_sAPNsId;
	pNva->apns_topic = m_sTopic;
	pNva->authorization = m_sAuthorization;

	return pNva;
}

//for Debug
//make the Head to http request Header
//bool	CHead::GetHttpHeadStr(char *str)
//{
//#ifdef HTTP2VERSION
//	if(0 == strlen(m_sAuthorization))
//	{
//		printf("the m_sAuthorization is not set, error!\n");
//		return false;
//	}
//#endif
//
//	if (/*m_sHost.empty() || */strlen(m_sTopic) == 0)
//	{
//		printf("the m_sHost or m_sTopic is not set, error!\n");
//		return false;
//	}
//	
//	sprintf(str, "%s\r\n,  %s\r\n", m_sAuthorization, /*m_sHost.c_str(),*/ m_sTopic);
//	
//	return true;
//}

//for Debug
//bool	CHead::GetHttpHeadStr_s(char * const str, uint32_t &uMemTotal)
//{
//#ifdef HTTP2VERSION
//	if(0 == strlen(m_sAuthorization))
//	{
//		printf("the m_sAuthorization is not set, error!\n");
//		return false;
//	}
//#endif
//	
//	if (/*m_sHost.empty() || */strlen(m_sTopic) == 0)
//	{
//		printf("the m_sHost or m_sTopic is not set, error!\n");
//		return false;
//	}
//	
//	size_t len = strlen(m_sAuthorization) + strlen(m_sTopic)/* + m_sHost.size()*/ + 6;
//	if (len > uMemTotal)
// 	{
//		printf("the HttpHeadStr is longger than memory size!\n");
//		return false;
//	}
//	
//	
//	//todo格式待定，需要测试
//	sprintf(str, "%s\r\n%s\r\n", m_sAuthorization/*, m_sHost.c_str()*/, m_sTopic);
//	
//	uMemTotal -= len;
//	
//	return true;
//}

//for Debug
void CHead::PrintAll()
{
	using namespace std;
	cout << "CHead print all for test" << endl;
	cout << "m_sMethod:" << m_sMethod << endl;
	cout << "m_sChemme:" << m_sChemme << endl;
	cout << "m_sPath:" << m_sDeviceToken << endl;
	cout << "m_sAuthorization:" << m_sAuthorization << endl;
	cout << "m_sTopic:" << m_sTopic << endl;
}

bool CHead::Init(/*const char* sHost,*/ const char* sTopic, const char* sAuthorization)
{
	//set the host ip
	//SetHost(sHost);

	if (!SetTopic(sTopic))
	{
		InfoLog("SetTopic");
		return false;
	}

	if (!SetAuthorization(sAuthorization))
	{
		InfoLog("SetAuthorization");
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////
//CRecvProtoBuf::CRecvProtoBuf()
//{
//	Clear();
//}
//
//CRecvProtoBuf::~CRecvProtoBuf()
//{
//	Clear();
//}
//
//
//CRecvProtoBuf::CRecvProtoBuf(char *buf, int bufLen)
//{
//	Clear();
//
//	if (nullptr == buf || bufLen < 0)
//	{
//		InfoLog("m_PSvrMsg  buf or buf length error!\n");
//	}
//	else if (!m_PSvrMsg.ParseFromArray((void *)buf, bufLen))
//	{
//		InfoLog("m_PSvrMsg.ParseFromArray error!\n");
//	}
//
//}
//
//void CRecvProtoBuf::Clear()
//{
//	m_head.Clear();
//	m_payLoad.Clear();
//	m_PSvrMsg.Clear();
//}
//
//
//bool CRecvProtoBuf::Init()
//{
//	return m_head.Init(/*sHTTP2HOST, */topicBunldId, JWT_SIGN_KEY.c_str());
//}
//
//
//bool CRecvProtoBuf::SetSendApnsMsg(char *buf, int bufLen)
//{
//	Clear();
//
//	if (nullptr == buf || bufLen < 0)
//	{
//		InfoLog("m_PSvrMsg  buf or buf length error!\n");
//		return false;
//	}
//
//	//Parse the protobuf data to m_PSvrMsg
//	if (!m_PSvrMsg.ParseFromArray((unsigned char *)buf, bufLen))
//	{
//		InfoLog("m_PSvrMsg.ParseFromArray error!\n");
//		return false;
//	}
//
//	//set the message id
//	SetMsgId(m_PSvrMsg.smsgid());
//
//	//////////////////////////////////////set head data
//	if (!m_head.Init(/*sHTTP2HOST,*/ topicBunldId, JWT_SIGN_KEY.c_str()))
//	{
//		return false;
//	}
//
//	if (!m_head.SetAPNsId(m_PSvrMsg.smsgid().c_str()))
//	{
//		InfoLog("SetAPNsId");
//		return false;
//	}
//
//	//设置设备token
//	if (!m_head.SetDeviceToken(m_PSvrMsg.stotoken().c_str()))
//	{
//		InfoLog("SetDeviceToken");
//		return false;
//	}
//
//	///////////////////////////////////////////set payLoad data
//	if (!m_payLoad.SetData(m_PSvrMsg))
//	{
//		InfoLog("Init payLoad failed");
//		return false;
//	}
//
//	return true;
//}
//
//
//void CRecvProtoBuf::SetMsgId(const string &strMsgid)
//{
//	m_strMsgId = strMsgid;
//}
//
//std::string CRecvProtoBuf::GetMsgId() const
//{
//	return m_strMsgId;
//}
//
//shared_ptr<CApnsPostData> CRecvProtoBuf::GetSendApnsBufVec()
//{
//	shared_ptr<CApnsPostData> pPostData(new CApnsPostData);
//	if (!pPostData)
//	{
//		InfoLog("new CApnsPostData error!\n");
//		return  pPostData;
//	}
//
//	pPostData->nva = m_head.GetNghttp2Nv(pPostData->navArrayLen);
//	if (!pPostData->nva)
//	{
//		InfoLog("GetNghttp2Nv is null!\n");
//		return  shared_ptr<CApnsPostData>(NULL);
//	}
//
//
//	pPostData->sToId = m_payLoad.GetToId();
//	pPostData->sMsgId = GetMsgId();
//
//	string jsonStr = m_payLoad.GetJson_s();
//	if (jsonStr.empty())
//	{
//		InfoLog("payload GetJson false!\n");
//		return  shared_ptr<CApnsPostData>(NULL);
//	}
//
//	char *body = (char *)malloc(jsonStr.size() + 1);
//	if (body == nullptr)
//	{
//
//		InfoLog("payload GetJson false!\n");
//		return  shared_ptr<CApnsPostData>(NULL);
//	}
//
//	strcpy(body, jsonStr.c_str());
//
//	pPostData->body = body;
//	pPostData->bodyLen = jsonStr.size();
//
//	InfoLog("%s\n", jsonStr.c_str());
//
//
//	return pPostData;
//}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<CApnsPostData> PostDataGenerator::GenerateAPNsPostData(const PSvrMsg& msg)
{
	std::shared_ptr<CApnsPostData> pPostData(NULL);

	//设置CHead
	static CHead	head;				//CHead vec for apns
	static bool bInit = false;
	if (!bInit)
	{
		if (!head.Init(topicBunldId.c_str(), JWT_SIGN_KEY.c_str()))
		{
			return pPostData;
		}
	}
	head.SetAPNsId(msg.smsgid().c_str());
	head.SetDeviceToken(msg.stotoken().c_str());

	//设置payLoad
	CPayLoad	payLoad;			//CPayLoad vec for apns
	if (!payLoad.SetData(msg))
	{
		InfoLog("Init payLoad failed");
		return pPostData;
	}

	//创建pPostData并设置值
	pPostData = std::shared_ptr<CApnsPostData>(new CApnsPostData);
	if (!pPostData)
	{
		return pPostData;
	}


	pPostData->nva = head.GetNghttp2Nv(pPostData->navArrayLen);
	if (!pPostData->nva)
	{
		InfoLog("GetNghttp2Nv is null!\n");
		return  shared_ptr<CApnsPostData>(NULL);
	}


	pPostData->sToId = payLoad.GetToId();
	pPostData->sMsgId = msg.smsgid();

	string jsonStr = payLoad.GetJson_s();
	if (jsonStr.empty())
	{
		InfoLog("payload GetJson false!\n");
		return  shared_ptr<CApnsPostData>(NULL);
	}

	char *body = (char *)malloc(jsonStr.size() + 1);
	if (body == nullptr)
	{

		InfoLog("payload GetJson false!\n");
		return  shared_ptr<CApnsPostData>(NULL);
	}

	strcpy(body, jsonStr.c_str());

	pPostData->body = body;
	pPostData->bodyLen = jsonStr.size();

	DbgLog("%s\n", jsonStr.c_str());


	return pPostData;

}


string PayLoadJsonStr(const string& message)
{
	Json::Value apsValue;
	//apsValue["content-available"] = "1";
	if(message.compare("cancel_p2p_call") == 0)
	{
		Json::Value categoryValue;
		categoryValue["p2p_call"] = "cancel";
		apsValue["category"]=categoryValue.toStyledString();
	}
	else
	{
		apsValue["alert"] = message;
	}

	apsValue["sound"] = "default";
	//apsValue["badge"] = 1;
	//apsValue["act"] = "Cancel";

	Json::Value 	root;
	root["aps"] = apsValue;

	return root.toStyledString();
}

string getMsg(const string& deviceToken, const string& message)
{
	string payLoad = PayLoadJsonStr(message);
	DbgLog("%s", payLoad.c_str());
	int len = 2 + 1 + deviceToken.size() + 4 + payLoad.size();
	char* msg = new char[len + 1];
	memset(msg, 0, len + 1);
	len = 2;
	len += sprintf(msg + len, "%c", 32);


	int value;
	for (unsigned int i = 0; i < (deviceToken.length() + 1) / 2; ++i)
	{
		//sscanf(string(string(deviceToken[i*2])+deviceToken[i*2+1]).c_str(), "%x", &value);
		string valueStr = deviceToken.substr(i * 2, 2);

		sscanf(valueStr.c_str(), "%x", &value);
		len += sprintf(msg + len, "%c", value);
	}

	len += sprintf(msg + len, "%c", (int)payLoad.length() / 256);
	len += sprintf(msg + len, "%c", (int)payLoad.length() % 256);

	len += sprintf(msg + len, "%s", payLoad.c_str());

	return string(msg, len);
}


string getPayLoadJsonStr(int nMsgType, const string& message)
{
	Json::Value extendValue;
	Json::Value categoryValue;
	categoryValue["pushType"] = PUSH_TYPE_CHAT;
	categoryValue["action"] = 0;
	//pushType 消息类型 0:普通消息   1:p2p音频 2:好友邀请 3:群邀请 4:会议呼叫 5:运营公告类型 6:PC Wakeup
	if(nMsgType == im::P2P_CALL)
	{
		categoryValue["pushType"] = PUSH_TYPE_P2P_CALL;
		if(message.compare("cancel_p2p_call") == 0)
			categoryValue["action"] = 1;
	}
	else if(nMsgType == im::CONTACTS)
	{
		categoryValue["pushType"] = PUSH_TYPE_CONTACTS;
	}
	else if(nMsgType == im::GRP_CONTACTS)
	{
		categoryValue["pushType"] = PUSH_TYPE_GRP_CONTACTS;
	}
	else if(nMsgType == im::REFERENCE_CALL)
	{
		categoryValue["pushType"] = PUSH_TYPE_REFERENCE_CALL;
	}
	else if(nMsgType == im::CHNN_TALK)
	{
		categoryValue["pushType"] = PUSH_TYPE_CHNN_CHAT;
	}
    else if (nMsgType == im::APP_WAKEUP)
    {
		categoryValue["pushType"] = PUSH_TYPE_APPWAKEUP;
    }
	categoryValue["extend"] = extendValue;
	Json::Value apsValue;
	apsValue["alert"] = message;
	apsValue["sound"] = "default";
	apsValue["category"] = categoryValue;
	Json::Value 	root;
	root["aps"] = apsValue;
	
	return root.toStyledString();
}


string getMsgBody(int nMsgType, const string& deviceToken, const string& message)
{
	string payLoad = getPayLoadJsonStr(nMsgType, message);
	DbgLog("%s", payLoad.c_str());
	int len = 2 + 1 + deviceToken.size() + 4 + payLoad.size();
	char* msg = new char[len + 1];
	memset(msg, 0, len + 1);
	len = 2;
	len += sprintf(msg + len, "%c", 32);


	int value;
	for (unsigned int i = 0; i < (deviceToken.length() + 1) / 2; ++i)
	{
		//sscanf(string(string(deviceToken[i*2])+deviceToken[i*2+1]).c_str(), "%x", &value);
		string valueStr = deviceToken.substr(i * 2, 2);

		sscanf(valueStr.c_str(), "%x", &value);
		len += sprintf(msg + len, "%c", value);
	}

	len += sprintf(msg + len, "%c", (int)payLoad.length() / 256);
	len += sprintf(msg + len, "%c", (int)payLoad.length() % 256);

	len += sprintf(msg + len, "%s", payLoad.c_str());

	return string(msg, len);
}



std::shared_ptr<CApnsPostData> PostDataGenerator::GenerateVoipPostData(const PSvrMsg& msg)
{
	std::shared_ptr<CApnsPostData> pPostData = std::shared_ptr<CApnsPostData>(new CApnsPostData);
	if (!pPostData)
	{
		return pPostData;
	}

	pPostData->sToId = msg.stoid();

	//string sendFN = getMsg("48d24d6c9b29389c86251203a34b777ce8ad42a0dd138db440ebd36d9e896276", "\344\275\240\346\234\211\344\270\200\344\270\252\346\226\260\347\232\204\345\221\274\345\217\253\357\274\201");

//	string sendFN = "";
//	if(msg.sappversion().empty())
//		sendFN = getMsg(msg.stotoken(),msg.sbody());
//	else
//		sendFN = getMsgBody(msg.emsgtype(), msg.stotoken(),msg.sbody());
	string sendFN = getMsgBody(msg.emsgtype(), msg.stotoken(),msg.sbody());
	char *body = (char *)malloc(sendFN.length() + 1);
	if (body == nullptr)
	{

		InfoLog("payload false!\n");
		return  shared_ptr<CApnsPostData>(NULL);
	}
	memset(body, 0, sendFN.length() + 1);

	memcpy(body, sendFN.c_str(), sendFN.length());

	pPostData->body = body;
	pPostData->bodyLen = sendFN.length();

	//DbgLog("push content %s\n", sendFN.c_str());
	return pPostData;

}
//new fun
string getiOSPayLoadJsonStr(const im::PSvrMsg& msg)
{
	Json::Value extendValue;
	Json::Value categoryValue;
	categoryValue["pushType"] = PUSH_TYPE_CHAT;
	categoryValue["action"] = 0;
	if(msg.emsgtype() == im::P2P_CALL)
	{
		categoryValue["pushType"] = PUSH_TYPE_P2P_CALL;
		Json::Value p2pValue;
		p2pValue["fromId"] = msg.sfromid();
		p2pValue["callId"] = msg.scallid();
		p2pValue["callType"] = msg.calltype();
		p2pValue["msgTime"] = (long long)getCurrentTime();
		extendValue["p2p_call"] = p2pValue;
	}
	else if(msg.emsgtype() == im::CONTACTS)
	{
		categoryValue["pushType"] = PUSH_TYPE_CONTACTS;
	}
	else if(msg.emsgtype() == im::GRP_CONTACTS)
	{
		categoryValue["pushType"] = PUSH_TYPE_GRP_CONTACTS;
	}
	else if(msg.emsgtype() == im::REFERENCE_CALL)
	{
		categoryValue["pushType"] = PUSH_TYPE_REFERENCE_CALL;
	}
	else if(msg.emsgtype() == im::CHNN_TALK)
	{
		categoryValue["pushType"] = PUSH_TYPE_CHNN_CHAT;
	}
    else if (msg.emsgtype() == im::APP_WAKEUP)
    {
		categoryValue["pushType"] = PUSH_TYPE_APPWAKEUP;
    }
	categoryValue["extend"] = extendValue;
	Json::Value apsValue;
	apsValue["alert"] = msg.sbody();
	apsValue["sound"] = "default";
	apsValue["category"] = categoryValue;
	Json::Value 	root;
	root["aps"] = apsValue;
	
	return root.toStyledString();
}


std::shared_ptr<CApnsPostData> PostDataGenerator::GenerateVoipHttp2PostData(const PSvrMsg& msg)
{

	std::shared_ptr<CApnsPostData> pPostData(NULL);

	//设置CHead
	static CHead	head;				//CHead vec for apns
	static bool bInit = false;
	if (!bInit)
	{
		if (!head.Init(topicBunldId.c_str(), JWT_SIGN_KEY.c_str()))
		{
			return pPostData;
		}
	}
	head.SetAPNsId(msg.smsgid().c_str());
	head.SetDeviceToken(msg.stotoken().c_str());

	//创建pPostData并设置值
	pPostData = std::shared_ptr<CApnsPostData>(new CApnsPostData);
	if (!pPostData)
	{
		return pPostData;
	}


	pPostData->nva = head.GetNghttp2Nv(pPostData->navArrayLen);
	if (!pPostData->nva)
	{
		InfoLog("GetNghttp2Nv is null!\n");
		return  shared_ptr<CApnsPostData>(NULL);
	}


//	pPostData->sToId = payLoad.GetToId();
    pPostData->sToId = msg.stoid();
	pPostData->sMsgId = msg.smsgid();

//	string jsonStr = payLoad.GetJson_s();
//  string jsonStr = getPayLoadJsonStr(msg.emsgtype(), msg.sbody());//get json payload
	string jsonStr = getiOSPayLoadJsonStr(msg);
	if (jsonStr.empty())
	{
		InfoLog("payload GetJson false!\n");
		return  shared_ptr<CApnsPostData>(NULL);
	}

	char *body = (char *)malloc(jsonStr.size() + 1);
	if (body == nullptr)
	{

		InfoLog("payload GetJson false!\n");
		return  shared_ptr<CApnsPostData>(NULL);
	}

	strcpy(body, jsonStr.c_str());

	pPostData->body = body;
	pPostData->bodyLen = jsonStr.size();

	DbgLog("%s\n", jsonStr.c_str());


	return pPostData;
}
