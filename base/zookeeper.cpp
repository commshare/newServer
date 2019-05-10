/*****************************************************************************************
Filename:  zookeeper.cpp
Author: jack			Version: im-1.0 		Date:2017/6/6
Description:   网络层zookeeper网络监测类实现。该类负责各关联服务的注测、链路维护、关联服务节点信息拉取
*****************************************************************************************/

#include "zookeeper.h"

CZookeeper::CZookeeper() 
	:m_zHandle(NULL),m_pAssocSvrList(NULL), m_bConnect(false), 
	m_bRegist(false),m_bRunning(false), m_nSessionId(-1)
{
	memset(m_sRegistName, 0, REGISTNODE_SIZE);
}

CZookeeper::~CZookeeper()
{
	if(m_bRegist)
	{
		zoo_delete(m_zHandle, m_sRegistName, -1);
		m_bRegist = false;
	}
	else if(m_pAssocSvrList)
	{
		delete []m_pAssocSvrList;
		m_pAssocSvrList = NULL;
	}
	Finalize();
	StopThread();
}

//void* CZookeeper::StartRoutine(void* arg)
//{
//}
void CZookeeper::StartThread()
{
	m_bRunning = true;
	CThread::StartThread();
}
void CZookeeper::StopThread()
{
	m_bRunning = false;

}
void CZookeeper::OnThreadRun(void)
{
	//int nTimes = 0;
	int i = 0;
	string sPath = "";
	AssociatedPathList_t::iterator it;

	//Associate server array allocate and initiate.
	m_pAssocSvrList = new AssocSvrList_t[m_lsAssocatedPath.size()];
	for (it = m_lsAssocatedPath.begin(); it != m_lsAssocatedPath.end(); it++) //Initialize struct list of assoc svr;
	{
		sPath = *it;
		m_pAssocSvrList[i].bUpdate = false;
		m_pAssocSvrList[i].nAssocSvrId = GetServiceIdByPath((char*)sPath.c_str());
		m_pAssocSvrList[i].mapAssocSvr.clear();
		i++;
	}
		
	while(m_bRunning)
	{
		ConnectZookeeper();
		RegistService();
		sleep(m_nRetryInterval);
	}

}

void CZookeeper::Event( zhandle_t* zh, int type, int state,const char* path,void* context)
{
	//DbgLog("Event type %d and the path is %s",type,path);
	if (type == ZOO_SESSION_EVENT) 
	{
		if (state == ZOO_CONNECTED_STATE) 		// Success  to connect zookeeper
		{
			//m_bConnect = true;
			InfoLog("Success to connect zookeeper host!");
			CZookeeper::GetInstance()->SetConnect(true);
		} 
		else if (state == ZOO_AUTH_FAILED_STATE) 
		{
			//ErrLog("Authentication failure.");
		} 
		else if (state == ZOO_EXPIRED_SESSION_STATE) 
		{
			ErrLog("Session expired.");
			//m_bConnect = false;
			CZookeeper::GetInstance()->ResetConnect();
		}
		else
		{
			//ErrLog("state=%d.", state);
		}
	}
	else if(type == ZOO_CHILD_EVENT)
	{
		DbgLog("zk child event: UpdateAssocSvr");
		CZookeeper::GetInstance()->UpdateAssocSvr(path);	//Update associated server list. 
	}

}

bool CZookeeper::Initialize(CConfigFileReader* pConfigReader)	
{
	if(!pConfigReader)
		return false;

	m_pConfigReader = pConfigReader;
	if(SetZookeeper()==false)
		return false;
	if(SetLocalHost()==false)
		return false;
	if(SetServicePath()==false)
		return false;
	if(SetAssocSvrId()==false)
		return false;
	if(SetSysLimitFile()==false)
		return false;

	InitAssocSvr();
	
	return true;
}

bool CZookeeper::SetZookeeper(void)
{
	if(!m_pConfigReader)
		return false;

	char* pZk = m_pConfigReader->GetConfigName("zookeeper_host");
	if(pZk==NULL)
		return false;
	m_sZookeeperUrl = pZk;

	pZk = m_pConfigReader->GetConfigName("zookeeper_timeout");
	if(pZk==NULL)
		return false;
	m_nTimeout = atoi(pZk);

	
	pZk = m_pConfigReader->GetConfigName("zookeeper_retrytimes");
	if(pZk==NULL)
		return false;
	m_nRetryTimes = atoi(pZk);


	
	pZk = m_pConfigReader->GetConfigName("zookeeper_retryinterval");
	if(pZk==NULL)
		return false;
	m_nRetryInterval = atoi(pZk);
	

	return true;
}


