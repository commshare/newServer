/*****************************************************************************************
Filename: serverlinkmgr.cpp
Author: jack			Version: im-1.0 		Date:2017/6/5
Description:     		关联服务端链路管理实现，包含关联服务节点信息增删改、节点变化重连发起功能。
*****************************************************************************************/

#include "serverlinkmgr.h"

const AssociatedSvr_t imsvr::assocSvrArray[] =
{
	{CM,"cm","/uscm"},
	{MSG,"msg","/usmsg"},
	{GROUP,"group","/usgroup"},
	{IPUSH,"ipush","/usipush"},	//for ios device push
	{APUSH,"apush","/usapush"},	//for android device push
	{BPN,"bpn","/usbpn"},
	{FS, "fs", "/usfs" },		//for freeswitch
	{CSR, "csr", "/uscsr" },	//for customer service
	{NOTIFY, "notify", "/usnotify" },	//for notify service
	{LOGIN, "login", "/uslogin"}, // for login server
	{CHANNEL, "channel", "/uschannel"}, // for channel server
	{DESKTOP, "desktop", "/usdesktop"}, // for channel server
	{-1,"",""}
};

ConnMap_t g_ServerPhysicalLinkMap;	//Used in net framework to manage physical link. 

AssocSvrInfo_t* CServerLinkMgr::m_pAssocSvrInfo;					// All associated server information . 
int CServerLinkMgr::m_nAssocSvrCount;	

CServerLinkMgr::CServerLinkMgr()
{
	CServerLinkMgr::m_nAssocSvrCount = 0;
	CServerLinkMgr::m_pAssocSvrInfo = 0;
	m_pConfigReader = NULL;
	m_nAssocSvrId = 0;
	m_bAssocHeartbeatEnabled = 0;    //default is disabled . 
	m_nAssocHeartBeatInterval = 0;
	m_pLock = new CLock();
}

CServerLinkMgr::~CServerLinkMgr()
{
	if(m_pLock != NULL)  	 // thread lock definition
	{
		delete m_pLock;
		m_pLock = NULL;
	}	
}
void CServerLinkMgr::ServerLinkTimeout(
		void * callback_data, 
		uint8_t msg, 
		uint32_t handle, 
		void * pParam)
{
	ConnMap_t::iterator it;	
	ConnMap_t::iterator nextIt;	
		
	CServerLink* pLink = NULL;
	uint64_t cur_time = get_tick_count();

	//CLock* pLock = CZookeeper::GetInstance()->GetSvrLock();
	//if(pLock)
	//	CAutoLock autolock(pLock);

	for (it = g_ServerPhysicalLinkMap.begin(); it != g_ServerPhysicalLinkMap.end();) 
	{
		nextIt = it;
		++nextIt;
			
		pLink = (CServerLink*)it->second;
		pLink->AddRef();
		pLink->OnTimer(cur_time);
		pLink->ReleaseRef();
		it = nextIt;
	}
	
	// reconnect Server
	serv_check_reconnect<CServerLink>(CServerLinkMgr::m_pAssocSvrInfo,CServerLinkMgr::m_nAssocSvrCount);

}


