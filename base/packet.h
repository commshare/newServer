/*****************************************************************************************
Filename:  packet.h
Author: jack			Version: im-1.0 		Date:2017/6/6
Description:   网络层PDU数据包基类定义， 包命令字注册、包队列、子线程处理包
*****************************************************************************************/
#ifndef __PACKET_H__
#define __PACKET_H__
#include <list>
#include <queue>
#include <string.h>
#include "util.h"
#include "thread.h"
#include "serverinfo.h"
#include "impdu.h"
#include "im.pub.pb.h"

using namespace std;
using namespace imsvr;

#define PACKET_PROCESS_UNIT		1000
#define PACKET_PROCESS_INTERVAL 5

class CPacket;

typedef std::queue<std::shared_ptr<CImPdu>> PduQueue_t;			   //  		
typedef bool (CPacket::*CommandProc)(std::shared_ptr<CImPdu>);  // command processing function pointer, use to process command. 




class CPacket : public CThread
{
public:
    CPacket();
	virtual ~CPacket();

	virtual void StartThread();					//Start packet processing thread.
	virtual void StopThread();					//Stop packet processing thread.
	virtual void OnThreadRun(void);				// Thread of Process queue message 
    virtual void CmdRegist(uint16_t nCmd, int nInstId,CommandProc pProc);  //Regist business command. 
	virtual void PostPdu(std::shared_ptr<CImPdu> pPdu);	//Post message to queue ,waitting for process. 
	virtual void ProcPdu(std::shared_ptr<CImPdu> pPdu);	// Process message poped from queue. 
protected:	
	int SendPdu(CImPdu* pPdu,int nDirection=0);		 // 0: to client; 1: to server , As default mode to seek the sending link by session id. 
	int SendPdu(string sUserId,CImPdu* pPdu);			//Send pdu by userid. to Seek the sending link by user id. 
	int SendPdu(string sIp,uint16_t nPort,CImPdu* pPdu);//Send pdu by host ip. Seek the sending link by host info.  
	int SendPdu(int16_t nServiceId,CImPdu* pPdu);	 // Send pdu by associated server id, as client mode use it. 
private:
	bool   m_bRunning;
	CLock* m_pLock;					//Thread Lock。
	PduQueue_t m_queuePdu;
};


#endif 
