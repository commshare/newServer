/*****************************************************************************************
Filename:  packetmgr.cpp
Author: jack			Version: im-1.0 		Date:2017/6/8
Description:   网络层PDU数据包管理类实现， 包命令包注册、包执行器产生和调用
*****************************************************************************************/

#include "packetmgr.h"
#include "clientlinkmgr.h"
#include <sys/types.h>
#include <sys/socket.h>

CPacketMgr::CPacketMgr()
	: m_nActualServiceInst(0)
{
	m_mapPacketExecutor.clear();
}

CPacketMgr::~CPacketMgr()
{
	PacketExecutorMap_t::iterator it;
	PacketExecutor_t* pExecutor = NULL;

	//Release resource of packet executor in map
	for (it = m_mapPacketExecutor.begin(); it != m_mapPacketExecutor.end(); it++) 
	{
		pExecutor = (PacketExecutor_t*)it->second;
		if(pExecutor)
		{
			delete pExecutor;
		}
	}
	
	m_mapPacketExecutor.clear();	
			
}

bool CPacketMgr::Initialize(int nActualServiceInst)
{
	m_nActualServiceInst = nActualServiceInst;

	return true;
}

void CPacketMgr::PacketRegist(uint16_t nCmd,int nInstId,CPacket* pPacket,CommandProc pProc)
{
	PacketExecutorMap_t::iterator it;
	PacketExecutor_t* pExecutor = 0;
	string sExecuteId;
	char tbuf[64] = {0};
	
	sprintf(tbuf,"%d:%d",nCmd,nInstId);
	sExecuteId = tbuf;
	
	pExecutor = new PacketExecutor_t;

	pExecutor->pPacket = pPacket;
	pExecutor->pProc = pProc;

	it = m_mapPacketExecutor.find(sExecuteId);
	 

	if(it == m_mapPacketExecutor.end())
	{
		m_mapPacketExecutor[sExecuteId] = pExecutor;
	}
	else
	{
		ErrLog("The packet cmd %x04 has already been regist in instance service %d ! \n",nCmd,nInstId);
	}
}


PacketExecutor_t* CPacketMgr::GetPacketExecutor(uint16_t nCmd)
{
	if(!m_nActualServiceInst)
		return NULL;

	PacketExecutor_t* pExecutor = 0;	
	int nInstId = rand() % m_nActualServiceInst;
	//int nInstId = BKDRHash(sSessionId) % m_nActualServiceInst;
	char buf[64] = {0};
	
	sprintf(buf,"%d:%d",nCmd,nInstId);
	string sExecuteId = buf;
	PacketExecutorMap_t::iterator it = m_mapPacketExecutor.find(sExecuteId);  // Find a packet executor from map	
	
	if(it != m_mapPacketExecutor.end())
	{
		pExecutor = it->second;
	}
	
	return pExecutor;
}
bool CPacketMgr::DispatchPacket(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu) 
	{
		ErrLog("Can't dispatch a null invalid pdu!");
		return false;
	}


	uint16_t nCmdId = pPdu->GetCommandId();

	PacketExecutor_t* pExecutor = GetPacketExecutor(nCmdId);			//Get a packet executor
	if(!pExecutor)
	{
		ErrLog("Can't find the corresponding executor ,cmd is 0x%x",nCmdId);
		pPdu.reset();
		return false;
	}

	CPacket* pPacket = pExecutor->pPacket;
	if(!pPacket)
	{
		ErrLog("Failed to post the packet 0x%x because Can't find a valid registed packet",nCmdId);
		pPdu.reset();
		return false;
	}
	
	pPacket->PostPdu(pPdu);							//Post a pdu into a packet queue.

	return true;
}