void CZookeeper::Finalize()						//Release zookeeper connection. 
{
	int ret = zookeeper_close(m_zHandle);
	if (ret != ZOK) {
	 	ErrLog("Failed to cleanup ZooKeeper, zookeeper_close: ");
	}
	m_zHandle = NULL;
	m_bConnect =  false;
}

void CZookeeper::ConnectZookeeper()
{
	
	//if (m_zHandle) {
    //	zookeeper_close(m_zHandle);
	//	m_zHandle = 0;
  	//}		

	int nRetryTimes = 0;
	while (!m_zHandle && (nRetryTimes < m_nRetryTimes)) 
	{
		//创建与ZooKeeper服务端通信的句柄和对应于此句柄的会话。会话的创建过程是异步的，收到ZOO_CONNECTED_STATE状态事件后，确认会话成功建立。
		m_zHandle = zookeeper_init(m_sZookeeperUrl.c_str(),CZookeeper::Event,m_nTimeout,0,0,0); //Connect to zookeeper
		++nRetryTimes;					
	}

	if(nRetryTimes >= m_nRetryTimes)
	{
		ErrLog("failed to connect zookeeper host %s ,over maxtimes %d",m_sZookeeperUrl.c_str(),m_nRetryTimes);
	}
}
bool CZookeeper::RegistService()				// Register a node information to zookeeper host
{	
	char str[5] ={0}; 
	string sPath;

	if (!m_zHandle || !m_bConnect || m_bRegist)
		return false;


	sprintf(str, ":%04x", m_nLocalPort);
	sPath = m_sServicePath;
    //check service node
    if(zoo_exists(m_zHandle, m_sServicePath.c_str(), 1, NULL) != 0) 
    {
        InfoLog("node '%s' doesn't exists! we will create this node follow.", m_sServicePath.c_str());
        int pos = m_sServicePath.find("\/", 0);
        if (pos != -1) {
            string znodedata =  m_sServicePath.substr(pos+1, m_sServicePath.length() - pos-1);
            int rc = zoo_create(
                                m_zHandle,
                                sPath.c_str(),
                                znodedata.c_str(),
                                znodedata.size(),
                                &ZOO_OPEN_ACL_UNSAFE,
                                0,
                                m_sRegistName,
                                REGISTNODE_SIZE);
            if(!rc) {
                InfoLog("The server has created node '%s' successfully to zookeeper host!",m_sRegistName);
            }
        }   
    } else {
        InfoLog("node %s exisits, come on", m_sServicePath.c_str());
    } 

    //create tmp node
	sPath += "/node_";
	sPath += m_sLocalIP;
	sPath += str;

    int rc = zoo_create(
                m_zHandle,
                sPath.c_str(),
                m_sLocalIP.c_str(),
                m_sLocalIP.size(),
                &ZOO_OPEN_ACL_UNSAFE,
                ZOO_EPHEMERAL,
                m_sRegistName,
                REGISTNODE_SIZE);

	if(rc)
	{
        ErrLog("Failed to regist service path = %s, error = %d",sPath.c_str(), rc);
		Finalize();
		return false;
	}
	InfoLog("The server has registed successfully to zookeeper host! : %s",m_sRegistName);

	m_bRegist = true;
	PullAssocSvr();
	return true;
}

void CZookeeper::ResetConnect(void)
{
	if(m_bConnect)
	{
		if(m_bRegist)
		{
			zoo_delete(m_zHandle, m_sRegistName, -1);
			m_bRegist = false;			
		}
		Finalize();
	}
}
bool CZookeeper::PullAssocSvr(void)								//Update associate server list from zookeeper
{
	//if (!m_zHandle || !m_bConnect) 
	//{	
	//	ConnectZookeeper();	
	//}	
	
	
	CAutoLock autolock(m_pLock);

	//Get all associated server information from zookeeper host. 
	//DbgLog("Pulling associated server ...");
	int nRet;
	int i = 0;
	string sPath;
	struct String_vector nodeinfo;  
	AssociatedPathList_t::iterator it;
	for (it = m_lsAssocatedPath.begin(); it != m_lsAssocatedPath.end(); it++) 
	{
		sPath = *it;

		if(!(nRet= zoo_wget_children(m_zHandle, sPath.c_str(), CZookeeper::Event,0, &nodeinfo)))
		{
			RefreshAssocSvr(&m_pAssocSvrList[i],nodeinfo);	//refresh  the associated server that get from zookeeper host. 		
		}
		else
		{
			WarnLog("Never get any nodeinfo when pulling %s info,the return code is %d",sPath.c_str(),nRet);
		}
		i++;
	}

	ConnectAssocSvr(m_pAssocSvrList);			//Start to connect to the specific associated server .
	return true;
}

