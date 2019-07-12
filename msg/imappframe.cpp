#include "commonTaskMgr.h"
#include "configfilereader.h"
#include "friendhandle.h"
#include "imappframe.h"
#include "mongoDbManager.h"
#include "msghandle.h"
//#include "mysqlPool.h"
#include "offLineMsgHandle.h"
#include "offlineMsg.h"
#include "offlineMsgMgr.h"
#include "packetmgr.h"
#include "redisPool.h"
#include "sighandler.h"
#include "notify_handle.h"
#include "groupChatHandle.h"
#include "exchangeKeyHandle.h"
#include "thread_pool_manager.h"

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

#if 0
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
#endif

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

	char* cache_size = m_pConfigReader->GetConfigName("group_msg_thread_num");
	int nCount = cache_size ? atoi(cache_size) : 40;
	CThreadPoolManager::getInstance()->initGroupMsgSendPoolPtr(nCount, "sendGroupMsg");
	
	return true;
}

bool CIMAppFrame::StartApp()
{
	for (int i = 0; i < m_nActualServiceInst; i++)
	{
		// Add your statement to create your application instance.
//		m_pFriendHandler[i] = new CFriendHandler(m_pConfigReader, i);
//		if (!m_pFriendHandler[i])
//		{
//			ErrLog("create friend handle failed");
//			return false;
//		}
//		if (false == m_pFriendHandler[i]->Initialize())
//		{
//			ErrLog("init friend handle failed");
//			return false;
//		}

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

		// 通知消息
		m_pNotifyHandle[i] = new CNotifyHandle(m_pConfigReader, i);
		if (!m_pNotifyHandle[i])
		{
			ErrLog("create notify handle failed");
			return false;
		}
		if (false == m_pNotifyHandle[i]->Initialize())
		{
			ErrLog("init notify handle failed");
			return false;
		}
		// 群组消息处理
		m_pGroupChatHandle[i] = new CGroupChatHandle(m_pConfigReader, i);
		if (!m_pGroupChatHandle[i])
		{
			ErrLog("create group chat handle failed");
			return false;
		}
		if (false == m_pGroupChatHandle[i]->Initialize())
		{
			ErrLog("init group chat handle failed");
			return false;
		}

		// 密钥交换消息处理
		m_pExchangeKeyHandle[i] = new CExchangeKeyHandle(m_pConfigReader, i);
		if (!m_pExchangeKeyHandle[i])
		{
			ErrLog("create exchange key handle failed");
			return false;
		}
		if (false == m_pExchangeKeyHandle[i]->Initialize())
		{
			ErrLog("init exchange key handle failed");
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
//		if (m_pFriendHandler[i])
//		{
//			delete m_pFriendHandler[i];
//		}
		if (m_pMsgHandler[i])
		{
			delete m_pMsgHandler[i];
		}
		if (m_pOfflineMsgHandler[i])
		{
			delete m_pOfflineMsgHandler[i];
		}
		if (m_pSigHandler[i])
		{
			delete m_pSigHandler[i];
		}
		if(m_pGroupChatHandle[i])
		{
			delete m_pGroupChatHandle[i];
		}
		if(m_pExchangeKeyHandle[i])
		{
			delete m_pExchangeKeyHandle[i];
		}
	}
}

