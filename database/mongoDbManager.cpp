/******************************************************************************
Filename: mongoDbManager.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/08/15
Description: 
******************************************************************************/

#include <mongocxx/client.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/uri.hpp>
#include "configfilereader.h"
#include "threadpool.h"
#include "mongoDbManager.h"
#include "util.h"
using mongocxx::collection;

CMongoDbConn::CMongoDbConn(std::unique_ptr< client, std::function< void(client *)>> client)
	:mRealClient(move(client))
{

}

CMongoDbConn::CMongoDbConn(CMongoDbConn && conn)
	: mRealClient(move(conn.mRealClient))
{

}

CMongoDbConn::~CMongoDbConn()
{

}

collection CMongoDbConn::GetCollection(const string& dbName, const string& collName) const
{
	return (*mRealClient)[dbName][collName];
}


////////////////////////////////////////////////////////////////////////////////////////////////
CMongoDbPool::CMongoDbPool(const char* pool_name, const char* db_server_ip, uint16_t db_server_port,
	const char* username, const char* password, int min_conn_cnt, int max_conn_cnt)
	:m_nServerPort(db_server_port),	 m_nMinConnCnt(std::min(min_conn_cnt, max_conn_cnt)),
	m_nMaxConnCnt(max_conn_cnt),		 m_sPoolName(pool_name),		m_sServerIp(db_server_ip),
	m_sUserName(username ? username : ""),		m_sPassword(password ? password : ""),
	m_RealPool(CMongoDbPool::generateUri(db_server_ip, db_server_port, username, password, m_nMinConnCnt, m_nMaxConnCnt) )
{

}

CMongoDbPool::~CMongoDbPool()
{

}

std::unique_ptr<CMongoDbConn> CMongoDbPool::GetDBConn()
{
	return std::unique_ptr<CMongoDbConn>(new CMongoDbConn(m_RealPool.acquire()));
}


mongocxx::uri CMongoDbPool::generateUri(const char* db_server_ip, uint16_t db_server_port,
				const char* username, const char* password, int min_conn_cnt, int max_conn_cnt)
{
	char uriStr[256] = { 0 };
	int len = 0;

	len += sprintf(uriStr + len, "mongodb://");

	//username and password
	if (username != NULL && username[0] != '\0')
	{
		len += sprintf(uriStr + len, "%s", username);
		if (password != NULL)
		{
			len += sprintf(uriStr + len, ":%s", password);
		}
		len += sprintf(uriStr + len, "@");
	}

	len += sprintf(uriStr + len, "%s:%d/?minPoolSize=%d&maxPoolSize=%d",
		db_server_ip, db_server_port, min_conn_cnt, max_conn_cnt);

	DbgLog("generateUri:%s", uriStr);
	return mongocxx::uri(uriStr);
}



///////////////////////////////////////////////////////////////

CMongoDbManager::CMongoDbManager()
	:m_inst{}	//load mongocxx driver
{

}

CMongoDbManager::~CMongoDbManager()
{

}

int CMongoDbManager::Init(CConfigFileReader* pReader)
{
	if (NULL == pReader)
	{
		m_bHasInit = false;
		ErrLog("configureFile not specified");
		return 1;
	}

	CConfigFileReader &config_file = *pReader;

	char* db_instances = config_file.GetConfigName("MsgSvrMongoDbInstances");

	if (!db_instances)
	{
		m_bHasInit = false;
		ErrLog("not configure MsgSvrMongoDbInstances");
		return 1;
	}

	char host[64];
	char port[64];
	char username[64];
	char password[64];
	char minconncnt[64];
	char maxconncnt[64];
	CStrExplode instances_name(db_instances, ',');

	for (uint32_t i = 0; i < instances_name.GetItemCnt(); i++)
	{
		char* pool_name = instances_name.GetItem(i);
		snprintf(host, 64, "%s_host", pool_name);
		snprintf(port, 64, "%s_port", pool_name);
		snprintf(username, 64, "%s_username", pool_name);
		snprintf(password, 64, "%s_password", pool_name);
		snprintf(minconncnt, 64, "%s_minconncnt", pool_name);
		snprintf(maxconncnt, 64, "%s_maxconncnt", pool_name);

		char* db_host = config_file.GetConfigName(host);
		char* str_db_port = config_file.GetConfigName(port);
		char* db_username = config_file.GetConfigName(username);
		char* db_password = config_file.GetConfigName(password);
		char* str_minconncnt = config_file.GetConfigName(minconncnt);
		char* str_maxconncnt = config_file.GetConfigName(maxconncnt);

		if (!db_host || !str_db_port)
		{
			m_bHasInit = false;
			ErrLog("not configure mongodb instance: %s", pool_name);
			return 2;
		}

		int db_port = atoi(str_db_port);
		int db_minconncnt = atoi(str_minconncnt);
		int db_maxconncnt = atoi(str_maxconncnt);
		CMongoDbPool* pDBPool = new CMongoDbPool(pool_name, db_host, db_port,
			db_username, db_password, db_minconncnt, db_maxconncnt);
		if (NULL == pDBPool)
		{
			m_bHasInit = false;
			ErrLog("create mongodb pool failed: %s", pool_name);
			return 3;
		}
		m_dbpoolMap.insert(make_pair(pool_name, pDBPool));
	}

	char* str_mongo_thread_num = config_file.GetConfigName("mongo_thread_num");
	int mongo_thread_num = str_mongo_thread_num ? atoi(str_mongo_thread_num) : 40;
	m_pMongoThreadPool = new CImThreadPool();
	//如果创建成功但初始化失败，则返回NULL
	if (m_pMongoThreadPool != NULL && m_pMongoThreadPool->Init(mongo_thread_num) != 0)
	{
		DbgLog("mongothread created failed, size = %d", mongo_thread_num);
		delete m_pMongoThreadPool;
		m_pMongoThreadPool = NULL;
		return 1;
	}
	log("CMongoDbManager created %d threads", mongo_thread_num);

	m_bHasInit = true;
	return 0;
}

CImThreadPool* CMongoDbManager::GetMongoThreadPoolInstance()
{
	return m_pMongoThreadPool;
}

CMongoDbPool* CMongoDbManager::getDBConnPool() const
{
	if (!m_bHasInit)
	{
		return NULL;
	}
	map<string, CMongoDbPool*>::const_iterator it = m_dbpoolMap.begin();
	if (it == m_dbpoolMap.end())
	{
		return NULL;
	}
	else
	{
		return it->second;
	}
}


std::unique_ptr<CMongoDbConn> CMongoDbManager::GetDBConn()
{
	CMongoDbPool* pPool = getDBConnPool();
	if (pPool) return pPool->GetDBConn();
	return NULL;
}

void CMongoDbManager::Stop()
{
	if (m_pMongoThreadPool)
	{
		m_pMongoThreadPool->StopThreads();
		m_pMongoThreadPool->waitForDone();		//主线程等待所有的mongoThread结束
	}
}

CImThreadPool* CMongoDbManager::m_pMongoThreadPool = NULL;

bool CMongoDbManager::m_bHasInit = false;
