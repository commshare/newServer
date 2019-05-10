 /*****************************************************************************************
Filename: clientlink.cpp
Author: jack			Version: im-1.0 		Date:2017/05/20
Description: 	底层网络框架客户端的链路实现，含业务协议包的分发，对客户端链路超时监测。
*****************************************************************************************/

#include "clientlinkmgr.h"
#include "packetmgr.h"

static const uint16_t ArrayOfUnFlowCtrl[] =
{
	MES_CHAT_READ,
	MES_CHAT_READ_DELIVER_ACK,
	MES_DECBLACKLIST,
	INVALID_CMDID
};

extern ConnMap_t g_ClientPhysicalLinkMap;   //import from clientlinkmgr.cpp, to manage physical link. 

CClientLink::CClientLink()
	: m_pLinkMgrLock(NULL),m_sUserId(""),m_sRegistIp(""),m_nRegistPort(0),m_nFlowCtrlInterval(0),
	m_nLastDispatchTime(0),m_nLastPacketCmd(INVALID_CMDID),m_nUnregistPacket(0),m_bRegist(false),
	m_bLogining(false),m_bClosing(false)
{

	memset(&m_SessionId, 0, UID_SIZE);

	m_pLock = new CLock();
	SetLock(m_pLock);
}

CClientLink::~CClientLink()
{
	if(m_pLock != NULL)  	 // thread lock definition
	{
		delete m_pLock;
		m_pLock = NULL;
	}	
	m_bRegist = false;
	m_bLogining = false;
	m_bClosing = false;
}

void CClientLink::Close()
{
	if(m_bClosing)
		return;
	//if(!m_pLinkMgrLock)
	//{
	//	ErrLog("resource exception when close the client link!");
	//	return;
	//}	
	
	//CAutoLock autolock(m_pLinkMgrLock);

	m_bClosing =  true;
			
	if(!m_bRegist || m_nLinkType==ASSOC_REGIST_LINKER)
	{	
		DbgLog("Close the unregisted client link! userid %s regist %d", GetUserId().c_str(), m_bRegist);
		CloseLink();
		return;
	}
	
	UserLeft();
}

void CClientLink::CloseLink(bool bUserLink)
{

	if (m_handle != NETLIB_INVALID_HANDLE) 
	{
		netlib_close(m_handle);
		g_ClientPhysicalLinkMap.erase(m_handle);
	}

	CClientLinkMgr::GetInstance()->DelLinkByHost(m_sRegistIp,m_nRegistPort);
	CClientLinkMgr::GetInstance()->DelLinkBySessionId(m_SessionId); //Delete the link from map since the link is closed . 

	CClientLinkMgr::GetInstance()->delSessionByHost(m_peer_ip, m_peer_port);

	if(bUserLink&&m_sUserId!="")
	{
		CClientLinkMgr::GetInstance()->DelLinkByUserId(m_sUserId);
	}
	
	m_bRegist = false;
	m_bLogining = false;
	
	m_sUserId = "";		// clear the binding user id. 
	ReleaseRef();
}

//void CClientLink::OnPingAck(UidCode_t sessionId,const string &msgId)
//{
//    SystemPingAck pingAck;
//    CImPdu 		pingAckPdu;

//    pingAck.set_msgid(msgId);
//    pingAckPdu.SetPBMsg(&pingAck);
//    pingAckPdu.SetCommandId(SYSTEM_PING_ACK);
//    pingAckPdu.SetSessionId(sessionId);
	
//    SendPdu(&pingAckPdu);
//}

void CClientLink::AssocSvrRegistAck(UidCode_t sessionId, ErrCode bCode)
{
    SYSAssocSvrRegistAck registAck;
    CImPdu 		registAckPdu;

    registAck.set_nerr(bCode);
    registAckPdu.SetPBMsg(&registAck);
    registAckPdu.SetCommandId(SYSTEM_ASSOCSVR_REGIST_ACK);
    registAckPdu.SetSessionId(sessionId);

    SendPdu(&registAckPdu);
}

