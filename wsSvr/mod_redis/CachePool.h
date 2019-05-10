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
#include "threadpool.h"
#include "hiredis.h"

class CachePool;

class CacheConn {
public:
	CacheConn(CachePool* pCachePool);
	virtual ~CacheConn();

	int Init();
	const char* GetPoolName();

	string get(string key);
	string setex(string key, int timeout, string value);
    string set(string key, string& value);
    
    //批量获取
    bool mget(const vector<string>& keys, map<string, string>& ret_value);
    // 判断一个key是否存在
    bool isExists(string &key);
	bool isExistsKeys(list<string> keys,list<string>& existKeys);

	//删除key
	bool del(string key);
	bool delKeys(list<string> keys);

	// Redis hash structure
	long hdel(string key, string field);
	string hget(string key, string field);
	bool hgetAll(string key, map<string, string>& ret_value);
	bool hgetAllBykeys(list<string> keys,map<string,map<string,string>>& ret_value);
	long hset(string key, string field, string value);

	long hincrBy(string key, string field, long value);
    long incrBy(string key, long value);
	string hmset(string key, map<string, string>& hash);
	bool hmget(string key, list<string>& fields, list<string>& ret_value);
    
    //原子加减1
    long incr(string key);
    long decr(string key);

	// Redis list structure
	long lpush(string key, string value);
	long lpop(string key,string& value);
	long lrem(string key,string value);
	long rpush(string key, string value);
	long rmpush(string key, list<string>& values);
	long llen(string key);
	bool lrange(string key, long start, long end, list<string>& ret_value);

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
