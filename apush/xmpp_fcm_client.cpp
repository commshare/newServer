/**
* 功能: xmpp_fcm_client.cpp
* 日期:2017-8-29-17:33
* 作者:jobs
**/


#include "xmpp_fcm_client.h"
#include "xmpp_fcm_client.h"
#include "apush_server_manager.h"


CXmppFcmClient::CXmppFcmClient(CConfigFileReader* pConfigReader)
{
	m_pCfg = pConfigReader;
	m_bInit = false;

	m_curUseSession = 0;
	m_numXmppSession = 0;
}

bool CXmppFcmClient::Init()
{
	if(!m_pCfg)
	{
		ErrLog("pConfigReader");
		return false;
	}

	string num = m_pCfg->GetConfigName("sessionNum");
	if(num.empty())
	{
		ErrLog("sessionNum is null");
		return false;
	}
	
	m_numXmppSession = atoi(num.c_str());
	if(m_numXmppSession< 0 || m_numXmppSession > 500)
	{
		ErrLog("sessionNum");
		return false;
	}

	m_sendId = m_pCfg->GetConfigName("sendId");
	if(m_sendId.empty())
	{
		ErrLog("sendId is null");
		return false;
	}	
	
	m_svrPrivateKey = m_pCfg->GetConfigName("svrPrivateKey");
	if(m_svrPrivateKey.empty())
	{
		ErrLog("svrPrivateKey is null");
		return false;
	}

	m_bInit = true;

	return true;
}


bool CXmppFcmClient::Regist()
{
	return CApushLocalManager::GetInstance()->RegistClient(this, GOOGLE_FCM);
}

void *CXmppFcmClient::XmppCheckHeartBeatFunc(void *arg)
{
	if (!arg)
	{
		ErrLog("CXmppFcmClient arg null");
		return nullptr;
	}

	CXmppFcmClient *fcmClient = (CXmppFcmClient *)arg;

	while (1)
	{
		fcmClient->ThreadRunCheck();

		sleep(1);
	}

	return NULL;
}

void CXmppFcmClient::ThreadRunCheck()
{
	for(int i=0; i<m_numXmppSession; i++)
	{
		if(m_pXmppSessionPool[i].CheckHeatBeat() == 0 || 
		   (time(NULL) - m_pXmppSessionPool[i].CheckHeatBeat() > 350))
		{
			WarnLog("ThreadRunCheck heartBeat time out!");
			m_pXmppSessionPool[i].CloseXmpp();
			m_pXmppSessionPool[i].XmppHandshake();
		}
	}
}

void CXmppFcmClient::Start()
{
	if(!Init())
	{
		ErrLog("CXmppFcmClient Init");
		return;
	}

	if (!m_bInit)
	{
		ErrLog("CXmppFcmClient Init");
		return;
	}


	//ErrLog("new CXmppImpl[%d]", m_numXmppSession);
	m_pXmppSessionPool = new CXmppImpl[m_numXmppSession];
	if (!m_pXmppSessionPool)
	{
		ErrLog("new CXmppImpl[m_numXmppSession]");
		exit(-1);
	}
	for (int i = 0; i < m_numXmppSession; i++)
	{
		if (m_pXmppSessionPool[i].Init(m_sendId.c_str(), m_svrPrivateKey.c_str(), this->OnNotifyCallBack, this) >= 0)
		{
			if(m_pXmppSessionPool[i].XmppHandshake() < 0)
			{
				ErrLog("XmppHandshake");
			}
		}
		else
		{
			ErrLog("CXmppImpl Init");

			//exit(-1);
			//return;
		}
	}

	Regist();

	//开启心跳检测线程
	pthread_t tid;
	pthread_create(&tid, nullptr, XmppCheckHeartBeatFunc, this);
}


static int xmppTask = 0;
int CXmppFcmClient::AddTask(shared_ptr<APushData> shared_APushData)
{

	DbgLog("CXmppFcmClient::AddTask %d", ++xmppTask);

	//循环次数
	int iCount = 0;

	while (1)
	{
		m_curUseSession++;
		iCount++;

		if(m_curUseSession == m_numXmppSession)
		{
			m_curUseSession = 0;
		}

		if (m_pXmppSessionPool[m_curUseSession].AddTask((shared_ptr<APushData>) shared_APushData) > 0)
		{
			return 1;
		}
		else
		{
			if(iCount == m_numXmppSession)
			{
				usleep(100);
				ErrLog("fcm xmpp server busy, while count:%d!", iCount);
			}
		} 
		//return -1;
	}
}

