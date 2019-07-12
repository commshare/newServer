/*****************************************************************************************
Filename: serverlink.cpp
Author: jack			Version: im-1.0 		Date:2017/06/5
Description:     		关联服务端链路头文件,派生于CImConn
*****************************************************************************************/

#ifndef __SERVERLINK_H__
#define __SERVERLINK_H__
//#include "zookeeper.h"
#include "serverinfo.h"


using namespace imsvr;


class CServerLink : public CImConn
{
public:
	CServerLink();
	virtual ~CServerLink();

	bool IsConnect() { return m_bConnect; }

	virtual void Connect(serv_info_t* pSvrInfo);
	virtual void Close();

	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);
	virtual void HandlePdu(std::shared_ptr<CImPdu> pPdu);
	void GetAssocSvrHost(string& sIp, uint16_t& nPort);
	UidCode_t GetSessionId(void) { return m_SessionId;}
	bool GetRegistStatus(void) { return m_bRegist; }
	bool GetConnect(void) { return m_bConnect; }
	void SetAssocSvrInfo(serv_info_t* pSvrInfo) {m_pAccoSvrInfo = pSvrInfo;}
protected:	
	void HeartbeatSend(void);
	void AssocSvrRegist(void);
	void OnAssocSvrRegistAck(std::shared_ptr<CImPdu> pPdu);
	void OnDispatchPacket(std::shared_ptr<CImPdu> pPdu);
	
private:
	CLock* m_pLock;					//Thread Lock。
	UidCode_t	m_SessionId;		//The session value allocated by remoted connected server.
	serv_info_t* m_pAccoSvrInfo; //Externally imported arguments for the asssociated server. 
								 //Including IP address information etc.
	bool m_bConnect;			 //  built a connectin with assoc svr.
	bool m_bRegist;				 //  Regist with assoc svr;
};

#endif
