/*****************************************************************************************
Filename: serverlinkmgr.h
Author: jack			Version: im-1.0 		Date:2017/6/5
Description:     		关联服务端链路管理基类定义，负责管理Zookeeper类中的链路管理
*****************************************************************************************/

#ifndef __SERVERLINKMGR_H__
#define __SERVERLINKMGR_H__
#include <string.h>
#include <list>
#include "configfilereader.h"
#include "util.h"
#include "singleton.h"
#include "serverinfo.h"
#include "serverlink.h"


using namespace std;
using namespace imsvr;

#define SYSTEM_STACKSIZE 16*1024*1024
typedef unordered_map<UidCode_t, CServerLink*,hash_func>  SessionServerLinkMap_t;
typedef list<string> AssociatedPathList_t;


class CServerLinkMgr 
{
public:

	CServerLinkMgr();
	~CServerLinkMgr();
	static AssocSvrInfo_t* m_pAssocSvrInfo;					// All associated server information . 
	static int m_nAssocSvrCount;									// Total of associated server. 
	static void ServerLinkTimeout(
				void* callback_data, 
				uint8_t msg, 
				uint32_t handle, 
				void* pParam);

	void InitLinkMgr();
	CServerLink* GetAssocSvrLink(int16_t nServiceId,UidCode_t sessionId); //Get a valid link for sending message. 
	void GetLocalHost(string& sIP,uint16_t& nPort) {sIP = m_sLocalIP; nPort = m_nLocalPort;}
	uint64_t GetAssocHeartbeat(void){return (m_bAssocHeartbeatEnabled) ? m_nAssocHeartBeatInterval : 0;}	
	void AddLinkBySessionId(UidCode_t sessionId,CServerLink* pLink);
	void DelLinkBySessionId(UidCode_t sessionId);
	CServerLink* GetLinkBySessionId(UidCode_t sessionId);		
	CLock* GetSvrLock(void) { return m_pLock;}
protected:
	void InitAssocSvr(void);
	virtual void ConnectAssocSvr(AssocSvrList_t* pAssocSvr);	
	virtual CServerLink* GetRandomLink(serv_info_t* pSvrList,int nServiceCount,UidCode_t sessionId);
	//virtual serv_info_t* GetExistAssocSvr(int item,serv_info_t* pSvr); //Get assocsvr info from list
	virtual bool CheckValidLink(serv_info_t* pSvrList,int nServiceCount);
	virtual bool SetLocalHost(void);	  	//Load server host parameter from Config file. 
	virtual bool SetServicePath(void);		//Set service path convert from service id.
	virtual bool SetAssocSvrId(void);		//Load associated server from config file .
	virtual bool SetAssocHeartbeat(void);  
	virtual bool SetSysLimitFile(void);
	int16_t GetServiceIdByType(char* pServiceStr); //Get service id by server type str. 
	int16_t GetServiceIdByPath(char* pPath); //Get service id by server path str. 
	string GetServicePath(int16_t nServiceId); //Get service path by server id. 
	string GetHostString(string sIp, uint16_t nPort);
	void MergeAssocSvr(AssocSvrList_t* pAssocSvr,int index);
protected:
	CConfigFileReader* m_pConfigReader;
	CLock* m_pLock;								//Thread lock 
	SessionServerLinkMap_t m_mapSessionLink; 
    AssociatedPathList_t m_lsAssocatedPath; 	// List of associated path, the list item format is same as m_sLocalPath

	/*** 			用来注册zookeeper节点				*****/
    string m_sServicePath;						// format as "/usxxx",used for setting zookeeper node path.
    string m_sLocalIP;
	uint16_t m_nLocalPort;
	
	int16_t m_nAssocSvrId;						// Associated server id , the value get from system config file. 
	uint8_t m_bAssocHeartbeatEnabled;			// Enable assoc server hearbeat
	uint64_t m_nAssocHeartBeatInterval;			// Associated server heartbeat interval. 
	int		m_nTotalOfSysOpenfile;
	unsigned int		m_nSizeOfSysStack;
};


#endif