void CClientLink::UserArrived(string sUserId)
{
	m_sUserId = sUserId;
	CClientLinkMgr::GetInstance()->AddLinkByUserId(m_sUserId,this);

	DbgLog("user %s has arrived with link %p",m_sUserId.c_str(),this);
	m_bRegist = true;
	m_bLogining = false;
}


void CClientLink::UserLeft(void)
{
	std::shared_ptr<CImPdu> pPdu(new CImPdu()); 
	
	pPdu->SetPBWithoutMsg(SYSTEM_TIMEOUT_NOTIFICATION,m_SessionId);
	CPacketMgr::GetInstance()->DispatchPacket(pPdu);
	m_bLogining = false;
}

void CClientLink::OnConnect(net_handle_t handle, int nLinkType,uint64_t nFlowCtrlInterval)
{
	m_handle = handle;
	m_nLinkType = nLinkType;
	m_nFlowCtrlInterval = nFlowCtrlInterval;
	m_pLinkMgrLock = CClientLinkMgr::GetInstance()->GetCliLock();
	ConnMap_t* pLinkMap = &g_ClientPhysicalLinkMap;

	pLinkMap->insert(make_pair(handle, this));		// Insert current link to physical link map . 
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)pLinkMap);

	
	netlib_get_remotehost(m_handle,m_peer_ip,m_peer_port);
	//	DbgLog("Client connecting %s:%d,physical link number=%d",m_peer_ip.c_str(),
	//		m_peer_port,pLinkMap->size());
}

void CClientLink::OnClose()
{
	DbgLog("remote peer closing : handle=%d,ip=%s,port=%d, userid=%s", m_handle,m_peer_ip.c_str(),m_peer_port, m_sUserId.c_str());

	if(((m_nLinkType==MOBILE_LOGIN_LINKER)&&!m_bLogining)||(m_nLinkType==ASSOC_REGIST_LINKER))	
	 	Close();
}

void CClientLink::OnClientTimeout()
{
	DbgLog("user %s close the remote host %s:%d for overtime connection!",m_sUserId.c_str(),m_peer_ip.c_str(),m_peer_port);
	Close();
}

void CClientLink::OnTimer(uint64_t curr_tick)
{
	if(m_nLinkType==ASSOC_REGIST_LINKER)//ignore timer in assoc svr
		return;

	uint64_t nClientTimeout = CClientLinkMgr::GetInstance()->GetClientLinkTimeout()+m_last_recv_tick;

	if(curr_tick>nClientTimeout)
	{
		OnClientTimeout();
	}
}

void CClientLink::OnAssocSvrRegist(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu ||  (m_nLinkType==MOBILE_LOGIN_LINKER)) 
	{
		if(pPdu)
			pPdu.reset();
		ErrLog("assoc server impdu erro %d", m_nLinkType);
		return;
	}


	SYSAssocSvrRegist regist;
	uchar_t* pContent = pPdu->GetBodyData();
	
	if(!pContent || !regist.ParseFromArray(pContent,pPdu->GetBodyLength()))
	{
		AssocSvrRegistAck(m_SessionId,ERR_SYS_REGIST);
		ErrLog("Assoc svr regist error!");
		pPdu.reset();
		
		return;
	}

	m_sRegistIp = regist.ip();
	m_nRegistPort = regist.port();

	DbgLog("OnAssocSvrRegist,Host is %s:%d",m_sRegistIp.c_str(),m_nRegistPort);
	
	CClientLinkMgr::GetInstance()->AddLinkByHost(m_sRegistIp,m_nRegistPort,this);
	
	GenerateUId(m_SessionId);
	CClientLinkMgr::GetInstance()->AddLinkBySessionId(m_SessionId,this);

	AssocSvrRegistAck(m_SessionId,NON_ERR);
	m_bRegist = true;

	
	/*
	DbgLog("Success to regist,regist from = %s,%d,session=%x%x%x%x%x%x%x%x%x%x%x%x",
	m_sRegistIp.c_str(),m_nRegistPort,m_SessionId.Uid_Item.code[0],m_SessionId.Uid_Item.code[1],
	m_SessionId.Uid_Item.code[2],m_SessionId.Uid_Item.code[3],m_SessionId.Uid_Item.code[4],
	m_SessionId.Uid_Item.code[5],m_SessionId.Uid_Item.code[6],m_SessionId.Uid_Item.code[7],
	m_SessionId.Uid_Item.code[8],m_SessionId.Uid_Item.code[9],m_SessionId.Uid_Item.code[10],
	m_SessionId.Uid_Item.code[11]);
	*/
	pPdu.reset();
	InfoLog("Success to regist from = %s,%d",m_sRegistIp.c_str(),m_nRegistPort);
}

