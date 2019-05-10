#ifndef HTTP_GOOGLE_FCM_PROTOC_H
#define HTTP_GOOGLE_FCM_PROTOC_H

#include <string>
#include "util.h"
#include "jsoncpp/json/json.h"
#include "im.push.android.pb.h"




/**
 * 
 * 
 * @author root (9/4/17)
//{
    "notify_type":"1",  //0=默认进入应用后拉取离线消息后进入会话列表页 
                        //1=去个人聊天页面 
                        //2=去群聊页面 
                        //3=去联系人页面
                        //4=跳转到我们的内嵌浏览器(可通过JavaScript和App交互)
    "id":"12345678",//uid或gid,取决于是去群聊还是个人聊天
    "name":"Tom",//个人名或群名,取决于是去群聊还是个人聊天
    "url":"http://www.xxx.com/xx.jpg",//个人头像或群头像或活动页面
    "extra":"unknow"//扩展字段
}
*/


class CHttpFcmPayLoad
{
public:
	CHttpFcmPayLoad();

	bool SetValue(const char *to, const char *msgId, const char *title, const char *body, const string &toId,int iNotify_type = 0);

	const string GetMsgIncJson();

private:
	void Init();

	bool	m_bSet;
	Json::Value m_jroot;

};

class CHttpFcmProtoc
{
public:
	CHttpFcmProtoc(im::ANDPushMsg *pPbMsg);

	string GetSendBuf();
    string GetFcmPushUrl();
    string GetFcmHeaders(vector<string>& vec);
    string GetFcmPostData();
private:
	im::ANDPushMsg *m_pPbMsg;
};

#endif
