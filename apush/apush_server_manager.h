/**
 * apush_server_manager.h 
 *  
 * app manager 
 * hold the apushclient and apushserver 
 *  
 * add by liulang 
 *  
 * 2017-07-01 
 */

#ifndef apush_server_manager_hpp
#define apush_server_manager_hpp

#include "push_provide_server.h"
//#include "hw_push_client.h"
#include "base_client.h"


class CApushLocalSvr;
class CBaseClient;

class CApushLocalManager
{

public:

	CApushLocalManager()
	{
	}
	~CApushLocalManager()
	{
	}

	static CApushLocalManager *GetInstance();

	bool RegistSvr(CApushLocalSvr *Svr)
	{
		if (!Svr)
		{
			ErrLog("Err RegistSvr!");
			return false;
		}

		m_apushLocalSvr = Svr;

		return true;
	}
	
	CApushLocalSvr *GetServer();


	bool RegistClient(CBaseClient *Client, int iType)
	{
		if (!Client || iType < 0)
		{
			ErrLog("Err RegistClient!");
			return false;
		}

		CAutoLock lock(&m_mapMutex);

		m_clientMap.insert(make_pair(iType, Client));
		
		return true;
	}


	CBaseClient *GetClient(int iType)
	{
		if (iType < 0)
		{

			ErrLog("GetClient!");
			return nullptr;
		}

		CBaseClient *client = nullptr;

		map<int, CBaseClient *>::iterator it = m_clientMap.find(iType);
		if (it != m_clientMap.end())
		{
			client = it->second;
		}
		
		if (client == nullptr)
		{
			ErrLog("client!");
		}
		
		return client;
	}

private:
	CApushLocalSvr	*m_apushLocalSvr;
	CLock			m_mapMutex;
	map<int, CBaseClient*> m_clientMap;

};

#endif


