/**
* 功能: xmpp_fcm_client.h
* 日期:2017-8-29-17:32
* 作者:jobs
**/

#ifndef XMPP_FCM_CLIENT_H
#define XMPP_FCM_CLIENT_H

#include "xmpp_ssl_google_fcm_impl.h"
#include "xmpp_google_fcm_protoc.h"
#include "configfilereader.h"

#include "base_client.h"


class CXmppFcmClient : public CBaseClient
{
public:
	CXmppFcmClient(CConfigFileReader* pConfigReader);

	bool Init();

	int AddTask(shared_ptr<APushData> shared_APushData);
						 
	bool Regist();
						 
	void Start();
						 
	void Stop(){};
	 
private:

	void ThreadRunCheck();

	static void *XmppCheckHeartBeatFunc(void *argc);

	bool	m_bInit;

	int		m_curUseSession;	//当前正在使用回话, 用来分配任务给不同的链路

	CXmppImpl *m_pXmppSessionPool;	//链路数组 连接池
	int		m_numXmppSession;	//链路数量

	string 	m_sendId;
	string 	m_svrPrivateKey;

	CConfigFileReader *m_pCfg;	//配置文件
};

#endif