void CServerLinkMgr::InitAssocSvr(void)
{
	uint16_t nId;
	string sPath;
	
	//parse all the associcated server id to the corresponding path name,
	//to use in zookeeper host. 
	if(!m_nAssocSvrId || (m_nAssocSvrId==INVALID_ASSOCSER_ID))    //No any associated server. 
	{
		InfoLog("There is no any of associated server, so need not to initialize associated server list!!!");
		return;
	}
	
	nId = m_nAssocSvrId & CM;
	if ( CM == nId)
	{
		sPath =  GetServicePath(CM);
		m_lsAssocatedPath.push_back(sPath);		//push associated path into associated list.
	
	}

	
	nId = m_nAssocSvrId & MSG;
	if ( MSG == nId)
	{
		sPath =  GetServicePath(MSG);
		m_lsAssocatedPath.push_back(sPath); 
	}

	nId = m_nAssocSvrId & GROUP;
	if ( GROUP == nId)
	{
		sPath =  GetServicePath(GROUP);
		m_lsAssocatedPath.push_back(sPath); 
	}

	nId = m_nAssocSvrId & IPUSH;
	if ( IPUSH == nId)
	{
		sPath =  GetServicePath(IPUSH);
		m_lsAssocatedPath.push_back(sPath); 
	}

	nId = m_nAssocSvrId & APUSH;
	if ( APUSH == nId)
	{
		sPath =  GetServicePath(APUSH);
		m_lsAssocatedPath.push_back(sPath); 
	}	

	nId = m_nAssocSvrId & BPN;
	if ( BPN == nId)
	{
		sPath =  GetServicePath(BPN);
		m_lsAssocatedPath.push_back(sPath); 
	}

	nId = m_nAssocSvrId & FS;
	if (FS == nId)
	{
		sPath = GetServicePath(FS);
		m_lsAssocatedPath.push_back(sPath);
	}

	nId = m_nAssocSvrId & CSR;
	if (CSR == nId)
	{
		sPath = GetServicePath(CSR);
		m_lsAssocatedPath.push_back(sPath);
	}

	nId = m_nAssocSvrId & LOGIN;
	if (LOGIN == nId)
	{
		sPath = GetServicePath(LOGIN);
		m_lsAssocatedPath.push_back(sPath);
	}

    nId = m_nAssocSvrId & NOTIFY;
    if (NOTIFY == nId)
    {
        sPath = GetServicePath(NOTIFY);
        m_lsAssocatedPath.push_back(sPath);
    }

	nId = m_nAssocSvrId & CHANNEL;
    if (CHANNEL == nId)
    {
        sPath = GetServicePath(CHANNEL);
        m_lsAssocatedPath.push_back(sPath);
    }
	
	nId = m_nAssocSvrId & DESKTOP;
    if (DESKTOP == nId)
    {
        sPath = GetServicePath(DESKTOP);
        m_lsAssocatedPath.push_back(sPath);
    }

	CServerLinkMgr::m_nAssocSvrCount = m_lsAssocatedPath.size(); // Allocate associcated server list and initiate. 
	CServerLinkMgr::m_pAssocSvrInfo = new AssocSvrInfo_t[CServerLinkMgr::m_nAssocSvrCount]; 
	
	int i = 0;
	for(i = 0; i < CServerLinkMgr::m_nAssocSvrCount; i++)
	{
		CServerLinkMgr::m_pAssocSvrInfo[i].nServiceId = -1;
		CServerLinkMgr::m_pAssocSvrInfo[i].nServiceCount = 0;
		CServerLinkMgr::m_pAssocSvrInfo[i].pServerList = NULL;
	}
}			

bool CServerLinkMgr::SetLocalHost(void)		//Load server host parameter from Config file. 
{
	if(!m_pConfigReader)
		return false;

	char* pIP = m_pConfigReader->GetConfigName("listen_ip");
	if(pIP==NULL)
		return false;
	m_sLocalIP = pIP;

	pIP = m_pConfigReader->GetConfigName("listen_port");
	if(pIP==NULL)
		return false;
	m_nLocalPort = atoi(pIP);

	return true;

}
bool CServerLinkMgr::SetServicePath(void)		//Set service path convert from service id.
{
	if(!m_pConfigReader)
		return false;

	char* pSvr =  m_pConfigReader->GetConfigName("local_service");
	if(pSvr==NULL)
		return false;
	
	int i;
	for(i = 0; i < MAX_ASSOCSER_TYPE; i++ )
	{
		if(INVALID_ASSOCSER_ID==imsvr::assocSvrArray[i].nServiceId)
			break;

		if(!strcmp(pSvr,imsvr::assocSvrArray[i].sServiceType.c_str()))
		{
			m_sServicePath = imsvr::assocSvrArray[i].sServicePath;
			break;
		}
	}

	return ((i < MAX_ASSOCSER_TYPE) && m_sServicePath !="") ? true : false;
}