void CClientLink::OnHeartbeat(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu || (m_nLinkType==ASSOC_REGIST_LINKER)) //ignore heartbeat in assoc  
	{
		if(pPdu)
			pPdu.reset();
		
		return; 
	}

	CImPdu pdu;
	
	//DbgLog("heartbeat is sent by user %s,the user host %s : %d",
	//	m_sUserId.c_str(),m_peer_ip.c_str(),m_peer_port);
	
	pdu.SetPBWithoutMsg(SYSTEM_HEARTBEAT_ACK,m_SessionId);

	SendPdu(&pdu);
	pPdu.reset();
}

bool CClientLink::OnLoginPretreat(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu || m_bRegist || m_bLogining)
	{
		WarnLog("Reject the relogin packet because the linker has been regist or logining busy!");
		if(pPdu)
			pPdu.reset();
		return false;
	}

	// 删除session管理的链路
	if(m_peer_ip.empty())
		netlib_get_remotehost(m_handle,m_peer_ip,m_peer_port);
	UidCode_t tmpSessionId = CClientLinkMgr::GetInstance()->getSessionByHost(m_peer_ip, m_peer_port);
	CClientLinkMgr::GetInstance()->DelLinkBySessionId(tmpSessionId);
	GenerateUId(m_SessionId);					//Builds a new session link at the first receiving login packet. 
	CClientLinkMgr::GetInstance()->AddLinkBySessionId(m_SessionId,this);

	CClientLinkMgr::GetInstance()->addSessionByHost(m_peer_ip, m_peer_port, m_SessionId);
	
	pPdu->SetSessionId(m_SessionId);
	m_nUnregistPacket = 0; 						// Clear number of unregist packet. 
	m_bLogining = true;

	#if 0
	DbgLog("Client loggined from %s:%p,session=%x%x%x%x%x%x%x%x%x%x%x%x",m_peer_ip.c_str(),m_peer_port,
		m_SessionId.Uid_Item.code[0],m_SessionId.Uid_Item.code[1],m_SessionId.Uid_Item.code[2],
		m_SessionId.Uid_Item.code[3],m_SessionId.Uid_Item.code[4],m_SessionId.Uid_Item.code[5],
		m_SessionId.Uid_Item.code[6],m_SessionId.Uid_Item.code[7],m_SessionId.Uid_Item.code[8],
		m_SessionId.Uid_Item.code[9],m_SessionId.Uid_Item.code[10],m_SessionId.Uid_Item.code[11]);
	#endif
	return true;
}

void CClientLink::OnPing(std::shared_ptr<CImPdu> pPdu)
{
    if(!pPdu || (m_nLinkType==ASSOC_REGIST_LINKER)) //ignore ping in assoc
    {
        if(pPdu)
            pPdu.reset();

        return;
    }

	pPdu->SetCommandId(SYSTEM_PING_ACK);
	SendPdu(pPdu);

//    SystemPing ping;
//    uchar_t* pContent = pPdu->GetBodyData();

//    if(!pContent || !ping.ParseFromArray(pContent,pPdu->GetBodyLength()))
//    {
    //	AssocSvrRegistAck(m_SessionId,ERR_SYS_REGIST);
//        ErrLog("phrase ping error!");
//        pPdu.reset();

//        return;
//    }

//    OnPingAck(m_SessionId, ping.msgid());

}

