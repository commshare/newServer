/******************************************************************************
Filename: mysqlPool.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/16
Description: 
******************************************************************************/
#include "mysqlPool.h"
#include "configfilereader.h"
#include <vector>
#include <stdio.h>
CDBConn::CDBConn(acl::db_handle* Handle, CDBPool* pDBPool)
	:m_pHandle(Handle), m_pDBPool(pDBPool)
{

}

CDBConn::~CDBConn()
{
	if (m_pHandle)
	m_pDBPool->GetRealPool()->put(m_pHandle);
}

bool CDBConn::sql_select(const char* sql, acl::db_rows* result /*= NULL*/)
{
	if (NULL == m_pHandle) return false;
	if (!m_pHandle->sql_select(sql, result))
	{
		WarnLog("exec (%s) error -- %s", sql, m_pHandle->get_error());
		return false;
	}
	DbgLog("sql_select %s", sql);
	return true;
}

bool CDBConn::sql_update(const char* sql)
{
	if (NULL == m_pHandle) return false;
	if (!m_pHandle->sql_update(sql))
	{
		WarnLog("exec (%s) error, (%s)", sql, m_pHandle->get_error());
		return false;
	}
	DbgLog("sql_update %s", sql);
	return true;
}

const acl::db_rows* CDBConn::get_result() const
{
	if (NULL == m_pHandle) return NULL;
	return m_pHandle->get_result();
}

const char* CDBConn::GetPoolName()
{
	return m_pDBPool->GetPoolName();
}

void CDBConn::free_result()
{
	if (NULL == m_pHandle) return;
	m_pHandle->free_result();
}

bool CDBConn::exec_select(acl::query& query, acl::db_rows* result /*= NULL*/)
{
	if (NULL == m_pHandle) return false;
	if (!m_pHandle->exec_select(query, result))
	{
		WarnLog("exec_select %s error, (%s)", query.to_string().c_str(), m_pHandle->get_error());
		return false;
	}
	DbgLog("exec_select %s", query.to_string().c_str());
	return true;
}

bool CDBConn::exec_update(acl::query& query)
{
	if (NULL == m_pHandle) return false;
	if (!m_pHandle->exec_update(query))
	{
		WarnLog("exec_update (%s) error, (%s)", query.to_string().c_str(), m_pHandle->get_error());
		return false;
	}
	DbgLog("exec_update %s", query.to_string().c_str());
	return true;
}


////////////////////////////////
CDBPool::CDBPool(const char* pool_name, const char* db_server_host, 
	const char* username, const char* password, const char* db_name, int max_conn_cnt)
	:m_nDbMaxConnCnt(max_conn_cnt), m_sPoolName(pool_name), m_sDbServerHost(db_server_host),
	m_sUserName(username?username:""), m_sPassword(password?password:""), m_sDbName(db_name)
{
	//char addr[64] = { '\0' };
	//sprintf(addr, "%s:%d", m_sDbServerIp.c_str(), m_nDbServerPort);
	m_pDBPool = new acl::mysql_pool(db_server_host, m_sDbName.c_str(), m_sUserName.c_str(), m_sPassword.c_str(), m_nDbMaxConnCnt);
	if (m_pDBPool)
	{
		m_pDBPool->set_idle(120);
	}
}

CDBPool::~CDBPool()
{

}

int CDBPool::Init()
{
	std::vector< std::unique_ptr<CDBConn> > dbs;
	for (int i = 0; i < m_nDbMaxConnCnt; i++)
	{
		std::unique_ptr<CDBConn> ptr = GetDBConn();
		if (NULL == ptr.get())
		{
			ErrLog("db pool: %s, size: %d", m_sPoolName.c_str(), m_nDbMaxConnCnt);
			return 1;
		}
		dbs.push_back(std::move(ptr));
	}

	ErrLog("db pool name =  %s, max conn size = %d", m_sPoolName.c_str(), m_nDbMaxConnCnt);
	return 0;
}

CDBPool& CDBPool::set_idle_ttl(time_t ttl)
{
	if (m_pDBPool)
		m_pDBPool->set_idle_ttl(ttl);
	return *this;
}
std::unique_ptr<CDBConn> CDBPool::GetDBConn()
{
	return std::unique_ptr<CDBConn>(new CDBConn(m_pDBPool->peek_open(), this));
}

//////////////////////////////////////////////////////////////////
CDBManager::CDBManager()
{

}

CDBManager::~CDBManager()
{

}


int CDBManager::Init(CConfigFileReader* pReader)
{
	//CConfigFileReader config_file(configFileName.empty() ? "server.conf" : configFileName.c_str());
	if (NULL == pReader)
	{
		//m_bHasInit = false;
		ErrLog("configureFile not specified");
		return 1;
	}

	CConfigFileReader &config_file = *pReader;
	
	//load mysql shared library
	//char* mysqlLibPath = config_file.GetConfigName("DBLibPath");
	//const char* libname = "libmysqlclient_r.so";
	//acl::string path;
	//path.format("%s/%s", mysqlLibPath ? mysqlLibPath : "/usr/lib64/mysql", libname);
	//acl::db_handle::set_loadpath(path);
	//DbgLog("set mysql loadPath = %s", path.c_str());

	//acl::log::stdout_open(true);

	char* db_instances = config_file.GetConfigName("GrpSvrMysqlDbInstances");

	if (!db_instances)
	{
		ErrLog("not configure MsgSvrMongoDbInstances");
		return 1;
	}

	char host[64];
	//char port[64];
	char dbname[64];
	char username[64];
	char password[64];
	char maxconncnt[64];
	CStrExplode instances_name(db_instances, ',');

	for (uint32_t i = 0; i < instances_name.GetItemCnt(); i++) 
	{
		char* pool_name = instances_name.GetItem(i);
		snprintf(host, 64, "%s_host", pool_name);
		//snprintf(port, 64, "%s_port", pool_name);
		snprintf(dbname, 64, "%s_datebase", pool_name);
		snprintf(username, 64, "%s_username", pool_name);
		snprintf(password, 64, "%s_password", pool_name);
		snprintf(maxconncnt, 64, "%s_maxconncnt", pool_name);

		char* db_host = config_file.GetConfigName(host);
		//char* str_db_port = config_file.GetConfigName(port);
		char* db_dbname = config_file.GetConfigName(dbname);
		char* db_username = config_file.GetConfigName(username);
		char* db_password = config_file.GetConfigName(password);
		char* str_maxconncnt = config_file.GetConfigName(maxconncnt);

		if (!db_host || !db_dbname || !str_maxconncnt) 
		{
			ErrLog("not configure db instance: %s", pool_name);
			return 2;
		}

		//int db_port = str_db_port ? atoi(str_db_port) : 3306;
		int db_maxconncnt = atoi(str_maxconncnt);
		CDBPool* pDBPool = new CDBPool(pool_name, db_host, db_username, db_password, db_dbname, db_maxconncnt);
		if (pDBPool->Init()) 
		{
			ErrLog("init db instance failed: %s", pool_name);
			return 3;
		}
		log("mysql connect to %s,db:%s success", db_host, db_dbname);
		m_dbpoolMap.insert(make_pair(pool_name, pDBPool));
	}

	return 0;
}

std::unique_ptr<CDBConn> CDBManager::GetDBConn(const string& dbpool_name)
{
	map<string, CDBPool*>::iterator it = m_dbpoolMap.begin();
	if (!dbpool_name.empty())
	{
		it = m_dbpoolMap.find(dbpool_name);
	}

	if (it == m_dbpoolMap.end())
	{
		return NULL;
	}
	else
	{
		return it->second->GetDBConn();
	}
}