bool CServerLinkMgr::SetAssocSvrId(void)
{
	int16_t nServerId;
	bool bRet=true;
	const char * split = "|"; 
	char * p; 

	if(!m_pConfigReader)
		return false;

	char* pSvr =  m_pConfigReader->GetConfigName("assoc_service");
	if(pSvr==NULL)
		return false;
	
	m_nAssocSvrId = 0;
	
	
	p = strtok (pSvr, split); 
	while(p!=NULL) 
	{ 
		if((nServerId = GetServiceIdByType(p))==INVALID_ASSOCSER_ID)
		{
			ErrLog("Assoc type err %p",p);
 			break;
 		}
		m_nAssocSvrId |= nServerId; 
		p = strtok(NULL, split); 
	} 

	return bRet;
}

bool CServerLinkMgr::SetSysLimitFile(void)
{
	char* pSysLimit =  m_pConfigReader->GetConfigName("limit_file");
	if(pSysLimit==NULL)
		return false;
	m_nTotalOfSysOpenfile = atoi(pSysLimit);
	m_nSizeOfSysStack = SYSTEM_STACKSIZE;
	SetSystemLimit(m_nTotalOfSysOpenfile,m_nSizeOfSysStack);
	return true;
}


bool CServerLinkMgr::SetAssocHeartbeat(void)
{
	if(!m_pConfigReader)
		return false;

	char* pTimer =  m_pConfigReader->GetConfigName("assoc_heartbeat_enable");
	if(pTimer==NULL)
		return false;
	
	m_bAssocHeartbeatEnabled = atoi(pTimer);		//Get the assoc serv heartbeat enabled state. 

	

	pTimer =  m_pConfigReader->GetConfigName("assoc_heartbeatinterval");
	if(pTimer==NULL)
		return false;
	
	m_nAssocHeartBeatInterval = atoi(pTimer)*1000;		//Millisecond
	
	return true;
}

int16_t CServerLinkMgr::GetServiceIdByType(char* pServiceStr)
{
	if(!pServiceStr)
		return -1;
	
	int16_t nServiceId = -1;
	int i;
	
	for(i = 0; i < MAX_ASSOCSER_TYPE; i++ )
	{
		if(INVALID_ASSOCSER_ID==imsvr::assocSvrArray[i].nServiceId)
			break;

		if(!strcmp(pServiceStr,imsvr::assocSvrArray[i].sServiceType.c_str()))
		{
			nServiceId = imsvr::assocSvrArray[i].nServiceId;
			break;
		}
	}

	return nServiceId;
}

int16_t CServerLinkMgr::GetServiceIdByPath(char* pPath)
{
	if(!pPath)
		return -1;
	
	int16_t nServiceId = -1;
	int i;
	
	for(i = 0; i < MAX_ASSOCSER_TYPE; i++ )
	{
		if(INVALID_ASSOCSER_ID==imsvr::assocSvrArray[i].nServiceId)
			break;

		if(!strcmp(pPath,imsvr::assocSvrArray[i].sServicePath.c_str()))
		{
			nServiceId = imsvr::assocSvrArray[i].nServiceId;
			break;
		}
	}

	return nServiceId;
}


string CServerLinkMgr::GetServicePath(int16_t nServiceId)
{
	int i;
	string sPath = "";
	for(i = 0; i < MAX_ASSOCSER_TYPE; i++ )
	{
		if(INVALID_ASSOCSER_ID==imsvr::assocSvrArray[i].nServiceId)
			break;

		else if(nServiceId==imsvr::assocSvrArray[i].nServiceId)
		{
			sPath = imsvr::assocSvrArray[i].sServicePath;
			break;
		}
	}

	return sPath;
}


