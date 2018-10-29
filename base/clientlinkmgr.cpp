/*****************************************************************************************
Filename: clientlinkmgr.cpp
Author: jack			Version: im-1.0 		Date:2017/06/7
Description:     		客户端链路管理实现，链路侦听、链路增删查等功能
*****************************************************************************************/

#include "clientlinkmgr.h"


ConnMap_t g_ClientPhysicalLinkMap;   //Used in net framework to manage physical link. 


CClientLinkMgr::CClientLinkMgr()
	: m_pConfigReader(NULL),m_nServiceId(INVALID_ASSOCSER_ID),
	m_nClientLinkTimeout(0),m_nAssocLinkTimeout(0),m_nFlowCtrlInterval(0)
{
	m_lock = new CLock();
}

CClientLinkMgr::~CClientLinkMgr()
{
//	CClientLink* pLink = NULL;
	ConnMap_t::iterator it;
	m_mapSessionLink.clear();
	m_mapUserLink.clear();
	m_mapHostLink.clear();
	g_ClientPhysicalLinkMap.clear();
	if(m_lock != NULL)  	 // thread lock definition
	{
		delete m_lock;
		m_lock = NULL;
	}		
}

void CClientLinkMgr::ClientLinkListener(void* callback_data, uint8_t msg, uint32_t handle, 	void* pParam)
{
 	if (msg == NETLIB_MSG_CONNECT)
	{
		CClientLink* pLink = new CClientLink();		// allocate new client link, to connect new client. 
		pLink->OnConnect(handle, CClientLinkMgr::GetInstance()->GetLinkType(),
			CClientLinkMgr::GetInstance()->GetFlowCtrlInterval());
	}
	else
	{
		ErrLog("!!!error msg: %d ", msg);
	}
}

void CClientLinkMgr::ClientLinkTimeout(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	CClientLink* pLink = NULL;
	uint64_t cur_time = get_tick_count();
	ConnMap_t::iterator it;
	ConnMap_t::iterator nextIt;

	//CAutoLock autolock(CClientLinkMgr::GetInstance()->GetCliLock());

	for (it = g_ClientPhysicalLinkMap.begin(); it != g_ClientPhysicalLinkMap.end();) 
	{
		nextIt = it;
		++nextIt;
		pLink = (CClientLink*)it->second;
		pLink->AddRef();
		pLink->OnTimer(cur_time);
		pLink->ReleaseRef();
		it = nextIt;
	}
}

bool CClientLinkMgr::Initialize(CConfigFileReader* pConfigReader)		//Initiate network configuration and build a listen port. 
{
	if(NULL==pConfigReader)
		return false;
	
	m_pConfigReader = pConfigReader;
		
	if(false==SetListenIPs())
		return false;
	else if(false==SetServiceId())
		return false;
	else if(false==SetLinkTimeout())
		return false;
	else if(false==SetFlowCtrlInterval())
		return false;

	if(NETLIB_ERROR == netlib_listen(											
						m_sListenIp.c_str(),
						m_nListenPort,
						CClientLinkMgr::ClientLinkListener,
						NULL)) 
	{			
		return false;
	}
	

	netlib_register_timer(CClientLinkMgr::ClientLinkTimeout, NULL, 1000);

	return true;
}				

bool CClientLinkMgr::SetListenIPs(void)
{
	if(!m_pConfigReader)
		return false;

	char* pIP = m_pConfigReader->GetConfigName("listen_ip");
	if(pIP==NULL)
		return false;
	m_sListenIp = pIP;

	pIP = m_pConfigReader->GetConfigName("listen_port");
	if(pIP==NULL)
		return false;
	m_nListenPort = atoi(pIP);

	return true;
}

bool CClientLinkMgr::SetServiceId(void)
{
	if(!m_pConfigReader)
		return false;

	char* pSvr =  m_pConfigReader->GetConfigName("local_service");
	if(pSvr==NULL)
		return false;
	
	int i;
	for(i = 0; i < MAX_ASSOCSER_TYPE; i++ )
	{
		if(INVALID_ASSOCSER_ID==assocSvrArray[i].nServiceId)
			break;

		if(!strcmp(pSvr,assocSvrArray[i].sServiceType.c_str()))
		{
			m_nServiceId = assocSvrArray[i].nServiceId;
			break;
		}
	}

	if((i<=MAX_ASSOCSER_TYPE)&&(m_nServiceId!=INVALID_ASSOCSER_ID))
	{
		m_nLinkType  = (m_nServiceId == CM) ? MOBILE_LOGIN_LINKER :			//type of client link. 
						ASSOC_REGIST_LINKER;
	}
	
	return true;
}

