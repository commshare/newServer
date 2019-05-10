#include "mi_push_client.h"
#include "apush_server_manager.h"
#include "configfilereader.h"
#include "utility.hpp"
string CMiPushClient::appSecret;         
string CMiPushClient::appPackageName; 

bool CMiPushClient::Init() 
{
	if (!m_pConfigReader)
	{
		ErrLog("m_pConfigReader is nullptr!");
		return false;
	}

	CMiPushClient::appSecret = m_pConfigReader->GetConfigName("appSecret");
	if (CMiPushClient::appSecret.empty())
	{
		InfoLog("mi_push_post_head_app_secret_default");
		CMiPushClient::appSecret = mi_push_post_head_app_secret_default;
	}

	CMiPushClient::appPackageName = m_pConfigReader->GetConfigName("appPackageName");
	if (CMiPushClient::appPackageName.empty())
	{
		InfoLog("mi_push_post_package_name_default");
		CMiPushClient::appPackageName = mi_push_post_package_name_default;
	}

	return Regist();
};

bool CMiPushClient::Regist()
{
	m_pManage =  CApushLocalManager::GetInstance();
	if (!m_pManage)
	{
		ErrLog("CApushLocalSvr Initialize");
		return false;
	}

	if (!m_pManage->RegistClient(this, XM_PUSH))
	{
		ErrLog("CApushLocalSvr StartApp");
		return false;
	}

	return true;
};

void CMiPushClient::Start()
{
    httpStart();
}

void CMiPushClient::Stop()
{
	 
}

void CMiPushClient::AddTask(shared_ptr<HTTP_REQDATA_> data) {
    addHttpData(data, [](void* p){
                cerr << "finished : " << (char*)p << endl;
    });
}
