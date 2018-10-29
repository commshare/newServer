/*****************************************************************************************
Filename: serverlink.cpp
Author: jack			Version: im-1.0 		Date:2017/05/23
Description:     		关联服务端链路实现代码，含向关联服务端发起连接、关闭，数据收发。
*****************************************************************************************/
#include "zookeeper.h"
#include "serverlink.h"
#include "packetmgr.h"


extern ConnMap_t g_ServerPhysicalLinkMap;  //Physical linker map for teamtalk network frame work

CServerLink::CServerLink() 
	: m_pAccoSvrInfo(NULL), m_bConnect(false),m_bRegist(false)
{
	memset(&m_SessionId, 0, UID_SIZE);
	m_pLock = new CLock();
	SetLock(m_pLock);
}
CServerLink::~CServerLink()
{
	if(m_pLock != NULL)  	 // thread lock definition
	{
		delete m_pLock;
		m_pLock = NULL;
	}		
}

void CServerLink::Connect(serv_info_t* pSvrInfo)
{
	if(NULL==pSvrInfo)
	{
		ErrLog("Error of connection, the associated server is null!");
		return;
	}

	DbgLog("Connecting to associated server %s:%d ...", pSvrInfo->server_ip.c_str(), pSvrInfo->server_port);
	
	m_pAccoSvrInfo = pSvrInfo;
	m_handle = netlib_connect(							//Connect to associated server now. 
				m_pAccoSvrInfo->server_ip.c_str(), 
				m_pAccoSvrInfo->server_port, 
				imconn_callback, 
				(void*)&g_ServerPhysicalLinkMap);

	if (m_handle != NETLIB_INVALID_HANDLE) 
	{					
		g_ServerPhysicalLinkMap.insert(make_pair(m_handle, this)); // Insert current link to physical link map. 	
	}
}

void CServerLink::Close()
{
	//DbgLog("Close current link...");
	//if(!m_pLinkMgrLock)
	//{
	//	ErrLog("resource exception when close the server link!");
	//	return;
	//}	
	
	CAutoLock autolock(CZookeeper::GetInstance()->GetSvrLock());
	
	serv_reset<CServerLink>(m_pAccoSvrInfo);			//reset the specific associated servert state. 
	
	if (m_handle != NETLIB_INVALID_HANDLE) 
	{
		netlib_close(m_handle);
		g_ServerPhysicalLinkMap.erase(m_handle);
	}
	CZookeeper::GetInstance()->DelLinkBySessionId(m_SessionId);
	m_bConnect = false;
	ReleaseRef();
	
}

void CServerLink::GetAssocSvrHost(string& sIp, uint16_t& nPort)
{ 
	if(m_pAccoSvrInfo) 
	{ 
		sIp = m_pAccoSvrInfo->server_ip; 
		nPort = m_pAccoSvrInfo->server_port;
	}
}

void CServerLink::OnConfirm()							//Confirm to connect associated server success. 	
{
	DbgLog("connect to server success,Host is %s:%d",m_pAccoSvrInfo->server_ip.c_str(),m_pAccoSvrInfo->server_port);
	m_bConnect = true;
	m_pAccoSvrInfo->reconnect_cnt = MIN_RECONNECT_CNT / 2;
	AssocSvrRegist(); 				//Regist to assocsvr . 
}

void CServerLink::OnClose()
{
	Close();
	DbgLog("this link is closed by remoted host");	
}

void CServerLink::OnTimer(uint64_t curr_tick)
{
	uint64_t nAssocHeartbeatInter = CZookeeper::GetInstance()->GetAssocHeartbeat();
	if(!nAssocHeartbeatInter)   //return 0 , indicated no heartbeat between assoc server . 
		return;

	nAssocHeartbeatInter+=m_last_send_tick;

	if(curr_tick > nAssocHeartbeatInter)
	{
		HeartbeatSend();
	}
}

void CServerLink::HeartbeatSend(void) //Send heartbeat between associated server . 
{
	CImPdu pdu;
	pdu.SetPBWithoutMsg(SYSTEM_HEARTBEAT,m_SessionId);
	
	DbgLog("Sending heartbeat...");
	SendPdu(&pdu);
}

void CServerLink::AssocSvrRegist(void)
{
	string 	 sIp;
	uint16_t nPort;
	CImPdu pdu;
	SYSAssocSvrRegist regist;

	CZookeeper::GetInstance()->GetLocalHost(sIp,nPort);

	DbgLog("AssocSvrRegist,Host is %s:%d",sIp.c_str(),nPort);

	regist.set_ip(sIp);
	regist.set_port(nPort);

	memset(&m_SessionId, 0, UID_SIZE);
	pdu.SetPBMsg(&regist);
	pdu.SetCommandId(SYSTEM_ASSOCSVR_REGIST);
	pdu.SetSessionId(m_SessionId);
	
	SendPdu(&pdu);
	DbgLog("send assoc server msg ,Host is %s:%d",m_pAccoSvrInfo->server_ip.c_str(),m_pAccoSvrInfo->server_port);
}

void CServerLink::OnAssocSvrRegistAck(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu) return;

	SYSAssocSvrRegistAck registAck;
	uchar_t* pContent = pPdu->GetBodyData();
	
	if(!pContent || !registAck.ParseFromArray(pContent,pPdu->GetBodyLength()) 
		|| (registAck.nerr()==ERR_SYS_REGIST))
	{
		m_bRegist = false;
		ErrLog("Assoc svr regist ack error!");
		pPdu.reset();
		return;
	}
	

	//m_sSessionId = pPdu->GetSessionId();
	m_SessionId = pPdu->GetSessionId();
	CZookeeper::GetInstance()->AddLinkBySessionId(m_SessionId,this);
	m_bRegist = true;
	pPdu.reset();
	DbgLog("regist to assoc svr %s:%d successfully !",m_pAccoSvrInfo->server_ip.c_str(),m_pAccoSvrInfo->server_port);
}

void CServerLink::OnDispatchPacket(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
		return;

	CPacketMgr::GetInstance()->DispatchPacket(pPdu);
}

void CServerLink::HandlePdu(std::shared_ptr<CImPdu> pPdu)     	  
{	
	if(!pPdu)
	{
		ErrLog("recv a null invalid pdu when processing assoc svr packet!");
		return;
	}
	//string sSessionId;
	uint16_t nCmdId = pPdu->GetCommandId();

	//UidCode_t sessionId = pPdu->GetSessionId();
	//toHexLower(sSessionId, (const void*)&sessionId,UID_SIZE);
	
	if(!m_bRegist && (nCmdId!=SYSTEM_ASSOCSVR_REGIST_ACK))
	{
		ErrLog("Reject any packet from assoc svr since no registration has been successful , and regist again ...");
		AssocSvrRegist();
		pPdu.reset();
		return;

	}

	if(nCmdId==SYSTEM_ASSOCSVR_REGIST_ACK)
	{
		OnAssocSvrRegistAck(pPdu);
	}
	else
	{
		OnDispatchPacket(pPdu);		// Invoke Dispatch to post pdu to pdu queue. 			
	}
}




