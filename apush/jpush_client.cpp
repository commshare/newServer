#include "jpush_client.h"
#include "apush_server_manager.h"
#include "configfilereader.h"
#include "utility.hpp"

string CJPushClient::appKey;
string CJPushClient::masterSecret;

bool CJPushClient::Init() 
{
	if (!m_pConfigReader)
	{
		ErrLog("m_pConfigReader is nullptr!");
		return false;
	}

	CJPushClient::appKey = m_pConfigReader->GetConfigName("appKey");
	if (CJPushClient::appKey.empty())
	{
		InfoLog("jpush_appKey_default");
		CJPushClient::appKey = jpush_appKey_default;
	}

	CJPushClient::masterSecret = m_pConfigReader->GetConfigName("masterSecret");
	if (CJPushClient::masterSecret.empty())
	{
		InfoLog("jpush_masterSecret_default");
		CJPushClient::masterSecret = jpush_masterSecret_default;
	}
	
    return Regist();
};

bool CJPushClient::Regist()
{
	m_pManage =  CApushLocalManager::GetInstance();
	if (!m_pManage)
	{
		ErrLog("CApushLocalSvr Initialize");
		return false;
	}

	if (!m_pManage->RegistClient(this, JPUSH))
	{
		ErrLog("CApushLocalSvr StartApp");
		return false;
	}

	return true;
};

void CJPushClient::Start()
{
    httpStart();
}

void CJPushClient::Stop()
{
	 
}

void CJPushClient::AddTask(shared_ptr<HTTP_REQDATA_> data) {
    addHttpData(data, [](void* p){
                cerr << "finished : " << (char*)p << endl;
    });
}

