#include "imappframe.h"
#include "configfilereader.h"
#include "packetmgr.h"
#include "groupcreate.h"
#include "groupjoin.h"
#include "groupleave.h"
#include "groupmodify.h"
#include "groupchat.h"
#include "groupnotify.h"
#include "mysqlPool.h"
#include "redisPool.h"
#include "mongoDbManager.h"
#include "mysqlGrpmemMgr.h"
#include "redisGrpMgr.h"
#include "pdusender.h"
#include "commonTaskMgr.h"

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

	if(!CMysqlHelper::GetInstance()->Initialize(m_pConfigReader))
	{
		ErrLog("Fail to initialize mysql helper");
		return false;
	}
	if(!CRedisHelper::GetInstance()->Initialize(m_pConfigReader))
	{
		ErrLog("Fail to initialize redis helper");
		return false;
	}

	CPacketMgr::GetInstance()->Initialize(m_nActualServiceInst);

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

	if (pMysqlDbManager->Init(m_pConfigReader))
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

	if (pRedisDbManager->Init(m_pConfigReader))
	{
		ErrLog("RedisDbManager init failed");
		delete pRedisDbManager;
		return false;
	}

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


	CPduSender* pPduSender = CPduSender::getInstance();
	if (NULL == pPduSender)
	{
		return false;
	}

	return true;
}

bool CIMAppFrame::StartApp()
{
	// Add your statement to create your application instance.

	//m_pCache = new CCache(m_pConfigReader);		// Redis cache instance definition
	//if(!m_pCache)
	//	return false;
	//if(false == m_pCahche->Initialize())
	//{
	//	ErrLog("Err of redis connection!");
	//	return false;
	//}
	//if(false == CMysqlHelper::GetInstance()->Initialize(m_pConfigReader)) // Initialize mysql helper
	//{
	//	ErrLog("Err of Mysql helper initialization");
	//	return false;
	//}
	bool bRet = true;
	int i;

	for(i = 0;  i < m_nActualServiceInst; i++)
	{
		m_pCreate[i] =  new CGroupCreate(m_pConfigReader,i);
		if(!m_pCreate[i] ||(false==m_pCreate[i]->Initialize(m_pConfigReader)))
		{
			bRet = false;
			break;
		}

		m_pJoin[i] = new CGroupJoin(m_pConfigReader,i);
		if(!m_pJoin[i] || (false==m_pJoin[i]->Initialize()))
		{
			bRet = false;
			break;
		}


		m_pLeave[i] = new CGroupLeave(m_pConfigReader,i);
		if(!m_pLeave[i] || (false==m_pLeave[i]->Initialize()))
		{
			bRet = false;
			break;
		}


		m_pModify[i] = new CGroupModify(m_pConfigReader,i);
		if(!m_pModify[i] || (false==m_pModify[i]->Initialize()))
		{
			bRet = false;
			break;
		}

		m_pChat[i] = new CGroupChat(m_pConfigReader,i);
		if(!m_pChat[i] || (false==m_pChat[i]->Initialize()))
		{
			bRet = false;
			break;
		}


		m_pNotify[i] = new CGroupNotify(m_pConfigReader,i);
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
		if(m_pCreate[i])
		{
			delete m_pCreate[i];
		}

		if(m_pJoin[i])
		{
			delete m_pJoin[i];
		}

		if(m_pLeave[i])
		{
			delete m_pLeave[i];
		}

		if(m_pModify[i])
		{
			delete m_pModify[i];
		}

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

//void CIMAppFrame::_testMysql()
//{
//	int count = 1000;
//	while (--count >= 0)
//	{
//		char memId[12];
//		memset(memId, 0, sizeof(memId));
//		sprintf(memId, "%d", count);
//		CMysqlGrpmemMgr::InsertGrpmem(CGrpMem("1", memId, 2));
//	}
//	CMysqlGrpmemMgr::InsertGrpmem(CGrpMem("1", "1213476", 2));
//	CMysqlGrpmemMgr::InsertGrpmem(CGrpMem("1", "1213477", 2));
//
//	//test mysql update
//	count = 1000;
//	while (--count >= 0)
//	{
//		char memId[12];
//		memset(memId, 0, sizeof(memId));
//		sprintf(memId, "%d", count);
//		CMysqlGrpmemMgr::UpdateGrpmemState("1", memId, 2);
//	}
//
//	//insert another grp 
//	count = 1000;
//	while (--count >= 0)
//	{
//		char memId[12];
//		memset(memId, 0, sizeof(memId));
//		sprintf(memId, "%d", count);
//		CMysqlGrpmemMgr::InsertGrpmem(CGrpMem("2", memId, 2));
//	}
//	CMysqlGrpmemMgr::InsertGrpmem(CGrpMem("2", "1213476", 2));
//	CMysqlGrpmemMgr::InsertGrpmem(CGrpMem("2", "1213477", 2));
//}
//
//void CIMAppFrame::_testRedis()
//{
//	std::vector<CGrpMem> grpmems = CMysqlGrpmemMgr::GetGrpmems("1");
//	for (unsigned int i = 0; i < grpmems.size(); ++i)
//	{
//		CReidsGrpMgr::InsertGrpMem(grpmems[i]);
//	}
//
//	grpmems = CMysqlGrpmemMgr::GetGrpmems("2");
//	for (unsigned int i = 0; i < grpmems.size(); ++i)
//	{
//		CReidsGrpMgr::InsertGrpMem(grpmems[i]);
//	}
//}

