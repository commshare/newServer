/**
 * apnsclient.cpp 
 *  
 * the apns client ,connect to Apple`s APNs Server with http2 
 * send apns request to APNs and recv respone
 *  
 * ad by liulang 
 *  
 * 2017-07-01 
 */

#include "voip_http2_client.h"
#include "utility.hpp"
#include "ssl_post_mgr.h"
#include "pushserver_manager.hpp"
#include "postDataSender.h"
#include "pushHandler.h"
#include "protohandle.h"
#include "voip_push_post_mgr.h"


static string devHost = "api.development.push.apple.com";
static string productionHost = "api.push.apple.com";


static void ResponeCallBack(shared_ptr<CApnsPostData> data, void *userData)
{
	if (!userData)
	{
		ErrLog("ResponeCallBack userData nullptr");
		return;
	}

	CVoipHttp2Client *pClient = (CVoipHttp2Client *)userData;

	if (!pClient->CompeleteTask(data))
	{
		ErrLog("CompeleteTask false");
		return;
	}
	InfoLog("push msgId:%s, tokenId:%s, toId:%s recv respone %s", data->sMsgId.c_str(), data->nva->path.c_str(), data->sToId.c_str(), data->apnsRetStatus.c_str());

	CAppPushserverManager::GetInstance()->GetServer()->OnApnsNotify(data);
}


static bool CheckCallBack(shared_ptr<CApnsPostData> data, void *userData)
{
	if (!userData)
	{
		ErrLog("CheckCallBack userData nullptr");
		return false;
	}
	if (data->sendTimes >= 3)
	{
		ErrLog("post msgId %s, toId %s already sent 3 times with no respond,abort it", data->sMsgId.c_str(), data->sToId.c_str());
		return false;
	}

	CVoipHttp2Client *pClient = (CVoipHttp2Client *)userData;

	if (!pClient->AddTask(data))
	{
		ErrLog("CConnectionsThreads AddTask");

		return false;
	}

	return true;
}

CVoipHttp2Client::CVoipHttp2Client(PUSH_CLIENT_TYPE type):m_bStart(false) {

	m_clientType = type; //http2 based token
	m_Host = "api.development.push.apple.com";
	m_Port = 443;

	m_pPostMgr = new CVoipPostMgr(m_pSslEventDispatch.get());

	m_PostPduCacheMgr.Init(CheckCallBack, this);
}

CVoipHttp2Client::~CVoipHttp2Client()
{

}


bool CVoipHttp2Client::Start()
{	
	if (m_bStart) return m_bStart;//maybe failed here
	if (!CClientBase::Start())
	{
		return false;
	}

	//创建线程监测已发送的推送是否超时无响应
	m_PostPduCacheMgr.CheckMap(10);

	m_bStart = true;
	return true;
}

bool CVoipHttp2Client::init(CConfigFileReader* pConfigReader)
{   
	char* apnsHost = NULL;
    if(m_clientType == PUSH_CLIENT_TYPE_VOIP_DEV) {
        apnsHost =  pConfigReader->GetConfigName("strDevAPNsHost");
	    m_Host = apnsHost ? apnsHost : devHost;

    } else if (m_clientType == PUSH_CLIENT_TYPE_VOIP_PRODUCTION){
        apnsHost =  pConfigReader->GetConfigName("strAPNsHost");
	    m_Host = apnsHost ? apnsHost : productionHost;
    } else {
        m_clientType = PUSH_CLIENT_TYPE_VOIP_DEV;
	    m_Host = apnsHost ? apnsHost : m_Host;
    }

	char* Nums = pConfigReader->GetConfigName("connectNums");
	int connNums = 20;
	if(Nums)
	{
		connNums = atoi(Nums);
		if (connNums <= 0 || connNums > 100)
		{
			connNums = 20;
		}
	}

	const int connPerThread = 10;
	int nSendThreadNum = (connNums + connPerThread - 1) / connPerThread;
	nSendThreadNum = nSendThreadNum > 20 ? 20 : nSendThreadNum;

	for (; nSendThreadNum > 0; --nSendThreadNum)
	{
		CPostDataSender * pSender = new CPostDataSender(m_pPostMgr, &m_PostPduCacheMgr);
		m_postSenders.push_back(pSender);
	}

	if (!m_pPostMgr->Init(connNums, ResponeCallBack, this, m_Host, m_Port/*, sslCeFileName, sslRsaPkFileName*/))
	{
		ErrLog("m_pPostMgr init");
		return false;
	}
	
	CAppPushserverManager *pManage = CAppPushserverManager::GetInstance();
	if (!pManage)
	{
		ErrLog("CAppPushserverManager GetInstance");
		return false;
	}
	if (!pManage->RegistClient(this))
	{
		ErrLog("CAppPushserverManager RegistClient");
		return false;
	}
	return Start();
}


bool CVoipHttp2Client::CompeleteTask(shared_ptr<CApnsPostData> data)
{
	return m_PostPduCacheMgr.Delete(data);
}


shared_ptr<CApnsPostData> CVoipHttp2Client::GeneratePostDataFromMsg(const im::PSvrMsg& msg)const
{
	return PostDataGenerator::GenerateVoipHttp2PostData(msg);

}
