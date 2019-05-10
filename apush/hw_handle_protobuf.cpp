//file hw_handle_protobuf.cpp


#include "jsoncpp/json/json.h"
#include "hw_handle_protobuf.h"

static unsigned int APP_INTENT_SIZE = 512;
static unsigned int MaxContentSize = 201400;

const char CHwAppOpenJson::notify_type[12] = "notify_type";
const char CHwAppOpenJson::id[3] = "id";
const char CHwAppOpenJson::name[5] = "name";
const char CHwAppOpenJson::url[4] = "url";
const char CHwAppOpenJson::extra[6] = "extra";

string CHwAppOpenJson::GetJson()
{
	Json::Value jroot;
	
	if (!stype.empty())
	{
		jroot[notify_type] = stype.c_str();
	}
	else
	{
		return "";
	}

	if (!sid.empty())
	{
		jroot[id] = sid.c_str();
	}

	if (!sname.empty())
	{
		jroot[name] = sname.c_str();
	}

	if (!surl.empty())
	{
		jroot[url] = surl.c_str();
	}

	if (!sextra.empty())
	{
		jroot[extra] = sextra.c_str();
	}
	
	return jroot.toStyledString();
}



char CHwPayloadJson::hps[4] = "hps";			//hps
char CHwPayloadJson::msg[4] = "msg";			//msg
char CHwPayloadJson::type[5] = "type";			//type:1,透传消息,3通知栏消息,2,4保留
char CHwPayloadJson::body[5] = "body";			//
char CHwPayloadJson::content[8] = "content";
char CHwPayloadJson::title[6] = "title"; 
char CHwPayloadJson::action[7] = "action";
char CHwPayloadJson::actionType[5] = "type";	//1自定义行为,2打开URL,3开特定APP
char CHwPayloadJson::param[6] = "param";
char CHwPayloadJson::intent[7] = "intent";		//1自定义行为
char CHwPayloadJson::url[4] = "url";			//2打开URL
char CHwPayloadJson::appPkgName[11] = "appPkgName";	//3开特定APP
char CHwPayloadJson::ext[4] = "ext";			//扩展信息，含BI消息统计，特定展示风格，消息折叠
char CHwPayloadJson::biTag[6] = "biTag";
char CHwPayloadJson::icon[5] = "icon";


bool CHwPayloadJson::MsgBody(Json::Value &jbody)
{
	if (strContent.empty() || strTitle.empty())
	{
		return false;
	}
	
	if (!strContent.empty()){
		jbody[content] = strContent;
	}

	if (!strTitle.empty())
	{
		jbody[title] = strTitle;
	}

	return true;
}

bool CHwPayloadJson::ExtMsg(Json::Value &jext)
{
	if (!strBiTag.empty())
	{
		jext[biTag] = strBiTag;
	}
	else if (!strIcon.empty())
	{
		jext[icon] = strIcon;
	}
	else
	{
		return false;
	}
	
	return  true;
}

bool CHwPayloadJson::ActionParam(Json::Value &jaction)
{

	
	Json::Value jactionType;
	Json::Value jparam;
	switch (uActionType)	//1自定义行为,2打开URL,3开特定APP
	{
		
	case 1:
		if (!strIntent.empty())
		{
			jparam[intent] = strIntent;
		}
		else
		{
			return false;
		}
		break;
	case 2:
		if (!strUrl.empty())
		{
			jparam[url] = strUrl;
		}
		else
		{
			return false;
		}
		break;
	case 3:
		if (!strAppPkgName.empty())
		{
			jparam[appPkgName] = strAppPkgName;
		}
		else
		{
			return false;
		}
		break;

	default:
		return false;
	}

	jaction[actionType] = uActionType;
	jaction[param] = jparam;
	return true;
}

string CHwPayloadJson::GetJson()
{
	Json::Value jroot;
	Json::Value jhps;
	Json::Value jmsg;

	if (uMsgType != 1 && uMsgType != 3)
	{
		ErrLog("uMsgType");
		return "";
	}
	jmsg[type] = uMsgType;


	if (uMsgType == 1)
	{
		Json::Value  jbody;
		if(!MsgBody(jbody))
		{
			ErrLog("MsgBody");
			return "";
		}
		else
			jmsg[body] = jbody;
	}
	
	if (uMsgType == 3)
	{
		Json::Value jaction;
		if(!ActionParam(jaction))
		{
			ErrLog("ActionParam");
			return "";
		}

		jmsg[action] = jaction;
		jmsg[type] = uMsgType;

		
		Json::Value  jbody;
		if(!MsgBody(jbody))
		{
			ErrLog("MsgBody");
			return "";
		}
		else
			jmsg[body] = jbody;
		
	}

	Json::Value jext;
	if (ExtMsg(jext))
	{
		jhps[ext] = jext;
	}
	

	jhps[msg] = jmsg;
	jroot[hps] = jhps;
	
	return jroot.toStyledString();

}

string CHwPostSendBuf::m_strHead = "";
string CHwPostSendBuf::m_strUrl = "";
/**
 * @author root (7/18/17)
 * 
 * @param dateTime 
 * 
 * @return string& 
 */
