/*****************************************************************************************
Filename: clientlink.h
Author: jack			Version: im-1.0 		Date:2017/05/20
Description: 	底层网络框架客户端的链路定义。
*****************************************************************************************/


#ifndef __CLIENTLINK_H__
#define __CLIENTLINK_H__
#include "util.h"
#include "imconn.h"
#include "packetmgr.h"

#define MAX_UNREGIST_PACKET 2
enum 
{
	MOBILE_LOGIN_LINKER = 1,
	ASSOC_REGIST_LINKER
};


class CClientLink : public CImConn
{
public:
	CClientLink();
	~CClientLink();


	virtual void Close();

	void OnConnect(net_handle_t handle, int nLinkType,uint64_t nFlowCtrlInterval);
    void OnConnect(uv_stream_t* nStream, int nLinkType,uint64_t nFlowCtrlInterval);
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);
	virtual void OnClientTimeout();
	virtual void HandlePdu(std::shared_ptr<CImPdu> pPdu);	
	virtual void UserArrived(string sUserId);
	virtual void UserLeft(void);
	UidCode_t GetSessionId() { return m_SessionId;}
	void CloseLink(bool bUserLink = true);
	void SetLinkLock(CLock* pLock) { m_pLinkMgrLock = pLock; }
	void ResetLogin() { m_bLogining = false; }
    string GetUserId(){return m_sUserId;}
protected:
	bool OnLoginPretreat(std::shared_ptr<CImPdu> pPdu);
	bool UnFlowCtrl(uint16_t nCmdId);
	void OnAssocSvrRegist(std::shared_ptr<CImPdu> pPdu);
	void OnHeartbeat(std::shared_ptr<CImPdu> pPdu);
    void OnPing(std::shared_ptr<CImPdu> pPdu);
	void OnDispatchPacket(std::shared_ptr<CImPdu> pPdu);
	void AssocSvrRegistAck(UidCode_t sessionId,ErrCode bCode);

		
	//void RleaseClientLink(void);
	//void SetUserId(string sUserId) { m_sUserId = sUserId;}
private:
	CLock* m_pLock;					//Thread Lock for net framework 
	CLock* m_pLinkMgrLock;				// socket link lock.
	UidCode_t m_SessionId;			// identify link with session id
	string m_sUserId;			// identify link with user id	
	string m_sRegistIp;			// ip address is registed from assoc client	
	uint16_t m_nRegistPort;		// ip port is registed from assoc client
	int16_t m_nLinkType;		// to differentiate a link ,  app client link or associate client  link. 
	uint64_t m_nFlowCtrlInterval; //to realize packet flow controling,the value is passed from clientmgr class. 
	uint64_t m_nLastDispatchTime; //Last time tick to dispatch packet.
	uint16_t m_nLastPacketCmd;	  //Last Packet command. 
	uint16_t m_nUnregistPacket;	  // Summary unregist packet;
	bool m_bRegist;			    // App or associcated client login or regist. 
	bool m_bLogining;			//Flag for distinguish busy logining or not.  true: busy loginging . 
	bool m_bClosing;			//Flag for distinguish closing or not. true: closing 
};





#endif