void CServerLinkMgr::ConnectAssocSvr(AssocSvrList_t* pAssocSvr)		 //Connect to associated server
{
	if(!pAssocSvr)
		return;

	int i,j; 
	bool bConnectPermisson=false;  // Permit connection flag;
	AssocSvrInfo_t AssocSvrInfo; 
//	serv_info_t* pExistSvr = 0;  //a aleady exist server info. 
	serv_info_t* psvr = 0;	
	CServerLink* pLink = 0;
	imsvr::ServerInfoMap_t::iterator it;

	
	AssocSvrInfo.nServiceCount=0;
	AssocSvrInfo.nServiceId = INVALID_ASSOCSER_ID;
	AssocSvrInfo.pServerList = NULL;
	
	for(i = 0; i < CServerLinkMgr::m_nAssocSvrCount; i++)							//Transfer server list from zookeeper.
	{
		if(pAssocSvr[i].bUpdate)
		{

			MergeAssocSvr(pAssocSvr,i);      //Merge the new assoc svr and the old valid assoc svr.
			
			AssocSvrInfo.nServiceId = pAssocSvr[i].nAssocSvrId;
			AssocSvrInfo.nServiceCount = pAssocSvr[i].mapAssocSvr.size();
			AssocSvrInfo.pServerList = new serv_info_t[pAssocSvr[i].mapAssocSvr.size()];
			j = 0;
			for (it = pAssocSvr[i].mapAssocSvr.begin(); it != pAssocSvr[i].mapAssocSvr.end(); it++) 
			{
				psvr = (serv_info_t*)it->second;
				//DbgLog("new assoc svr , ip = %s : %d, link = %p",psvr->server_ip.c_str(),psvr->server_port,psvr->serv_conn);
				pLink = (CServerLink*)psvr->serv_conn;
				if(pLink)
					pLink->SetAssocSvrInfo((serv_info_t*)&AssocSvrInfo.pServerList[j]);
				AssocSvrInfo.pServerList[j] = *psvr;

				++j;
			}
			CServerLinkMgr::m_pAssocSvrInfo[i].nServiceId = AssocSvrInfo.nServiceId ;
			CServerLinkMgr::m_pAssocSvrInfo[i].nServiceCount = AssocSvrInfo.nServiceCount;
			if(CServerLinkMgr::m_pAssocSvrInfo[i].pServerList)			
			{
				delete []CServerLinkMgr::m_pAssocSvrInfo[i].pServerList; // clear the old server list
			}
			CServerLinkMgr::m_pAssocSvrInfo[i].pServerList = AssocSvrInfo.pServerList;

			//for(x = 0; x < CServerLinkMgr::m_pAssocSvrInfo[i].nServiceCount; x++)
			//{
			//	DbgLog("real assoc svr , ip = %s : %d, link = %p",CServerLinkMgr::m_pAssocSvrInfo[i].pServerList[x].server_ip.c_str(),
			//		CServerLinkMgr::m_pAssocSvrInfo[i].pServerList[x].server_port,CServerLinkMgr::m_pAssocSvrInfo[i].pServerList[x].serv_conn);
			//}
			
			pAssocSvr[i].bUpdate = false;
			bConnectPermisson = true;
		}
	}

	// start connect with the new server list and register connection overtime callback

	if(bConnectPermisson)
	{
		serv_init<CServerLink>(CServerLinkMgr::m_pAssocSvrInfo,CServerLinkMgr::m_nAssocSvrCount); 
		netlib_register_timer(CServerLinkMgr::ServerLinkTimeout, NULL, 1000);
	}
	//else
	//{
	//	netlib_delete_timer(CServerLinkMgr::ServerLinkTimeout, NULL);  // release the timer resource if no any of server to be connected . 
	//}
}

