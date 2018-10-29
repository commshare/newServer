#include "util.h"
#include "imappframe.h"
#include "configfilereader.h"
#include "webSvrclientProxy.h"
#include "customerSvrProxy.h"
#include "serverinfo.h"
#include "redisPool.h"
#include "packetmgr.h"


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

	//Æô¶¯redisÊý¾Ý¿â
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
	}

	// End 
	return true;
	
}

void CIMAppFrame::StopApp()
{

}

bool CIMAppFrame::HandleClientMsg(const std::string& jsonMsg)
{
	static int index = 0;
	if (m_pWebSvrClientProxys.empty())
	{
		return false;
	}
	m_pWebSvrClientProxys[index%m_pWebSvrClientProxys.size()]->PostClientMsg(jsonMsg);
	++index;
	return true;
}

bool CIMAppFrame::HandleCustomSvrMsg(const std::string& jsonMsg)
{
	static int index = 0;
	if (m_pCustomerSvrProxys.empty()) return false;

	m_pCustomerSvrProxys[index++ % m_pCustomerSvrProxys.size()]->postCustMsg(jsonMsg);
	return true;
}

std::vector<std::shared_ptr<CCustomerSvrProxy> > CIMAppFrame::m_pCustomerSvrProxys;

std::vector<std::shared_ptr<CWebSvrClientProxy>	> CIMAppFrame::m_pWebSvrClientProxys;