bool CClientLinkMgr::SetLinkTimeout(void)
{
	if(!m_pConfigReader)
		return false;

	char* pTimer =  m_pConfigReader->GetConfigName("client_timeout");
	if(pTimer==NULL)
		return false;
	
	m_nClientLinkTimeout = atoi(pTimer)*1000;		//Millisecond


	pTimer =  m_pConfigReader->GetConfigName("assoc_heartbeat_enable");
	if(pTimer==NULL)
		return false;
		
	m_bAssocHeartbeatEnabled = atoi(pTimer);		//Get the assoc serv heartbeat enabled state. 
	

	pTimer =  m_pConfigReader->GetConfigName("assoc_timeout");
	if(pTimer==NULL)
		return false;
	
	m_nAssocLinkTimeout = atoi(pTimer)*1000;		//Millisecond


	return true;
}


bool CClientLinkMgr::SetFlowCtrlInterval(void)
{
	if(!m_pConfigReader)
		return false;

	char* pFlowCtrlInterval =  m_pConfigReader->GetConfigName("client_flowctrlinterval");
	if(pFlowCtrlInterval==NULL)
		return false;

	m_nFlowCtrlInterval = atoi(pFlowCtrlInterval);
	return true;
}

string CClientLinkMgr::GetHostString(string sIp, uint16_t nPort)
{
	char str[5] = {0};
	string sHost =  sIp;

	sprintf(str, ":%04x", nPort);
	sHost += str;

	return sHost;
}


void CClientLinkMgr::AddLinkBySessionId(UidCode_t sessionId,CClientLink* pLink)
{
	CAutoLock autolock(m_lock);
	
	SessionClientLinkMap_t::iterator it = m_mapSessionLink.find(sessionId);


	if(it == m_mapSessionLink.end()) // Add the link to map if link non-exist. 
	{								
		m_mapSessionLink[sessionId] = pLink;
	}

}
void CClientLinkMgr::DelLinkBySessionId(UidCode_t sessionId)
{
	CAutoLock autolock(m_lock);
	SessionClientLinkMap_t::iterator it = m_mapSessionLink.find(sessionId);
	
	if(it != m_mapSessionLink.end())
	{
		m_mapSessionLink.erase(sessionId);							// Erase the specific session in map
	}
}
CClientLink* CClientLinkMgr::GetLinkBySessionId(UidCode_t sessionId)
{
	CAutoLock autolock(m_lock);
	SessionClientLinkMap_t::iterator it = m_mapSessionLink.find(sessionId);  //Search the corresponding sSessionId in map
	CClientLink* pLink  = NULL;

	if(it!=m_mapSessionLink.end())
	{
		pLink = it->second;
		if(pLink){pLink->AddRef();} 
	}

	return pLink;
}
void CClientLinkMgr::AddLinkByUserId(string sUserId,CClientLink* pLink)
{
	if(sUserId=="") return;
	CAutoLock autolock(m_lock);
	m_mapUserLink[sUserId] = pLink;
}
void CClientLinkMgr::DelLinkByUserId(string       sUserId)
{
	if(sUserId=="") return;
	CAutoLock autolock(m_lock);  //the clientlink has locked ;
	ClientLinkMap_t::iterator it = m_mapUserLink.find(sUserId);
	
	if(it != m_mapUserLink.end())
	{
		m_mapUserLink.erase(sUserId);			//Erase the specific user in map
	}
}
CClientLink* CClientLinkMgr::GetLinkByUserId(string sUserId)
{
	if(sUserId=="") return NULL;

	CAutoLock autolock(m_lock);
	ClientLinkMap_t::iterator it = m_mapUserLink.find(sUserId);  //Search the corresponding sUserId in map
	CClientLink* pLink  = NULL;

	if(it!=m_mapUserLink.end())
	{
		pLink = it->second;
		if(pLink){pLink->AddRef();} 
	}

	return pLink;
}

void CClientLinkMgr::AddLinkByHost(string sIp, uint16_t nPort,CClientLink* pLink)
{
	if(sIp=="") return;

	CAutoLock autolock(m_lock);

	string sHost = GetHostString(sIp,nPort);
		
	ClientLinkMap_t::iterator it = m_mapHostLink.find(sHost);


	if(it == m_mapHostLink.end())	// Add the link to map if link non-exist. 
	{								 
		m_mapHostLink[sHost] = pLink;
	}

}

void CClientLinkMgr::DelLinkByHost(string sIp, uint16_t nPort)
{
	if(sIp=="") return;

	CAutoLock autolock(m_lock);

	string sHost = GetHostString(sIp,nPort);
	
	ClientLinkMap_t::iterator it = m_mapHostLink.find(sHost);
	
	if(it != m_mapHostLink.end())
	{
		m_mapHostLink.erase(sHost);			//Erase the specific user in map
	}
}

CClientLink* CClientLinkMgr::GetLinkByHost(string sIp, uint16_t nPort)
{
	if(sIp=="") return NULL;
	
	CClientLink* pLink	= NULL;

	CAutoLock autolock(m_lock);

	string sHost = GetHostString(sIp,nPort);
	
	ClientLinkMap_t::iterator it = m_mapHostLink.find(sHost);  //Search the corresponding sUserId in map

	if(it!=m_mapHostLink.end())
	{
		pLink = it->second;
		if(pLink){pLink->AddRef();} 
	}
	return pLink;

	//return (it!=m_mapHostLink.end()) ? it->second : NULL;  
}


