/// 
///file jpush_protoc.h 
///add by liulang 2017-09-27 
/// 

#ifndef __JPUSH_PROTOC_H_INCL__
#define __JPUSH_PROTOC_H_INCL__

#include "util.h"
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
	// Constructor
	CJPushProtoc(im::ANDPushMsg *pPbMsg);

	// Destructor
	virtual ~CJPushProtoc();

	// Copy constructor
	// TODO: Uncomment the copy constructor when you need it.
	//CJPushProtoc(const CJPushProtoc& src);

	// Assignment operator
	// TODO: Uncomment the assignment operator when you need it.
	//CJPushProtoc& operator=(const CJPushProtoc& src);

	string GetSendBuf();

private:
	im::ANDPushMsg *m_pPbMsg;

	static char m_sendBuf[4096];
	static string base64Auth;
};

#endif // __JPUSH_PROTOC_H_INCL__