void CClientLink::OnDispatchPacket(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
		return;

	uint16_t nCurrentCmd =  pPdu->GetCommandId();
	#if 0
	uint64_t nCurrentTick = get_tick_count();
	uint64_t nDispatchTime = m_nLastDispatchTime + m_nFlowCtrlInterval;

	if((m_nLinkType==MOBILE_LOGIN_LINKER) && ((nCurrentCmd == m_nLastPacketCmd)&&
		(nCurrentTick < nDispatchTime)&&!UnFlowCtrl(nCurrentCmd))&&
		(m_nLastDispatchTime > 0 ))
	{
		WarnLog("The packet 0x%x from host %s:%d can't dispatch because of flow controlling",
			nCurrentCmd,m_peer_ip.c_str(),m_peer_port);

		pPdu.reset();
		
		return;
	}
	#endif
	

	if(true == CPacketMgr::GetInstance()->DispatchPacket(pPdu))
	{
		m_nLastDispatchTime = get_tick_count();
		m_nLastPacketCmd = nCurrentCmd;		
	}	
}

bool CClientLink::UnFlowCtrl(uint16_t nCmdId)
{
	bool bRet = false;
	int i= 0;		
	for(i = 0;;i++)
	{	
		if(ArrayOfUnFlowCtrl[i]==INVALID_CMDID)
			break;
		
		if(nCmdId==ArrayOfUnFlowCtrl[i])
		{
			bRet = true;
			break;
		}
	}

	return bRet;
}

void CClientLink::HandlePdu(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu || m_bClosing)
	{
		WarnLog("recv a invalid pdu when processing client packet with link status %d !",m_bClosing);
		if(pPdu)		
			pPdu.reset();
		
		return;
	}
	UidCode_t sessionId;
	string sSessionId = "";


	uint16_t nCmdId = pPdu->GetCommandId();

	if(m_bRegist)
	{
		sessionId = pPdu->GetSessionId();
		toHexLower(sSessionId, (const void*)&sessionId,UID_SIZE);
	}	

	CClientLink* pCurLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
	//
#if 0
	if((m_bRegist && !IsValidUid(sSessionId.c_str(),UID_SIZE*2)) || (!m_bRegist&&(((m_nLinkType==ASSOC_REGIST_LINKER)&&
	 (nCmdId!=SYSTEM_ASSOCSVR_REGIST)) || ((m_nLinkType==MOBILE_LOGIN_LINKER)&&(nCmdId!=CM_LOGIN)&&(!pCurLink || pCurLink->GetUserId()=="")))))  // Reject to post any packet if no logining . 
	{
		ErrLog("Reject any packet from client %s: %d since no any logining successfully or using invalid session!",m_peer_ip.c_str(),m_peer_port);
		if(m_nLinkType!=ASSOC_REGIST_LINKER)
			Close();
		
		pPdu.reset();
		if(pCurLink){ pCurLink->ReleaseRef();}
		return;
	}
#endif 
	if(pCurLink){ pCurLink->ReleaseRef();}
	
	if(nCmdId==SYSTEM_ASSOCSVR_REGIST) 
	{
		OnAssocSvrRegist(pPdu);
 	}
    if(nCmdId==SYSTEM_PING)
    {
        OnPing(pPdu);
    }
	else if(nCmdId==SYSTEM_HEARTBEAT)
	{
		OnHeartbeat(pPdu);
	}
	else
	{
		if(nCmdId==CM_LOGIN || nCmdId == PC_LOGIN)
		{
			if(!OnLoginPretreat(pPdu)) //allway dispatch ?
				return;
		}
		OnDispatchPacket(pPdu);
	}
}


