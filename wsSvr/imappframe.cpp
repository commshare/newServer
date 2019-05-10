#include "util.h"
#include "imappframe.h"
#include "configfilereader.h"
#include "webSvrclientProxy.h"
#include "customerSvrProxy.h"
#include "serverinfo.h"
#include "redisMgr.h"
#include "packetmgr.h"
#include "custWebProxy.h"
#include "mongoDbManager.h"
#include "mysqlPool.h"

CIMAppFrame::CIMAppFrame() 
{
}

	
CIMAppFrame::~CIMAppFrame()
{
	StopApp();
}

bool CIMAppFrame::Initialize(CConfigFileReader * pReader)
{
	m_pConfigReader = pReader;
	m_nActualServiceInst = 1;

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

	//init RedisDbManager
	CRedisMgr * pRedisDbManager = CRedisMgr::getInstance();
	if (!pRedisDbManager)
	{
		ErrLog("RedisDbManager create failed");
		return false;
	}

	if (pRedisDbManager->init(20))
	{
		ErrLog("RedisDbManager init failed");
		delete pRedisDbManager;
		return false;
	}

	//init MongoDbManager
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

	//init MysqlDBManager
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

	//customer web mod init
	char* pCustListenPort = m_pConfigReader->GetConfigName("cust_listen_port");
	char* pWorkSize = m_pConfigReader->GetConfigName("cust_work_number");
	if(NULL == pCustListenPort)
	{
		ErrLog("The cust web configure item : cust_listen_port not spcified!");
		return false;
	}
	if(NULL == pWorkSize)
	{
		ErrLog("The cust web configure item : cust_work_number not spcified!");
		return false;
	}

	//m_custWebProxy.init(atoi(pCustListenPort), atoi(pWorkSize));
	CCustWebProxy::getInstance()->init(atoi(pCustListenPort), atoi(pWorkSize));
	
	return true;
	
}

bool CIMAppFrame::StartApp()
{
	for (int i = 0; i < m_nActualServiceInst; i++)
	{
		//Add your statement to create your application instance.
		std::shared_ptr<CCustomerSvrProxy> pCustomerSvrProxy = std::shared_ptr<CCustomerSvrProxy>(new CCustomerSvrProxy(m_pConfigReader, i));
		if (!pCustomerSvrProxy)
		{
			ErrLog("create CCustomerSvrProxy failed");
			return false;
		}
		if (false == pCustomerSvrProxy->Initialize())
		{
			ErrLog("init friend handle failed");
			return false;
		}
		m_pCustomerSvrProxys.push_back(pCustomerSvrProxy);

		#if 1
		std::shared_ptr<CWebSvrClientProxy> pWebSvrClientProxy = std::shared_ptr<CWebSvrClientProxy>(new CWebSvrClientProxy(m_pConfigReader));
		if (!pWebSvrClientProxy)
		{
			ErrLog("create CWebSvrClientProxy failed");
			return false;
		}
		if (false == pWebSvrClientProxy->Initialize())
		{
			ErrLog("init CWebSvrClientProxy failed");
			return false;
		}
		m_pWebSvrClientProxys.push_back(pWebSvrClientProxy);
		#endif
	}

	// End 
	return true;
	
}

void CIMAppFrame::StopApp()
{

}

bool CIMAppFrame::HandleClientMsg(const std::string& jsonMsg)
{
	#if 1
	static int index = 0;
	if (m_pWebSvrClientProxys.empty())
	{
		return false;
	}
	m_pWebSvrClientProxys[index%m_pWebSvrClientProxys.size()]->PostClientMsg(jsonMsg);
	++index;
	return true;
	#endif
}

bool CIMAppFrame::HandleCustomSvrMsg(const std::string& jsonMsg)
{
	#if 1
	static int index = 0;
	if (m_pCustomerSvrProxys.empty()) return false;

	//m_pCustomerSvrProxys[index++ % m_pCustomerSvrProxys.size()]->postCustMsg(jsonMsg);
	return true;
	#endif
}

std::vector<std::shared_ptr<CCustomerSvrProxy> > CIMAppFrame::m_pCustomerSvrProxys;

std::vector<std::shared_ptr<CWebSvrClientProxy>	> CIMAppFrame::m_pWebSvrClientProxys;


