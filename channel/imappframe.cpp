#include "imappframe.h"
#include "configfilereader.h"
#include "packetmgr.h"
#include "channel_chat.h"
#include "channel_notify.h"
#include "redisMgr.h"
#include "mongoDbManager.h"
#include "channelMongoOperator.h"
#include "common.h"
#include <thread>

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
	
	CPacketMgr::GetInstance()->Initialize(m_nActualServiceInst);

	CMongoDbManager* pMongoDbManager = CMongoDbManager::getInstance();
	if (!pMongoDbManager)
	{
		ErrLog("MongoDbManager create failed");
		return false;
	}

	if (pMongoDbManager->Init(m_pConfigReader, false))
	{
		ErrLog("MongoDbManager init failed");
		delete pMongoDbManager;
		return false;
	}

	char* name  = m_pConfigReader->GetConfigName("currmongodb_name");
	if(nullptr != name)
		ChannelMongoOperator::getInstance()->initCurrDbName(name);

	//init RedisDbManager
	CRedisMgr * pRedisDbManager = CRedisMgr::getInstance();
	if (!pRedisDbManager)
	{
		ErrLog("RedisDbManager create failed");
		return false;
	}

	if (!pRedisDbManager->init())
	{
		ErrLog("RedisDbManager init failed");
		delete pRedisDbManager;
		return false;
	}

	char* pUrl = m_pConfigReader->GetConfigName("chnnInfoListUrl");
	char* pSecret = m_pConfigReader->GetConfigName("check_appsecret");
	if(nullptr == pUrl || nullptr == pSecret)
		return false;
	std::thread initChnn([this, pUrl, pSecret]{
		std::vector<CHANNEL_INIFO_> vecChnnInfo;
		std::string strCode = "";
		if(CRedisMgr::getInstance()->getChannelInfoList(pUrl, pSecret, vecChnnInfo, strCode) && vecChnnInfo.size() > 0)
			CRedisMgr::getInstance()->setChannelInfoList(vecChnnInfo);
	});
	initChnn.detach();
	
	return true;
}

bool CIMAppFrame::StartApp()
{
	bool bRet = true;
	for(int i = 0;  i < m_nActualServiceInst; i++)
	{
		m_pChat[i] = new CChannelChat(m_pConfigReader,i);
		if(!m_pChat[i] || (false==m_pChat[i]->Initialize()))
		{
			bRet = false;
			break;
		}

		m_pNotify[i] = new CChannelNotify(m_pConfigReader,i);
		if(!m_pNotify[i] || (false==m_pNotify[i]->Initialize()))
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
	
	for(i = 0;	i < m_nActualServiceInst; i++)
	{
		if(m_pChat[i])
		{
			delete m_pChat[i];
		}
		
		if(m_pNotify[i])
		{
			delete m_pNotify[i];
		}	
	}

}

