#ifndef __VOIP_PUSH_CLIENT_H__
#define __VOIP_PUSH_CLIENT_H__


#include "utility.hpp"
#include "configfilereader.h"
#include "ssl_socket.h"
#include "postDataSender.h"
#include "clientbase.h"

class CPostDataSender;

//负责跟APNs的连接，并且发送推送请求

class CVoipPushClient:public CClientBase
{
public:

	CVoipPushClient();
	
	~CVoipPushClient();

	bool init(CConfigFileReader* pConfigReader);

	////用在收到响应的回调
	//bool CompeleteTask(shared_ptr<CApnsPostData> data);


private:
	bool Start();

	virtual shared_ptr<CApnsPostData> GeneratePostDataFromMsg(const im::PSvrMsg& msg)const override;

private:

	bool	m_bStart;
	//CPostPduCacheMgr m_PostPduCacheMgr;			//管理已经发送的推送

};  

#endif // __VOIP_PUSH_CLIENT_H__
