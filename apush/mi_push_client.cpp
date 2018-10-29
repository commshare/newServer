#include "mi_push_client.h"

// TODO: your implementation here


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

	string num = m_pConfigReader->GetConfigName("miSessionNum");
	if(num.empty())
	{
		ErrLog("sessionNum is null use defalut 100 connects");
		m_uConnectNum = 100;
	}
	else
	{
		m_uConnectNum = atoi(num.c_str());
		if(m_uConnectNum< 0 || m_uConnectNum > 2000)
		{
			ErrLog("miSessionNum error, use defalut 100 connects");
			m_uConnectNum = 100;
			//return false;
		}
	}

	m_pPostPoolMgr = new CPostPoolMgr;
	if (!m_pPostPoolMgr)
	{
		ErrLog("m_pPostPoolMgr is null");
		return false;
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
	if (!m_pPostPoolMgr)
	{
		ErrLog("m_pPostPoolMgr is nullptr");
		return;
	}
	if (m_uConnectNum < 0 || m_uConnectNum > 2000)
	{
		InfoLog("m_uConnectNum < 0 || m_uConnectNum > 2000");
		m_uConnectNum = 100;
	}

	m_pPostPoolMgr->Init(m_uConnectNum, CBaseClient::OnNotifyCallBack, this, "api.xmpush.xiaomi.com", 443);
}

void CMiPushClient::Stop()
{
	 
}

int CMiPushClient::AddTask(shared_ptr<APushData> data)
{
	return m_pPostPoolMgr->Post(data);
}

