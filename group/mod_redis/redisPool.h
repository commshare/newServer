/******************************************************************************
Filename: redisPool.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/24
Description: 
******************************************************************************/
#ifndef CACHEPOOL_H_
#define CACHEPOOL_H_

#include <string>
#include <map>
#include <vector>
#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "singleton.h"

class CachePool;
class CConfigFileReader;

class CRedisConn
{
public:
	CRedisConn(acl::redis_client_cluster* cluster);
	virtual ~CRedisConn();

	std::string get(const std::string& key);
	bool		set(const std::string& key, const std::string& val);

	// Redis hash structure
	long hdel(const std::string& key, const std::string& field);
	long hset(const std::string& key, const std::string& field, const std::string& value);
	std::string hget(const std::string& key, const std::string& field);
	bool hgetAll(const std::string& key, std::vector<const char*>& names,
					std::vector<const char*>& values);

	bool hmset(const std::string& key, std::map<std::string, std::string>& hash);
	bool hmset(const std::string& key, std::map<std::string, const char*>& hash);
	bool hmget(const char* key, const std::vector<const char*>& names,
		std::vector<acl::string>* result = NULL);

private:
	acl::redis_client_cluster*  m_pCluster;
	acl::redis_key m_keyCmd;
	acl::redis_string m_stringCmd;
	acl::redis_hash m_hashCmd;
};

class CRedisManager :public Singleton < CRedisManager >
{
public:
	virtual ~CRedisManager();

	int Init(CConfigFileReader* pReader);
	CRedisConn GetCRedisConn();
private:
	friend class Singleton < CRedisManager >;
	CRedisManager();

private:
	std::map<std::string, acl::redis_client_cluster*>	m_redisClusterMap;
	static bool m_bHasInit;
};

#endif /* CACHEPOOL_H_ */
