/*****************************************************************************************
Filename:  packet.cpp
Author: jack			Version: im-1.0 		Date:2017/6/6
Description:   网络层PDU数据包基类实现， 包命令字注册、包队列、子线程处理包
*****************************************************************************************/

#include <unistd.h>
#include "packetmgr.h"
#include "clientlinkmgr.h"
#include "zookeeper.h"

CPacket::CPacket() 
	:m_bRunning(false)
{
	m_pLock = new CLock();
}

CPacket::~CPacket()
{

	StopThread();

	
	PduQueue_t empty;
	if(m_pLock != NULL)  	 // thread lock definition
	{
		delete m_pLock;
		m_pLock = NULL;
	}	

	swap(m_queuePdu,empty); //Release  pduqueue
}


void CPacket::StartThread()
{

	pthread_attr_t threadAttr;
	pthread_attr_init(&threadAttr);
	pthread_attr_setschedpolicy(&threadAttr, SCHED_RR); //Create packet thread in SCHED_RR mode


	if(int err = pthread_create(&m_thread_id, &threadAttr, StartRoutine, this))
	{
		ErrLog("Failed to create thread with err : %s",strerror(err));
	}

	
	//if(!pthread_create(&m_thread_id, &threadAttr, StartRoutine, this))	
	//{
	//	m_bRunning = true;
	//}
}


void CPacket::StopThread()
{
	if (m_bRunning)
	{
		m_bRunning = false;
		pthread_join(m_thread_id, NULL);
	}
}

void CPacket::OnThreadRun()
{
	std::shared_ptr<CImPdu> pPdu = 0;
//	static int nTimes = 0;
	int i = 0;

	
	//DbgLog("OnThread times : %d,%d",nTimes++,m_bRunning);
	m_bRunning = true;
	while(m_bRunning)
	{
		m_pLock->lock();
		
		//Process pdu queue if queue size > 0 ,It will sleep 
		//10ms per 2000 packet to release time slice for 
		//other packet queue, 
		if(m_queuePdu.size()&&(i <= PACKET_PROCESS_UNIT))
		{
			pPdu = m_queuePdu.front();
			m_queuePdu.pop();
			m_pLock->unlock();
			
			ProcPdu(pPdu);							// Invoke ProcPdu to process pdu;	
			i++;			
		}
		else
		{
			//if(m_queuePdu.size())
			//	printf("********* remain packet number = %d \n **************",(int)m_queuePdu.size());
			//DbgLog("Waitting for proc thread sleep = %d",i);
			m_pLock->unlock();
			usleep(PACKET_PROCESS_INTERVAL*1000);
			i = 0;
		}
	}
			
}


void CPacket::CmdRegist(uint16_t nCmd, int nInstId,CommandProc pProc)
{
	CPacketMgr::GetInstance()->PacketRegist(nCmd,nInstId,this,pProc);
}

void CPacket::PostPdu(std::shared_ptr<CImPdu> pPdu)
{
	CAutoLock autolock(m_pLock);

	m_queuePdu.push(pPdu);						 // Insert pdu to pdu queue.
}

void CPacket::ProcPdu(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
		return;
	uint16_t nCmd;
	PacketExecutor_t* pExecutor = 0;
	CommandProc pProc;
	
	nCmd = pPdu->GetCommandId();
	pExecutor = CPacketMgr::GetInstance()->GetPacketExecutor(nCmd);
	//DbgLog("Proc pdu,cmd = 0x%x",nCmd);
	if(pExecutor)
	{
		pProc = pExecutor->pProc;
		(this->*pProc)(pPdu);		//Invork the callback function to process the packet. 
		pPdu.reset();
	}
}
//CAutoLock autolock(pLinkMgrLock);
//autolock below for safety write a tcp channel!!!
int CPacket::SendPdu(CImPdu* pPdu,int nDirection)
{
	if(!pPdu) return -1;

	CLock*  pLinkMgrLock = NULL;
	UidCode_t sessionId;
	void* pLink;
	int nLen = -1;


	sessionId = pPdu->GetSessionId();
	pLink = (!nDirection) ? (void*)CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId) :
			(void*)CZookeeper::GetInstance()->GetLinkBySessionId(sessionId);
	if(!pLink)
	{
		ErrLog("Err sending for the session link is not exist!");
		return -1;
	}

	if(!nDirection)
	{
		pLinkMgrLock = CClientLinkMgr::GetInstance()->GetCliLock();
		CAutoLock autolock(pLinkMgrLock);
		nLen = ((CClientLink*)pLink)->SendPdu(pPdu);
		((CClientLink*)pLink)->ReleaseRef();
	}
	else
	{
		pLinkMgrLock = CZookeeper::GetInstance()->GetSvrLock();
		CAutoLock autolock(pLinkMgrLock);
		nLen = ((CServerLink*)pLink)->SendPdu(pPdu);
		((CServerLink*)pLink)->ReleaseRef();
	}

	return nLen;
}
int CPacket::SendPdu(string sUserId,CImPdu* pPdu)
{
	if(!pPdu) return -1;

	CLock*  pLinkMgrLock = NULL;

	int nLen = -1;
	CClientLink* pLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId);
	if(!pLink)
	{
		ErrLog("Err sending for the user %s link is not exist!",sUserId.c_str());
		return -1;
	}

	pLinkMgrLock = CClientLinkMgr::GetInstance()->GetCliLock();
	CAutoLock autolock(pLinkMgrLock);
	
	pPdu->SetSessionId(pLink->GetSessionId());
	nLen = pLink->SendPdu(pPdu);
	pLink->ReleaseRef();
	
	return nLen;
}

int CPacket::SendPdu(string sIp,uint16_t nPort,CImPdu* pPdu)
{
	if(!pPdu) return -1;

	CLock*  pLinkMgrLock = NULL;
	int nLen;

	CClientLink* pLink = CClientLinkMgr::GetInstance()->GetLinkByHost(sIp,nPort);
	if(!pLink)
	{
		ErrLog("Err sending for the link  %s : %d is not exist!",sIp.c_str(),nPort);
		return -1;
	}

	pLinkMgrLock = CClientLinkMgr::GetInstance()->GetCliLock();
	CAutoLock autolock(pLinkMgrLock);
	
	nLen = pLink->SendPdu(pPdu);
	pLink->ReleaseRef();
	return nLen;
}

int CPacket::SendPdu(int16_t nServiceId,CImPdu* pPdu)
{
	if(!pPdu) return -1;
	
	CLock*  pLinkMgrLock = NULL;
	CServerLink* pLink = 0;
	UidCode_t sessionId =  pPdu->GetSessionId();	

	pLinkMgrLock = CZookeeper::GetInstance()->GetSvrLock();
	pLink = CZookeeper::GetInstance()->GetAssocSvrLink(nServiceId,sessionId);
	if(!pLink || !pLink->GetRegistStatus())
	{
		if(pLink)
		{
			CAutoLock autolock(pLinkMgrLock);
			pLink->ReleaseRef();
		}
		ErrLog("Err sending for the link is not exist or the link is not regist!");
		return -1;
	}

	int nLen = -1;
#if 0						// open the statement only for debuging . 
	string sIp;
	uint16_t nPort;
	pLink->GetAssocSvrHost(sIp, nPort);
	DbgLog("Sending message to assoc svr %s:%d ...",sIp.c_str(),nPort);
#endif	
	CAutoLock autolock(pLinkMgrLock);

	pPdu->SetSessionId(pLink->GetSessionId());  
	nLen = pLink->SendPdu(pPdu);
	pLink->ReleaseRef();
	
	return nLen;
}

//USE_CONSISTENT_LINK