bool CZookeeper::UpdateAssocSvr(string sPath)
{
	//if (!m_zHandle || !m_bConnect) 
	//{	
	//	ConnectZookeeper(); 
	//}	

	
	int16_t nServiceId = INVALID_ASSOCSER_ID;
	AssocSvrList_t* pAssocSvrList = NULL;
	
	CAutoLock autolock(m_pLock);

	nServiceId = GetServiceIdByPath((char*)sPath.c_str());
	if((INVALID_ASSOCSER_ID==nServiceId) || !(pAssocSvrList = GetAssocSvr(nServiceId)))
	{
		ErrLog("Failed to update assocsvr list because the path info %s is invalid!",sPath.c_str());
		return false;
	}

	
	struct String_vector nodeinfo;
	int nRet= zoo_wget_children(m_zHandle, sPath.c_str(), CZookeeper::Event,0, &nodeinfo);
	if (nRet)
	{
		WarnLog("Never get any nodeinfo when Updating %s info,the return code is %d",sPath.c_str(),nRet);
		return false;
	}

	RefreshAssocSvr(pAssocSvrList,nodeinfo);		//Refresh the specified associated server information. 

	ConnectAssocSvr(m_pAssocSvrList);			//Reconnect to the associcated server since the server info is updated.
	return true;
	
}

bool CZookeeper::RefreshAssocSvr(AssocSvrList_t* pAssocSvrList,struct String_vector& nodeInfo) //Restore service node infomation
{
	int i;
	char* str;
	char* ipStr;
	uint16_t nPort;

	if(!pAssocSvrList)
	{
		ErrLog("Find a null invalid assoc svr list  when refreshing assoc svr!");
		return false;
	}
	
	//DbgLog("Associated id = %d , count = %d ",pAssocSvrList->nAssocSvrId,nodeInfo.count);
	ClearAssocSvr(pAssocSvrList);

	for (i=0; i < nodeInfo.count; i++)
	{
		str = nodeInfo.data[i];
		DbgLog("The assoc svr info : %s",str);
		if(str && (str=strchr(str,'_')))
		{
			ipStr=++str;
			
			if(ipStr && (str=strchr(ipStr,':')))
			{
				*str++ = 0;
				sscanf(str, "%4hx", &nPort);    //Format Port str to Por number. 
				RefreshAssocSvrItem(pAssocSvrList,ipStr,nPort);
			}
		}
	}
	
	

	return true;
}

void CZookeeper::RefreshAssocSvrItem(AssocSvrList_t* pAssocSvrList,string sIP,uint16_t nPort)
{
	if(!pAssocSvrList)	
		return;
	serv_info_t* pSvrInfo = new serv_info_t;
	
	pSvrInfo->idle_cnt = 0;
	pSvrInfo->reconnect_cnt = 0;
	pSvrInfo->serv_conn = NULL;
	pSvrInfo->server_ip = sIP;
	pSvrInfo->server_port = nPort;

	string sHost = GetHostString(sIP,nPort);
	pAssocSvrList->mapAssocSvr[sHost]=pSvrInfo;
	pAssocSvrList->bUpdate = true;
}

void CZookeeper::ClearAssocSvr(AssocSvrList_t* pAssocSvrList)
{
	if(!pAssocSvrList || !pAssocSvrList->mapAssocSvr.size())
		return;

	//serv_info_t* pSvrInfo = 0;
	ServerInfoMap_t::iterator it;
	ServerInfoMap_t::iterator nextIt;
	//DbgLog("clear map size : %d",pAssocSvrList->mapAssocSvr.size());
	for(it = pAssocSvrList->mapAssocSvr.begin(); it != pAssocSvrList->mapAssocSvr.end();)
	{
		nextIt = it;
		++nextIt;

		//pSvrInfo = (serv_info_t*)it->second;		
		//DbgLog("svr info ip = %s:%d,serv link = %p",pSvrInfo->server_ip.c_str(),pSvrInfo->server_port,pSvrInfo->serv_conn);
		//if(pSvrInfo)
		//{
		//	delete pSvrInfo;
		//	pSvrInfo = NULL;
		//}
		pAssocSvrList->mapAssocSvr.erase(it->first);
		it = nextIt;
	}
	
	pAssocSvrList->mapAssocSvr.clear();
}

AssocSvrList_t* CZookeeper::GetAssocSvr(int16_t nServiceId) //Get handle of assoc svr , 
{
	AssocSvrList_t* pAssocSvrList = 0;
	unsigned int i = 0;

	for(i = 0; i < m_lsAssocatedPath.size(); i++)
	{	
		if(m_pAssocSvrList[i].nAssocSvrId==nServiceId)
		{
			pAssocSvrList = (AssocSvrList_t*)&m_pAssocSvrList[i];
			break;
		}
	}

	return pAssocSvrList;
}





