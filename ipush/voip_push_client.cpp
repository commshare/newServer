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

#include "voip_push_client.h"
#include "utility.hpp"
#include "app_pushserver_manager.hpp"
#include "postDataSender.h"
#include "pushHandler.h"
#include "protohandle.h"
#include "ssl_post_mgr.h"

void ResponeCallBack(shared_ptr<CApnsPostData> data, void *userData);
bool CheckCallBack(shared_ptr<CApnsPostData> data, void *userData);


CVoipPushClient::CVoipPushClient()
	:m_bStart(false)
{
	m_clientType = PUSH_CLIENT_TYPE_VOIP;
	m_Host = "gateway.sandbox.push.apple.com";
	//m_Host = "192.168.1.93";
	m_Port = 2195;

	m_pPostMgr = new CPostMgr(m_pSslEventDispatch.get());


	//m_PostPduCacheMgr.Init(CheckCallBack, this);
}



CVoipPushClient::~CVoipPushClient()
{

}


bool CVoipPushClient::Start()
{	
	if (m_bStart) return m_bStart;
	if (!CClientBase::Start())
	{
		return false;
	}

	m_bStart = true;
	return true;
}

bool CVoipPushClient::init(CConfigFileReader* pConfigReader)
{
	char* apnsHost = pConfigReader->GetConfigName("strVoipPushHost");
	const char* sslCeFileName = pConfigReader->GetConfigName("strVoipCeFileName");
	sslCeFileName = sslCeFileName ? sslCeFileName : "kimi.pem";
	const char* sslRsaPkFileName = pConfigReader->GetConfigName("strVoipPkFileName");
	sslRsaPkFileName = sslRsaPkFileName ? sslRsaPkFileName : "kimi.pem";
	m_Host = apnsHost ? apnsHost : m_Host;

	char* Nums = pConfigReader->GetConfigName("voipConnNums");

	int connNums = 1;
	if (Nums)
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
		CPostDataSender * pSender = new CPostDataSender(m_pPostMgr, NULL);
		m_postSenders.push_back(pSender);
	}

	
	if (!m_pPostMgr->Init(connNums, ResponeCallBack, this, m_Host, m_Port, sslCeFileName, sslRsaPkFileName))
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



//bool CVoipPushClient::CompeleteTask(shared_ptr<CApnsPostData> data)
//{
//	return m_PostPduCacheMgr.Delete(data);
//}

shared_ptr<CApnsPostData> CVoipPushClient::GeneratePostDataFromMsg(const im::PSvrMsg& msg)const
{
	return PostDataGenerator::GenerateVoipPostData(msg);
}