void CServerLinkMgr::MergeAssocSvr(AssocSvrList_t* pAssocSvr,int index)
{
	AssocSvrInfo_t* pAssocSvrInfo = (AssocSvrInfo_t*)&CServerLinkMgr::m_pAssocSvrInfo[index];
	if(!pAssocSvr || !pAssocSvrInfo)
		return;

	int16_t nServiceCount = pAssocSvrInfo->nServiceCount;
	serv_info_t* pSvrInfo = new serv_info_t[nServiceCount];
	ServerInfoMap_t::iterator it;
	serv_info_t* psvr = 0;
	string sHost;
	int i;

	for(i = 0; i < nServiceCount; i++)
	{

		pSvrInfo[i] = pAssocSvrInfo->pServerList[i];
				
		if(pSvrInfo[i].serv_conn)			//Ignore the old assoc svr if the svr is not connected 
		{
			sHost = GetHostString(pSvrInfo[i].server_ip, pSvrInfo[i].server_port);
			it = pAssocSvr[index].mapAssocSvr.find(sHost);
			
			// Delete the assoc svr appeared in the new associated map since the svr has aleady been connected . 
			if(it != pAssocSvr[index].mapAssocSvr.end())  
			{
				psvr = (serv_info_t*)it->second;
				if(psvr)
				{
					delete psvr;
					psvr = NULL;
				}
			
				pAssocSvr[index].mapAssocSvr.erase(sHost); 
			}
			//Merge the old valid svr to the associated map. 
			pAssocSvr[index].mapAssocSvr[sHost] = (serv_info_t*)&pSvrInfo[i]; 
		}
	}

}

string CServerLinkMgr::GetHostString(string sIp, uint16_t nPort)
{
	char str[5] = {0};
	string sHost =  sIp;

	sprintf(str, ":%04x", nPort);
	sHost += str;

	return sHost;
}

CServerLink* CServerLinkMgr::GetAssocSvrLink(int16_t nServiceId,UidCode_t sessionId) //Get a spare time link for sending messagge 
{
	CServerLink* pLink = 0;
	serv_info_t* pServerList = 0;
	int nServerCount = 0;
	int i;

	CAutoLock autolock(m_pLock);
	
	for(i = 0; i < CServerLinkMgr::m_nAssocSvrCount; i++)
	{
		if(CServerLinkMgr::m_pAssocSvrInfo[i].nServiceId==nServiceId)
		{
			nServerCount = CServerLinkMgr::m_pAssocSvrInfo[i].nServiceCount; //Get the corresponding server list.
			pServerList = CServerLinkMgr::m_pAssocSvrInfo[i].pServerList;
			
			pLink = GetRandomLink(pServerList,nServerCount,sessionId);				 // Get a random valid link 
			break;
		}
	}
	if(pLink)
		pLink->AddRef();
	
	return pLink;
}

CServerLink* CServerLinkMgr::GetRandomLink(serv_info_t* pSvrList,int nServiceCount,UidCode_t sessionId)
{
	int nLinkIndex = 0;
	int i = 0;
	CServerLink* pRandLink = NULL;

	if(!CheckValidLink(pSvrList, nServiceCount)) // 
	{
		WarnLog("There is no any valid link, so can not to allocate a link!");
		return NULL;
	}
	//string sSessionId="";
	//char sSessionBuf[64] = {0};
	
	
	//toHexLower(sSessionId, (const void*)&sessionId,UID_SIZE);
	//sprintf(sSessionBuf,"%s",sSessionId.c_str());
	//int hash_value = (int)BKDRHash(sSessionBuf);
	//while (i < nServiceCount) 
	while(true)
	{
		//nLinkIndex = ((i == 0) ? hash_value : rand())% nServiceCount;						//generate rand for selecting link. 
		nLinkIndex = rand()% nServiceCount;						//generate rand for selecting link. 
		//DbgLog("Select rand link , rand_value = %d, total = %d",rand_value,nServiceCount);
		if((pRandLink = (CServerLink*)pSvrList[nLinkIndex].serv_conn ))
		{
			if (pRandLink->IsConnect() && pRandLink->GetRegistStatus()) 	//select a valid link 
			{				
				break;
			}
		}
		i++;
		
	}

	return pRandLink;
}

