#include "imappframe.h"
#include "thread_pool_manager.h"
#include "redis_manager.h"


CIMAppFrame::CIMAppFrame(CConfigFileReader * pReader)
	: m_pConfigReader(pReader),m_nActualServiceInst(0)
{
}
	
CIMAppFrame::~CIMAppFrame()
{
	StopApp();
}


bool CIMAppFrame::Initialize()
{
	char* pInstNumber = m_pConfigReader->GetConfigName("actual_instant_number");
	if(pInstNumber==NULL)
		return false;

	m_nActualServiceInst = atoi(pInstNumber);
	if(m_nActualServiceInst > MAX_INSTANCE_SERVICE)
	{
		ErrLog("The actual service inst can't over %d!",MAX_INSTANCE_SERVICE);
		return false;
	}

	char* pCount = m_pConfigReader->GetConfigName("login_cache_pool_count");
	if(nullptr != pCount)
		CThreadPoolManager::getInstance()->initLoginCacheMgrPoolPtr(atoi(pCount), "login_cache");
	CRedisManager::getInstance()->initManager();

	CPacketMgr::GetInstance()->Initialize(m_nActualServiceInst);

	return true;
}


bool CIMAppFrame::StartApp()
{
	bool bRet = true;
	int i;

	for(i = 0;  i < m_nActualServiceInst; i++)
	{
		m_pLoginHandle[i] =  new CLoginHandle(m_pConfigReader,i);
		if(!m_pLoginHandle[i] || (false==m_pLoginHandle[i]->Initialize()))
		{
			bRet = false;
			break;
		}
	}	

	return bRet;
}

void CIMAppFrame::StopApp()
{
	int i;

	for(i = 0;  i < m_nActualServiceInst; i++)
	{
		if(m_pLoginHandle[i])
		{
			delete m_pLoginHandle[i];
		}
	}
}

