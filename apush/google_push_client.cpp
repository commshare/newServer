#include "google_push_client.h"
#include "configfilereader.h"
#include "apush_server_manager.h"

string  CGooglePushClient::sendId;
string  CGooglePushClient::privateAppKey;
bool CGooglePushClient::Init() 
{
	if (!m_pConfigReader)
	{
		ErrLog("m_pConfigReader is nullptr!");
		return false;
	}

	CGooglePushClient::privateAppKey = m_pConfigReader->GetConfigName("svrPrivateKey");
	if (CGooglePushClient::privateAppKey.empty())
	{
		InfoLog("google_push_post_head_app_secret_default");
		CGooglePushClient::privateAppKey = "AIzaSyBj1FIhdGqDh-1zjFhGARs2I2bP2AlL6ik";
	}

	CGooglePushClient::sendId = m_pConfigReader->GetConfigName("sendId");
	if (CGooglePushClient::sendId.empty())
	{
		InfoLog("google_push_sendId_default");
		CGooglePushClient::sendId = "846530884595";
	}
	return Regist();
};

bool CGooglePushClient::Regist()
{
	m_pManage =  CApushLocalManager::GetInstance();
	if (!m_pManage)
	{
		ErrLog("CApushLocalSvr Initialize");
		return false;
	}

	if (!m_pManage->RegistClient(this, GOOGLE_FCM))
	{
		ErrLog("CApushLocalSvr StartApp");
		return false;
	}

	return true;
};

void CGooglePushClient::Start()
{
    httpStart();
}

void CGooglePushClient::Stop()
{
	 
}

void CGooglePushClient::AddTask(shared_ptr<HTTP_REQDATA_> data) {
    addHttpData(data, [](void* p){
                cerr << "finished : " << (char*)p << endl;
    });
}