bool CHwPostSendBuf::_GetExpireTime(string &dateTime)
{
	

	time_t rawtime = 0;
	struct tm ptm;

	time(&rawtime);
	if (rawtime <=0)
	{
		ErrLog("time return 0");
		return false;
	}

	//the utc of beijing time, and huawei Expire Time need delay a few minuters
	//29100 = (8*60*60 + 5*60);
	rawtime += 29100;

	if(!gmtime_r(&rawtime, &ptm))
	{
		ErrLog("gmtime_r return null");
		return false;
	}

	char stime[16];
	sprintf(stime, "%04d-%02d-%02d", ptm.tm_year + 1900, ptm.tm_mon + 1, ptm.tm_mday);

	char sectime[8];
	sprintf(sectime, "%02d:%02d",ptm.tm_hour, ptm.tm_min);


	string strHMMtime = rfc1738_encode(sectime);

	string strYYDtime = rfc1738_encode(stime);

	dateTime = strYYDtime + "T" + strHMMtime;

	return true;
}

string CHwPostSendBuf::_PraseData(const char *buf, int len, const string &PushSvrToken)
{
	im::ANDPushMsg pbMsg;
	pbMsg.ParseFromArray(buf, len);

	struct CHwPayloadJson hwPayloadJson;

	struct CHwAppOpenJson appdata;

	char appType[12] = {0};
	sprintf(appType, "%d", (int)pbMsg.emsgtype());
	appdata.stype = appType;
	
	string listDeviceToken;
	
    {
		static const string letfBracket = "[\"";
		static const string rightBracket = "\"]";

		string tmpToken =  pbMsg.sdivece_token();
		tmpToken = letfBracket + tmpToken + rightBracket;

		listDeviceToken = rfc1738_encode(tmpToken);
		appdata.sid = pbMsg.stoid();
	}

	string appIntentApend =  (appdata.GetJson());
	if (appIntentApend.empty())
	{
		return "";
	}
	
	if (APP_INTENT_SIZE < appIntentApend.size() + app_data_intent.size())
	{
		printf("APP_INTENT_SIZE short");
		return "";
	}
	char app_intent[APP_INTENT_SIZE];
	sprintf(app_intent, app_data_intent.c_str(), appIntentApend.c_str());

	hwPayloadJson.strIntent = app_intent;

	//all of the app is that
	hwPayloadJson.uMsgType = 3;
	hwPayloadJson.uActionType = 1;

	hwPayloadJson.strTitle = pbMsg.stitle();
	hwPayloadJson.strContent = pbMsg.sbody();

	string sPayload = hwPayloadJson.GetJson();
	if (sPayload.empty())
	{
		ErrLog("hwPayloadJson.GetJson");
		return "";
	}

	string expireTime;
	if (!_GetExpireTime(expireTime))
	{
		ErrLog("expireTime");
		return "";
	}
	
	string nsp_ts;
	char ctime[16];
	sprintf(ctime, "%u", (unsigned int)time(NULL));
	nsp_ts = ctime;

	if (MaxContentSize < postPushContectStr.size() + PushSvrToken.size() + nsp_ts.size() 
		+ listDeviceToken.size() + sPayload.size() + expireTime.size())
	{
		ErrLog("MaxContentSize");
		return "";
	}
	
	char sendbufContent[MaxContentSize];
	sprintf(sendbufContent, postPushContectStr.c_str(), PushSvrToken.c_str(),  nsp_ts.c_str(),
			listDeviceToken.c_str(), rfc1738_encode(sPayload).c_str(),  expireTime.c_str());


	return sendbufContent;
}

string CHwPostSendBuf::PraseProtocbufToSendBuf(const char *buf, int len, const string &PushSvrToken)
{
	if (!buf || len <= 0 || PushSvrToken.empty())
	{
		ErrLog("PraseProtocbufToSendBuf");
		return "";
	}
	
	m_strContent = _PraseData(buf, len, PushSvrToken);
	if (m_strContent.empty())
	{
		ErrLog("PraseProtocbufToSendBuf _PraseData");
		return "";
	}

	if (m_strHead.empty())
	{
		ErrLog("PraseProtocbufToSendBuf m_strHead");
		return "";
	}

	char contextLen[32];
	sprintf(contextLen, "%lu\r\n\n", m_strContent.size());

	return m_strHead + contextLen + m_strContent;
}

bool CHwPostSendBuf::Init(const char* sVer, const char *sAppId)
{
	if (!sVer || !sAppId)
	{
		ErrLog("cHwPostSendBuf init");
		return false;
	}

	if (max_ver_appid_size < verAppid.size() + strlen(sVer) + strlen(sAppId) + 1)
	{

		ErrLog("max_ver_appid_size");
		return false;
	}

	char strVerAppId[max_ver_appid_size];
	sprintf(strVerAppId, verAppid.c_str(), sVer, sAppId);

	char shead[max_head_size];
	sprintf(shead, postPushHeadStr.c_str(), rfc1738_encode(strVerAppId).c_str());
	m_strHead = shead;
    
    char strurl[max_head_size];
    sprintf(strurl, postRequestUrl.c_str(), rfc1738_encode(strVerAppId).c_str());
    m_strUrl = strurl;

	return true;
}

string CHwPostSendBuf::HwGetHttpUrl()
{
    return m_strUrl;
}

void CHwPostSendBuf::HwGetHttpHeaders(vector<string>& vecHeader) {
    vecHeader.push_back("");
}

string CHwPostSendBuf::HwGetHttpPostData(const char *buf, int len, const string &PushSvrToken) {

	if (!buf || len <= 0 || PushSvrToken.empty())
	{
		ErrLog("PraseProtocbufToSendBuf");
		return "";
	}
	
	return _PraseData(buf, len, PushSvrToken);
}
