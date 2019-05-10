#ifndef HW_HANDLE_PROTOBUF_H
#define HW_HANDLE_PROTOBUF_H

#include "util.h"
#include "hw_push_client.h"
#include "im.push.android.pb.h"
#include <vector>
#include <string>
struct CHwPayloadJson;
using namespace im;

struct CHwAppOpenJson
{
	string GetJson();

	const static char notify_type[12];
	string stype;

	const static char id[3];
	string sid;

	const static char name[5];
	string sname;

	const static char url[4];
	string surl;

	const static char extra[6];
	string sextra;
};

struct CHwPayloadJson
{
public:
	CHwPayloadJson()
	{
		uMsgType = 0;
		uActionType = 0;
	}

	string GetJson();
	bool ActionParam(Json::Value &jaction);
	bool MsgBody(Json::Value &jbody);
	bool ExtMsg(Json::Value &jext);


	static char hps[4];			//hps

	static char	msg[4];			//msg
						
	static char	type[5];		//type:1,透传消息,3通知栏消息,2,4保留
	int uMsgType;

	static char	body[5];		//body

	static char content[8];		//content
	string strContent;

	static char title[6];		//title
	string strTitle;

	static char action[7];

	static char actionType[5];	//1自定义行为,2打开URL,3开特定APP
	int uActionType;

	static char param[6];

	static char intent[7];		//1自定义行为
	string strIntent;

	static char url[4];			//2打开URL
	string strUrl;

	static char appPkgName[11];	//3开特定APP
	string strAppPkgName;

	static char ext[4];			//扩展信息，含BI消息统计，特定展示风格，消息折叠

	static char biTag[6];
	string strBiTag;

	static char icon[5];
	string strIcon;

};


class CHwPostSendBuf
{

public:
	CHwPostSendBuf()
	{

	}

	~CHwPostSendBuf()
	{
	}

	static bool Init(const char* sVer, const char *sAppId);

	string PraseProtocbufToSendBuf(const char *buf, int len, const string &PushSvrToken);

    string HwGetHttpUrl();
    void HwGetHttpHeaders(vector<string>& vecHeader);
    string HwGetHttpPostData(const char *buf, int len, const string &PushSvrToken);
private:

	string _PraseData(const char *buf, int len, const string &PushSvrToken);

	bool _GetExpireTime(string &dataTime);

    static string m_strUrl;
	static string m_strHead;

	string m_strContent;
};

#endif