bool CServerLinkMgr::CheckValidLink(serv_info_t* pSvrList,int nServiceCount)
{
	int i;
	CServerLink* pLink = NULL;

	DbgLog("Chk whether or not there is any valid link, total list = %d",nServiceCount);
	for (i = 0; i < nServiceCount; i++) 
	{
		DbgLog("server link ip=%s, port=%d",pSvrList[i].server_ip.c_str(), pSvrList[i].server_port);
		if((pLink = (CServerLink*)pSvrList[i].serv_conn))
		{
			if(pLink->IsConnect() && pLink->GetRegistStatus())
			{
				break;
			}
		}
	}
		

	if (i >= nServiceCount) 
	{
		return false;
	}

	return true;
}

vector<CServerLink*> CServerLinkMgr::GetAssocSvrLinkList(int16_t nServiceId,UidCode_t sessionId)
{
    vector<CServerLink*> vecLinks;
	CServerLink* pLink = 0;
	serv_info_t* pServerList = 0;
	int nServerCount = 0;
	int i;

	CAutoLock autolock(m_pLock);
	
	for(i = 0; i < CServerLinkMgr::m_nAssocSvrCount; i++)
	{
		if(CServerLinkMgr::m_pAssocSvrInfo[i].nServiceId==nServiceId)
		{
			nServerCount = CServerLinkMgr::m_pAssocSvrInfo[i].nServiceCount; //Get the corresponding server list.
			pServerList = CServerLinkMgr::m_pAssocSvrInfo[i].pServerList;
			
		//	pLink = GetRandomLink(pServerList,nServerCount,sessionId);				 // Get a random valid link 
            for(int j = 0; j < nServerCount; j++) 
            {
                if(pLink = (CServerLink*)pServerList[j].serv_conn) 
                {
                    if(pLink->IsConnect() && pLink->GetRegistStatus())
                    {
                        vecLinks.push_back(pLink);
                    }
                }    
            }
			break;
		}
	}
   // if(pLink)
   // 	pLink->AddRef();
    for(auto it = vecLinks.begin(); it != vecLinks.end(); it++) 
    {
        (*it)->AddRef();
    }

    //	return pLink;
    return vecLinks;
}

void CServerLinkMgr::AddLinkBySessionId(UidCode_t sessionId,CServerLink* pLink)
{
	SessionServerLinkMap_t::iterator it = m_mapSessionLink.find(sessionId);


	if(it == m_mapSessionLink.end()) // Add the link to map if link non-exist. 
	{								
		m_mapSessionLink[sessionId] = pLink;
	}

}
void CServerLinkMgr::DelLinkBySessionId(UidCode_t sessionId)
{
	SessionServerLinkMap_t::iterator it = m_mapSessionLink.find(sessionId);
	
	//CAutoLock autolock(m_pLock);
	if(it != m_mapSessionLink.end())
	{
		m_mapSessionLink.erase(sessionId);							// Erase the specific session in map
	}
}
CServerLink* CServerLinkMgr::GetLinkBySessionId(UidCode_t sessionId)
{
	CAutoLock autolock(m_pLock);

	SessionServerLinkMap_t::iterator it = m_mapSessionLink.find(sessionId);  //Search the corresponding sSessionId in map
	CServerLink* pLink = NULL;

	
	if(it!=m_mapSessionLink.end())
	{
		pLink = it->second;
		if(pLink){pLink->AddRef();} 
	}

	return pLink;
	//return (it!=m_mapSessionLink.end()) ? it->second : NULL;  
}


