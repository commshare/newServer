#include "commonTaskMgr.h"
#include "configfilereader.h"
#include "friendhandle.h"
#include "grphandle.h"
#include "imappframe.h"
#include "mongoDbManager.h"
#include "msghandle.h"
#include "mysqlPool.h"
#include "offLineMsgHandle.h"
#include "offlineMsg.h"
#include "offlineMsgMgr.h"
#include "packetmgr.h"
#include "redisPool.h"
#include "sighandler.h"

CIMAppFrame::CIMAppFrame(CConfigFileReader * pReader)
	: m_pConfigReader(pReader), m_nActualServiceInst(1)
{
}
	
CIMAppFrame::~CIMAppFrame()
{
	StopApp();
}

bool CIMAppFrame::Initialize()
{
	char* pInstNumber = m_pConfigReader->GetConfigName("actual_instant_number");
	if (pInstNumber == NULL)
	{
		ErrLog("The actual service configure item : actual_instant_number not spcified!");
		return false;
	}

	m_nActualServiceInst = atoi(pInstNumber);
	if (m_nActualServiceInst > MAX_INSTANCE_SERVICE)
	{
		ErrLog("The actual service inst configure item : actual_instant_number can't over %d!", MAX_INSTANCE_SERVICE);
		return false;
	}

	CPacketMgr::GetInstance()->Initialize(m_nActualServiceInst);

	CCommonTaskMgr* pCommonTaskManager = CCommonTaskMgr::getInstance();
	if (!pCommonTaskManager)
	{
		ErrLog("CommonTaskManager create failed");
		return false;
	}

	if (pCommonTaskManager->Init(m_pConfigReader))
	{
		ErrLog("CommonTaskManager init failed");
		delete pCommonTaskManager;
		return false;
	}

	CMongoDbManager* pMongoDbManager = CMongoDbManager::getInstance();
	if (!pMongoDbManager)
	{
		ErrLog("MongoDbManager create failed");
		return false;
	}

	if (pMongoDbManager->Init(m_pConfigReader))
	{
		ErrLog("MongoDbManager init failed");
		delete pMongoDbManager;
		return false;
	}

	CDBManager* pMysqlDbManager = CDBManager::getInstance();
	if (!pMysqlDbManager)
	{
		ErrLog("MysqlDbManager create failed");
		return false;
	}

	if (pMysqlDbManager->Init(CONFIG_FILE))
	{
		ErrLog("MysqlDbManager init failed");
		delete pMysqlDbManager;
		return false;
	}

	CRedisManager* pRedisDbManager = CRedisManager::getInstance();
	if (!pRedisDbManager)
	{
		ErrLog("RedisDbManager create failed");
		return false;
	}

	if (pRedisDbManager->Init(CONFIG_FILE))
	{
		ErrLog("RedisDbManager init failed");
		delete pRedisDbManager;
		return false;
	}
	return true;
}

bool CIMAppFrame::StartApp()
{
	for (int i = 0; i < m_nActualServiceInst; i++)
	{
		// Add your statement to create your application instance.
		m_pFriendHandler[i] = new CFriendHandler(m_pConfigReader, i);
		if (!m_pFriendHandler[i])
		{
			ErrLog("create friend handle failed");
			return false;
		}
		if (false == m_pFriendHandler[i]->Initialize())
		{
			ErrLog("init friend handle failed");
			return false;
		}

		m_pMsgHandler[i] = new CMsgHandler(m_pConfigReader, i);
		if (!m_pMsgHandler[i])
		{
			ErrLog("create msg handle failed");
			return false;
		}
		if (false == m_pMsgHandler[i]->Initialize())
		{
			ErrLog("init msg handle failed");
			return false;
		}

		m_pOfflineMsgHandler[i] = new COfflineMsgHandler(m_pConfigReader, i);
		if (!m_pOfflineMsgHandler[i])
		{
			ErrLog("create offlineMsg handle failed");
			return false;
		}
		if (false == m_pOfflineMsgHandler[i]->Initialize())
		{
			ErrLog("init offlineMsg handle failed");
			return false;
		}

		m_pGrpMsgHandler[i] = new CGrpMsgHandler(m_pConfigReader, i);
		if (!m_pGrpMsgHandler[i])
		{
			ErrLog("create GrpMsg handle failed");
			return false;
		}
		if (false == m_pGrpMsgHandler[i]->Initialize())
		{
			ErrLog("init GrpMsg handle failed");
			return false;
		}

		m_pSigHandler[i] = new CSigHandler(m_pConfigReader, i);
		if (!m_pSigHandler[i])
		{
			ErrLog("create GrpMsg handle failed");
			return false;
		}
		if (false == m_pSigHandler[i]->Initialize())
		{
			ErrLog("init GrpMsg handle failed");
			return false;
		}
	}

	// End 
	return true;
	
}

void CIMAppFrame::StopApp()
{
	CMongoDbManager* pMongoDbManager = CMongoDbManager::getInstance();
	if (pMongoDbManager)
	{
		pMongoDbManager->Stop();
	}
	for (int i = 0; i < m_nActualServiceInst; i++)
	{
		if (m_pFriendHandler[i])
		{
			delete m_pFriendHandler[i];
		}
		if (m_pMsgHandler[i])
		{
			delete m_pMsgHandler[i];
		}
		if (m_pOfflineMsgHandler[i])
		{
			delete m_pOfflineMsgHandler[i];
		}
		if (m_pGrpMsgHandler[i])
		{
			delete m_pGrpMsgHandler[i];
		}
		if (m_pSigHandler[i])
		{
			delete m_pSigHandler[i];
		}
	}
}

