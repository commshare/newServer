/**
  *  
  * file apnsclient.h 
  * create by liulang 
  *  
  * client to send apns request and recive respone 
  *  
  * 2017-07-01 
  */
#ifndef __VOIHTTP2CLIENT__	
#define __VOIHTTP2CLIENT__	

#include "utility.hpp"
#include "configfilereader.h"
#include "ssl_socket.h"

#include "postDataSender.h"
#include "clientbase.h"

class CPostDataSender;

//负责跟APNs的连接，并且发送推送请求

class CVoipHttp2Client : public CClientBase
{
public:

	CVoipHttp2Client(PUSH_CLIENT_TYPE type);
	//CVoipHttp2Client();
	
	~CVoipHttp2Client();

	bool init(CConfigFileReader* pConfigReader);

	//添加推送任务（放到任务队列中）
	//bool AddTask(shared_ptr<CApnsPostData> data);


	//用在收到响应的回调
	bool CompeleteTask(shared_ptr<CApnsPostData> data);

private:
	bool Start();

	virtual shared_ptr<CApnsPostData> GeneratePostDataFromMsg(const im::PSvrMsg& msg) const override;

private:

	bool	m_bStart;
	CPostPduCacheMgr m_PostPduCacheMgr;			//管理已经发送的推送
};

#endif //__VOIHTTP2CLIENT__
