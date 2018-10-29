/*****************************************************************************************
Filename:  zookeeper.h
Author: jack			Version: im-1.0 		Date:2017/6/6
Description:   	网络层zookeeper网络监测类定义。该类负责各关联服务的注测、
				链路维护、关联服务节点信息拉取
*****************************************************************************************/

#ifndef __ZOOKEEPER_H__
#define __ZOOKEEPER_H__
#include "zookeeper/zookeeper.h"
#include "util.h"
#include "thread.h"
#include "singleton.h"
#include "serverinfo.h"
#include "serverlinkmgr.h"

#define REGISTNODE_SIZE 128

class CZookeeper : public CServerLinkMgr,
	public CThread, public Singleton<CZookeeper>
{
public:
	CZookeeper();
	~CZookeeper();
	
	//static void* StartRoutine(void* arg);
	virtual void StartThread();
	virtual void StopThread();
	virtual void OnThreadRun(void);	
		
	virtual bool Initialize(CConfigFileReader* pConfigReader);
	virtual void ConnectZookeeper();					//Connect to zookeeper host
	virtual void Finalize();							//Close a zookeeper connection			
	virtual bool RegistService();						//Regist server node to zookeeper host
	void SetConnect(bool bConn) { m_bConnect = bConn;if(m_bConnect)RegistService();}
	void ResetConnect(void);
	int64_t GetSessionId() {return (m_zHandle) ? zoo_client_id(m_zHandle)->client_id : -1;}
protected:
	static void Event(zhandle_t* zh,int type,	int state,const char* path,void* context); // event callback function
//	static void StringCompletion(int ret,const char* value,const void* data); 			   // reserverd for acreate mode
	virtual bool PullAssocSvr(void);			// pull all the assoc svr from zookeeper host.
	virtual bool UpdateAssocSvr(string sPath);					//update associated server captured from zookeeper host	
	virtual bool RefreshAssocSvr(AssocSvrList_t* pAssocSvrList,struct String_vector& nodeInfo); 
	virtual void RefreshAssocSvrItem(AssocSvrList_t* pAssocSvrList,string sIP,uint16_t nPort);
	virtual void ClearAssocSvr(AssocSvrList_t* pAssocSvrList); //Free assoc svr list allocated last. 
	virtual bool SetZookeeper(void);	  	//Load zookeeper config parameter from Config file. 
	virtual AssocSvrList_t* GetAssocSvr(int16_t nServiceId);
	
private:
	zhandle_t* m_zHandle;			//Handle of zoo_init return value. 
	AssocSvrList_t* m_pAssocSvrList;	//Associated server array capture from zookeepr  
	string m_sZookeeperUrl;			//zookeeper host , support in single or cluster mode			
	bool m_bConnect;				//zookeeper connection flag 
	bool m_bRegist;					//zookeeper regist flag;
	bool m_bRunning;				//zookeeper client thread startup flag
	int64_t m_nSessionId;			//Session id driverd from zookeeper host
	int	 m_nRetryTimes;
	int  m_nTimeout;
	int  m_nRetryInterval;			//Interval time every scanning zk host. 
	char m_sRegistName[REGISTNODE_SIZE];

};


#endif
