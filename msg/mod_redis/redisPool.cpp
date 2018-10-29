/******************************************************************************
Filename: redisPool.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/24
Description: 
******************************************************************************/
#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "redisPool.h"
#include "configfilereader.h"


CRedisConn::CRedisConn(acl::redis_client_cluster* cluster)
:m_pCluster(cluster)
{
	m_keyCmd.set_cluster(m_pCluster, 100);
	m_stringCmd.set_cluster(m_pCluster, 100);
	m_hashCmd.set_cluster(m_pCluster, 100);
}

CRedisConn::~CRedisConn()
{

}

std::string CRedisConn::get(const std::string& key)
{
	m_stringCmd.clear();
	acl::string value;

	if (m_stringCmd.get(key.c_str(), value))
	{
		return value.c_str();
	}
	return "";
}

bool CRedisConn::set(const std::string& key, const std::string& val)
{
	m_stringCmd.clear();
	return m_stringCmd.set( key.c_str(), val.c_str() );
}


long CRedisConn::hdel(const std::string& key)
{
	m_keyCmd.clear();
	long result = m_keyCmd.del(key.c_str());
	if (-1 == result)
	{
		ErrLog("%s", m_keyCmd.result_error());
	}
	return result;
}

long CRedisConn::hdel(const std::string& key, const std::string& field)
{
	m_hashCmd.clear();
	long result = m_hashCmd.hdel(key.c_str(), field.c_str());
	if (-1 == result)
	{
		ErrLog("%s", m_hashCmd.result_error());
	}
	return result;
	//return m_hashCmd.hdel(key.c_str(), field.c_str());
}

std::string CRedisConn::hget(const std::string& key, const std::string& field)
{
	m_hashCmd.clear();
	acl::string value;

	if (m_hashCmd.hget(key.c_str(),field.c_str(), value))
	{
		return value.c_str();
	}
	return "";
}

bool CRedisConn::hgetAll(const std::string& key, std::vector<const char*>& names, std::vector<const char*>& values)
{
	m_hashCmd.clear();
	bool result = m_hashCmd.hgetall(key.c_str(), names, values);
	if (!result)
	{
		ErrLog("%s", m_hashCmd.result_error());
	}
	return result;
	//return m_hashCmd.hgetall(key.c_str(), names, values);
}

long CRedisConn::hset(const std::string& key, const std::string& field, const std::string& value)
{
	m_hashCmd.clear();
	long result = m_hashCmd.hset(key.c_str(), field.c_str(), value.c_str());
	if (-1 == result)
	{
		ErrLog("%s", m_hashCmd.result_error());
	}
	return result;
	//return m_hashCmd.hset(key.c_str(), field.c_str(), value.c_str());
}

bool CRedisConn::hmset(const std::string& key, std::map<std::string, std::string>& fieldVals)
{
	m_hashCmd.clear();
	std::map<acl::string, acl::string> hashVals;

	std::map<std::string, std::string>::iterator it = fieldVals.begin();
	while (it != fieldVals.end())
	{
		hashVals.insert(std::pair<acl::string, acl::string>(it->first.c_str(), it->second.c_str()));
		++it;
	}
	bool result = m_hashCmd.hmset(key.c_str(), hashVals);
	if (!result)
	{
		ErrLog("%s", m_hashCmd.result_error());
	}
	return result;
}

bool CRedisConn::hmset(const std::string& key, std::map<std::string, const char*>& fieldVals)
{
	m_hashCmd.clear();
	std::map<acl::string, const char*> hashVals;

	std::map<std::string, const char*>::iterator it = fieldVals.begin();
	while (it != fieldVals.end())
	{
		hashVals.insert(std::pair<acl::string, const char*>(it->first.c_str(), it->second));
		++it;
	}
	bool result = m_hashCmd.hmset(key.c_str(), hashVals);
	if (!result)
	{
		ErrLog("%s", m_hashCmd.result_error());
	}
	return result;
	//return m_hashCmd.hmset(key.c_str(), hashVals);
}

bool CRedisConn::hmget(const char* key, const std::vector<const char*>& names, std::vector<acl::string>* result /*= NULL*/)
{
	m_hashCmd.clear();
	return m_hashCmd.hmget(key, names, result);
}

////////////////////////////////////////////////////////////////
CRedisManager::~CRedisManager()
{

}

