/*****************************************************************************************
Filename: clientlinkmgr.h
Author: jack			Version: im-1.0 		Date:2017/06/7
Description:     		客户端链路管理类定义，链路侦听、链路增删查等功能
*****************************************************************************************/

#ifndef __CLIENTLINKMGR_H__
#define __CLIENTLINKMGR_H__
#include <string.h>
#include <tr1/unordered_map>
#include "singleton.h"
#include "configfilereader.h"
#include "serverinfo.h"
#include "clientlink.h"
#include <memory>
#include <uv.h>

using namespace std;
using namespace std::tr1;
using namespace imsvr;
 
typedef std::tr1::unordered_map<string, std::shared_ptr<CClientLink>>  ClientLinkMap_t;
typedef std::tr1::unordered_map<UidCode_t, std::shared_ptr<CClientLink>,hash_func>  SessionClientLinkMap_t;
class CClientLinkMgr : public Singleton<CClientLinkMgr>
{
public:

	CClientLinkMgr();
	~CClientLinkMgr();
	
	static void ClientLinkListener		(				//Listen port network frame callback. 
			void* callback_data, 
			uint8_t msg, 
			uint32_t handle, 
			void* pParam);
    static void ClientLinkNewConnection (
            uv_stream_t* server, 
            int status);
	static void ClientLinkTimeout(					   //Client link network frame time out callback
			void* callback_data, 
			uint8_t msg, 
			uint32_t handle, 
			void* pParam);
	
	bool Initialize(CConfigFileReader* pConfigReader);
	int16_t GetLinkType() {return m_nLinkType;} 
	uint64_t GetFlowCtrlInterval() { return m_nFlowCtrlInterval;}
	uint64_t GetClientLinkTimeout() { return m_nClientLinkTimeout;}
	uint64_t GetAssocLinkTimeout() { return (m_bAssocHeartbeatEnabled) ? m_nAssocLinkTimeout : 0;}
	CLock* GetCliLock() { return m_lock;}
	int GetServiceId() {return m_nServiceId;}
	void GetLocalHost(string& sIP,uint16_t& nPort) {sIP = m_sListenIp; nPort = m_nListenPort;}
	void AddLinkBySessionId(UidCode_t sessionId,std::shared_ptr<CClientLink> pLink);
	void DelLinkBySessionId(UidCode_t sessionId);
	std::shared_ptr<CClientLink> GetLinkBySessionId(UidCode_t sessionId);
	void AddLinkByUserId(string sUserId,std::shared_ptr<CClientLink> pLink);
	void DelLinkByUserId(string sUserId);
	//CLock* Lock() {return m_lock;}
	std::shared_ptr<CClientLink> GetLinkByUserId(string       sUserId);
	void AddLinkByHost(string sIp, uint16_t nPort,std::shared_ptr<CClientLink> pLink);
	void DelLinkByHost(string sIp, uint16_t nPort);
	std::shared_ptr<CClientLink> GetLinkByHost(string sIp, uint16_t nPort);	
protected:
	virtual bool SetListenIPs(void);
	virtual bool SetServiceId(void);
	virtual bool SetLinkTimeout(void);
	virtual bool SetFlowCtrlInterval(void);
	string GetHostString(string sIp, uint16_t nPort);
private:
	CConfigFileReader* m_pConfigReader;
	CLock* m_lock;	
	SessionClientLinkMap_t m_mapSessionLink;
	ClientLinkMap_t m_mapUserLink;
	ClientLinkMap_t m_mapHostLink;
	string m_sListenIp;
	uint16_t m_nListenPort;
	int16_t m_nServiceId;
	int16_t m_nLinkType;
	uint64_t m_nClientLinkTimeout;
	uint8_t m_bAssocHeartbeatEnabled;
	uint64_t m_nAssocLinkTimeout;	//To manage hearbeat communication between associated .
	uint64_t m_nFlowCtrlInterval; 	// to limit packet received , uses for flow ctrolling . 
};

#endif
