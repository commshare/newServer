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
#include <string>
#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "singleton.h"

class CachePool;

class CRedisConn
{
public:
	CRedisConn(acl::redis_client_cluster* cluster);
	virtual ~CRedisConn();
	std::string lpop(const char* key);
	int rpush(const char* key, const std::vector<acl::string>& values);

private:
	acl::redis_client_cluster*  m_pCluster;
	acl::redis_list m_listCmd;
};

class CRedisManager :public Singleton < CRedisManager >
{
public:
	virtual ~CRedisManager();

	int Init(const std::string& configFileName = std::string(""));
	CRedisConn GetCRedisConn();
private:
	friend class Singleton < CRedisManager >;
	CRedisManager();

private:
	std::map<std::string, acl::redis_client_cluster*>	m_redisClusterMap;
	static bool m_bHasInit;
};

#endif /* CACHEPOOL_H_ */