int CRedisManager::Init(const std::string& configFileName /*= string("")*/)
{
	CConfigFileReader config_file(configFileName.empty() ? "server.conf" : configFileName.c_str());

	char* db_instances = config_file.GetConfigName("MsgSvrRedisInstances");

	if (!db_instances)
	{
		m_bHasInit = false;
		ErrLog("not configure MsgSvrMongoDbInstances");
		return 1;
	}

	char retry_inter[64] = { '\0' };
	char redirect_max[64] = { '\0' };
	char redirect_sleep[64] = { '\0' };
	char addrs[128] = {'\0'};
	char maxconncnt[64] = { '\0' };
	char conn_timeout[64] = { '\0' };
	char rw_timeout[64] = { '\0' };
	char password[64] = { '\0' };

	CStrExplode instances_name(db_instances, ',');

	for (uint32_t i = 0; i < instances_name.GetItemCnt(); i++)
	{
		char* pool_name = instances_name.GetItem(i);

		snprintf(retry_inter, 64, "%s_retry_inter", pool_name);
		snprintf(redirect_max, 64, "%s_redirect_max", pool_name);
		snprintf(redirect_sleep, 64, "%s_redirect_sleep", pool_name);
		snprintf(addrs, 64, "%s_addrs", pool_name);
		snprintf(maxconncnt, 64, "%s_maxconncnt", pool_name);
		snprintf(conn_timeout, 64, "%s_conn_timeout", pool_name);
		snprintf(rw_timeout, 64, "%s_rw_timeout", pool_name);
		snprintf(password, 64, "%s_password", pool_name);		

		char* str_retry_inter = config_file.GetConfigName(retry_inter);
		char* str_redirect_max = config_file.GetConfigName(redirect_max);
		char* str_redirect_sleep = config_file.GetConfigName(redirect_sleep);
		char* db_addrs = config_file.GetConfigName(addrs);
		char* str_maxconncnt = config_file.GetConfigName(maxconncnt);
		char* str_conn_timeout = config_file.GetConfigName(conn_timeout);
		char* str_rw_timeout = config_file.GetConfigName(rw_timeout);
		char* db_password = config_file.GetConfigName(password);

		if (!db_addrs)
		{
			m_bHasInit = false;
			ErrLog("not configure redis instance: %s", pool_name);
			return 2;
		}

		int db_retry_inter = str_retry_inter ? atoi(str_retry_inter): -1;
		int db_redirect_max = str_redirect_max ? atoi(str_redirect_max) : 15;
		int db_redirect_sleep = str_redirect_max ? atoi(str_redirect_sleep) : 100 ;
		int db_maxconncnt = str_maxconncnt ? atoi(str_maxconncnt) : 0;
		int db_conn_timeout = str_conn_timeout ? atoi(str_conn_timeout) : 10;
		int db_rw_timeout = str_rw_timeout ? atoi(str_rw_timeout) : 10;
		//bool preset = true;

		acl::redis_client_cluster* pCluster = new acl::redis_client_cluster();

		if (NULL == pCluster)
		{
			m_bHasInit = false;
			ErrLog("create mongodb pool failed: %s", pool_name);
			return 3;
		}
		// 设置连接池失败后重试的时间时间隔（秒），
		//当该值 <= 0 时，若连接池出现问题则会立即被重试
		pCluster->set_retry_inter(db_retry_inter);

		// 设置重定向的最大阀值，若重定向次数超过此阀值则报错
		pCluster->set_redirect_max(db_redirect_max);

		// 当重定向次数 >= 2 时每次再重定向此函数设置休息的时间(毫秒)
		pCluster->set_redirect_sleep(db_redirect_sleep);
		
		// 设置连接 redis 集群的密码，第一个参数为一个 redis 服务节点的服务地址，
		// 当第一个参数值为 default 时，则设置了所有节点的统一连接密码
		if (db_password)
			pCluster->set_password("default", db_password);

		pCluster->init(NULL, db_addrs, db_maxconncnt, db_conn_timeout, db_rw_timeout);		

		m_redisClusterMap.insert(make_pair(pool_name, pCluster));
	}
	m_bHasInit = true;
	return 0;
}

CRedisConn CRedisManager::GetCRedisConn()
{
	if (!m_bHasInit)
	{
		if (false == Init())
		{
			return NULL;
		}
	}

	std::map<std::string, acl::redis_client_cluster*>::iterator it = m_redisClusterMap.begin();
	if (it == m_redisClusterMap.end())
	{
		return NULL;
	}
	else
	{
		return CRedisConn(it->second);
	}
}

CRedisManager::CRedisManager()
{

}

bool CRedisManager::m_bHasInit = false;
