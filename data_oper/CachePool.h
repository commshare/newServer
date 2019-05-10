/*
 * CachePool.h
 *
 *  Created on: 2014年7月24日
 *      Author: ziteng
 *  Modify By ZhangYuanhao 
 *  2015-01-13
 *  Add some redis command
 */

#ifndef CACHEPOOL_H_
#define CACHEPOOL_H_

#include <vector>
#include "../base/util.h"
#include "hiredis.h"
#include "threadpool.h"

enum REDIS_OPER_CODE
{
	REDIS_OPER_OK = 0,
	REDIS_OPER_DISCONN = 1,
	REDIS_OPER_CONNFAIL = 2,
	REDIS_OPER_NODATE = 3,
	REDIS_OPER_PARAM_EXCEPT = 4
};

class CachePool;
class CacheConn {
public:
	CacheConn(CachePool* pCachePool);
	virtual ~CacheConn();

	int Init();
	const char* GetPoolName();
	
	REDIS_OPER_CODE get(const string& key, string& ret_value);
	REDIS_OPER_CODE setex(const string& key, int timeout, const string& value, string& ret_value);
	REDIS_OPER_CODE set(const string& key, const string& value, string& ret_value);

	//批量获取
	REDIS_OPER_CODE mget(const vector<string>& keys, map<string, string>& ret_value);
	// 判断一个key是否存在
	REDIS_OPER_CODE isExists(const string &key);
	REDIS_OPER_CODE isExistsKeys(const vector<string>& keys, vector<string>& existKeys);

	//删除key
	REDIS_OPER_CODE del(const string& key);
	REDIS_OPER_CODE delKeys(const vector<string>& keys);

	// Redis hash structure
	REDIS_OPER_CODE hdel(const string& key, const string& field, long& ret_value);
	REDIS_OPER_CODE hget(const string& key, const string& field, string& ret_value);
	REDIS_OPER_CODE hgetAll(const string& key, map<string, string>& ret_value);
	REDIS_OPER_CODE hgetAllBykeys(const vector<string>& keys, map<string,map<string,string>>& ret_value);
	REDIS_OPER_CODE hmgetAllBykeys(std::vector<string> keys, std::vector<string> fields, map<string, vector<string>>& ret_value);
	REDIS_OPER_CODE hset(const string& key, const string& field, string value, long& ret_value);

	REDIS_OPER_CODE hincrBy(const string& key, const string& field, long value, long& ret_value);
	REDIS_OPER_CODE incrBy(const string& key, long value, long& ret_value);
	REDIS_OPER_CODE hmset(const string& key, map<string, string>& hash, string& ret_value);
	REDIS_OPER_CODE hmget(const string& key, const vector<string>& fields, vector<string>& ret_value);

	//原子加减1
	REDIS_OPER_CODE incr(const string& key, long& ret_value);
	REDIS_OPER_CODE decr(const string& key, long& ret_value);

	// Redis list structure
	REDIS_OPER_CODE lpush(const string& key, const string& value, long& ret_value);
	REDIS_OPER_CODE lpop(const string& key,string& value);
	REDIS_OPER_CODE lrem(const string& key, const string& value, long& ret_value);
	REDIS_OPER_CODE rpush(const string& key, const string& value, long& ret_value);
	REDIS_OPER_CODE rmpush(const string& key, vector<string>& values, long& ret_value);
	REDIS_OPER_CODE llen(const string& key, long& ret_value);
	REDIS_OPER_CODE lrange(const string& key, long start, long end, vector<string>& ret_value);

	// redis set structure
	REDIS_OPER_CODE sadd(const std::string& key, const std::vector<string>& values, long& ret_value);
	REDIS_OPER_CODE sadd(const std::string& key, const string& value, long& ret_value);
	// 获取集合元素的个数
	REDIS_OPER_CODE scard(const std::string& key, long& ret_value);
	// 获取集合所有元素
	REDIS_OPER_CODE smembers(const std::string& key, vector<string>& ret_value);
	// 删除元素
	REDIS_OPER_CODE srem(const std::string& key, const std::vector<string>& values, long& ret_value);
	REDIS_OPER_CODE srem(const std::string& key, const std::string& values, long& ret_value);
	// 移动元素
	REDIS_OPER_CODE smove(const std::string& srcKey, const std::string& destKey, const std::string& value, long& ret_value);
	

private:
	CachePool* 		m_pCachePool;
	redisContext* 	m_pContext;
	uint64_t		m_last_connect_time;
};

class CachePool {
public:
	CachePool(const char* pool_name, const char* server_ip, int server_port, int db_num, int max_conn_cnt,string password);
	virtual ~CachePool();

	int Init();

	CacheConn* GetCacheConn();
	void RelCacheConn(CacheConn* pCacheConn);

	const char* GetPoolName() { return m_pool_name.c_str(); }
	const char* GetServerIP() { return m_server_ip.c_str(); }
	int GetServerPort() { return m_server_port; }
	int GetDBNum() { return m_db_num; }
	string GetPassWord() { return m_password;}
private:
	string 		m_pool_name;
	string		m_server_ip;
	int			m_server_port;
	int			m_db_num;
	string      m_password;

	int			m_cur_conn_cnt;
	int 		m_max_conn_cnt;
	list<CacheConn*>	m_free_list;
	CThreadNotify		m_free_notify;
};

class CacheManager {
public:
	virtual ~CacheManager();

	static CacheManager* getInstance();

	int Init();
	CacheConn* GetCacheConn(const char* pool_name);
	void RelCacheConn(CacheConn* pCacheConn);
private:
	CacheManager();

private:
	static CacheManager* 	s_cache_manager;
	map<string, CachePool*>	m_cache_pool_map;
};

#endif /* CACHEPOOL_H_ */
