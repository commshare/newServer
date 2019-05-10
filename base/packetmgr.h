/*****************************************************************************************
Filename:  packetmgr.h
Author: jack			Version: im-1.0 		Date:2017/6/8
Description:   网络层PDU数据包管理类定义， 包命令包注册、包执行器产生和调用
*****************************************************************************************/

#ifndef __PACKETMGR_H__
#define __PACKETMGR_H__

#include <tr1/unordered_map>

#include "singleton.h"
#include "imconn.h"
#include "impdu.h"
#include "packet.h"
#include "im.pub.pb.h"


using namespace std;
using namespace std::tr1;
using namespace im;



typedef struct _PacketExecutor
{
	CPacket* pPacket;
	CommandProc pProc;
} PacketExecutor_t;

typedef std::tr1::unordered_map<string, PacketExecutor_t*>  PacketExecutorMap_t; // 



class CPacketMgr : public Singleton<CPacketMgr>
{
public:
	CPacketMgr();
	~CPacketMgr();
	
   	virtual void PacketRegist(uint16_t nCmd,int nInstId,CPacket* pPacket,CommandProc pProc);	//Register a packet. 
	virtual PacketExecutor_t* GetPacketExecutor(uint16_t nCmd);			//Get packet executor to process packet.
	virtual bool DispatchPacket(std::shared_ptr<CImPdu> pPdu); //Dispatch a PDU to pduqueue.
	bool Initialize(int nActualServiceInst);
private:
	PacketExecutorMap_t m_mapPacketExecutor;  //Map of packet executor. 
	int					m_nActualServiceInst;	
};


#endif 
