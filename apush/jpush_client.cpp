#include "jpush_client.h"


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

	string num = m_pConfigReader->GetConfigName("jSessionNum");
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

	m_pPostPoolMgr->Init(m_uConnectNum, CBaseClient::OnNotifyCallBack, this, jpush_webSit.c_str(), 443);
}

void CJPushClient::Stop()
{
	 
}

int CJPushClient::AddTask(shared_ptr<APushData> data)
{
	return m_pPostPoolMgr->Post(data);
}

