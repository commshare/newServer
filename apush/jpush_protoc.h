/// 
///file jpush_protoc.h 
///add by liulang 2017-09-27 
/// 

#ifndef __JPUSH_PROTOC_H_INCL__
#define __JPUSH_PROTOC_H_INCL__

#include "util.h"
#include "base64.h"
#include "jpush_protoc.h"
#include "im.push.android.pb.h"



class CJsonJPushNotification
{
public:
	CJsonJPushNotification(string body);

	Json::Value GetJsonNotify(im::ANDPushMsg *pPbMsg);

private:
	string m_sBody;
};

class CJsonAudience
{
public:
	CJsonAudience(string alias);
	Json::Value GetJsonAudience();

private:
	string m_sAlias;
};


/**
 * TODO: Add class description
 * 
 * @author   lang
 */
class CJPushProtoc
{
public:
	CJPushProtoc(im::ANDPushMsg *pPbMsg);
	virtual ~CJPushProtoc();

	string GetSendBuf();
//https
    string GetHttpUrl();
    void GetHttpHeaders(vector<string>& vstr);
    string GetHttpPostData();
private:
	im::ANDPushMsg *m_pPbMsg;

	static char m_sendBuf[4096];
	static string base64Auth;
};

#endif // __JPUSH_PROTOC_H_INCL__
