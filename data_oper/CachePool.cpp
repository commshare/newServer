/*
 * CachePool.cpp
 *
 *  Created on: 2014年7月24日
 *      Author: ziteng
 *  Modify By ZhangYuanhao
 *  2015-01-13
 *  Add some redis command
 * Modify By ZhangYuanhao
 * 2015-01-14
 * Add comment for function
 *
 */

#include "CachePool.h"
#include "configfilereader.h"


#define MIN_CACHE_CONN_CNT	2

CacheManager* CacheManager::s_cache_manager = NULL;

CacheConn::CacheConn(CachePool* pCachePool)
{
	m_pCachePool = pCachePool;
	m_pContext = NULL;
	m_last_connect_time = 0;
}

CacheConn::~CacheConn()
{
	if (m_pContext) {
		redisFree(m_pContext);
		m_pContext = NULL;
	}
}

/*
 * redis初始化连接和重连操作，类似mysql_ping()
 */
int CacheConn::Init()
{
	if (m_pContext) {
		return 0;
	}

	// 4s 尝试重连一次
	uint64_t cur_time = (uint64_t)time(NULL);
	if (cur_time < m_last_connect_time + 4) {
		return 1;
	}

	m_last_connect_time = cur_time;

	// 200ms超时
	struct timeval timeout = {0, 200000};
	m_pContext = redisConnectWithTimeout(m_pCachePool->GetServerIP(), m_pCachePool->GetServerPort(), timeout);
	if (!m_pContext || m_pContext->err) {
		if (m_pContext) {
			ErrLog("redisConnect failed: %s", m_pContext->errstr);
			redisFree(m_pContext);
			m_pContext = NULL;
		} else {
			ErrLog("redisConnect failed");
		}

		return 1;
	}

	if(!m_pCachePool->GetPassWord().empty())
	{
		redisReply* reply = (redisReply *)redisCommand(m_pContext, "AUTH %s", m_pCachePool->GetPassWord().c_str());
		if (reply && (reply->type == REDIS_REPLY_STATUS) && (strncmp(reply->str, "OK", 2) == 0)) {
			freeReplyObject(reply);
			
		} else {
			ErrLog("author failed");
			return 2;
		}
	}
		
	redisReply* reply1 = (redisReply *)redisCommand(m_pContext, "SELECT %d", m_pCachePool->GetDBNum());
	if (reply1 && (reply1->type == REDIS_REPLY_STATUS) && (strncmp(reply1->str, "OK", 2) == 0)) {
		freeReplyObject(reply1);
		return 0;
	} else {
		ErrLog("select cache db failed");
		return 3;
	}
}


const char* CacheConn::GetPoolName()
{
	return m_pCachePool->GetPoolName();
}


REDIS_OPER_CODE CacheConn::get(const string& key, string& ret_value)
{
	if(Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "GET %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	if (reply->type == REDIS_REPLY_STRING)
	{
		ret_value.append(reply->str, reply->len);
		freeReplyObject(reply);
		return REDIS_OPER_OK;
	}

	freeReplyObject(reply);
	return REDIS_OPER_NODATE;
}

REDIS_OPER_CODE CacheConn::setex(const string& key, int timeout, const string& value, string& ret_value)
{
	if(Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SETEX %s %d %s", key.c_str(), timeout, value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value.append(reply->str, reply->len);
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::set(const string& key, const string& value, string& ret_value)
{
	if(Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SET %s %s", key.c_str(), value.c_str());
	if (!reply)
	{
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value.append(reply->str, reply->len);
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::mget(const vector<string>& keys, map<string, string>& ret_value)
{
	if(keys.empty())
		return REDIS_OPER_PARAM_EXCEPT;
	if(Init())
		return REDIS_OPER_CONNFAIL;
	string strKey;
	bool bFirst = true;
	for (auto& it: keys) 
	{
		if(bFirst)
		{
			bFirst = false;
			strKey = it;
		}
		else
		{
			strKey += " " + it;
		}
	}
	strKey = "MGET " + strKey;
	redisReply* reply = (redisReply*) redisCommand(m_pContext, strKey.c_str());
	if (!reply) 
	{
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}
	if(reply->type == REDIS_REPLY_ARRAY)
	{
		for(size_t i=0; i<reply->elements; ++i)
		{
			redisReply* child_reply = reply->element[i];
			if (child_reply->type == REDIS_REPLY_STRING) 
			{
				ret_value[keys[i]] = child_reply->str;
			}
		}
		freeReplyObject(reply);
		return REDIS_OPER_OK;
	}
	freeReplyObject(reply);
	return REDIS_OPER_NODATE;
}

REDIS_OPER_CODE CacheConn::isExists(const string &key)
{
    if (Init()) 
        return REDIS_OPER_CONNFAIL;
    redisReply* reply = (redisReply*) redisCommand(m_pContext, "EXISTS %s", key.c_str());
    if(!reply)
    {
        log("redisCommand failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
		m_pContext = NULL;
        return REDIS_OPER_DISCONN;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    if(0 == ret_value)
        return REDIS_OPER_NODATE;
	return REDIS_OPER_OK;
}


REDIS_OPER_CODE CacheConn::isExistsKeys(const vector<string>& keys, vector<string>& existKeys)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	for(auto& it : keys)
	{
		redisAppendCommand(m_pContext, "EXISTS %s", it.c_str());
		
	}

	redisReply* reply;
	for(auto& it : keys)
	{
		redisGetReply(m_pContext,(void**)(&reply));
		
		if(!reply)
	    {
	        log("redisCommand failed:%s", m_pContext->errstr);
	        redisFree(m_pContext);
			m_pContext = NULL;
	        return REDIS_OPER_DISCONN;
	    }
		
	    long ret_value = reply->integer;
	    freeReplyObject(reply);
	    if(0 != ret_value)
	    {
			existKeys.emplace_back(it);
		}
	}
	return REDIS_OPER_OK;
	
}

REDIS_OPER_CODE CacheConn::del(const string& key)
{
	 if (Init()) 
		return REDIS_OPER_CONNFAIL;
    redisReply* reply = (redisReply*) redisCommand(m_pContext, "DEL %s", key.c_str());
    if(!reply)
    {
        log("redisCommand failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    if(0 == ret_value || 1 == ret_value )
    {
        return REDIS_OPER_OK;
    }
	return REDIS_OPER_NODATE;

}

REDIS_OPER_CODE CacheConn::delKeys(const vector<string>& keys)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;
	
	int i =0;
	for(auto& it : keys)
	{
		redisAppendCommand(m_pContext, "DEL %s", it.c_str());
		i++;
	}

	redisReply* reply;
	for(; i>0; i--)
	{
		redisGetReply(m_pContext,(void**)(&reply));
		if(!reply)
		{
			log("redisCommand failed:%s", m_pContext->errstr);
			redisFree(m_pContext);
			m_pContext = NULL;
			return REDIS_OPER_DISCONN;
		}
		
		long ret_value = reply->integer;
		freeReplyObject(reply);
		if(0 == ret_value || 1 == ret_value )
			continue;
	}
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::hdel(const string& key, const string& field, long& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HDEL %s %s", key.c_str(), field.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::hget(const string& key, const string& field, string& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HGET %s %s", key.c_str(), field.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	if (reply->type == REDIS_REPLY_STRING)
	{
		ret_value.append(reply->str, reply->len);
		freeReplyObject(reply);
		return REDIS_OPER_OK;
	}

	freeReplyObject(reply);
	return REDIS_OPER_NODATE;
}

REDIS_OPER_CODE CacheConn::hgetAll(const string& key, map<string, string>& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HGETALL %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	if ( (reply->type == REDIS_REPLY_ARRAY) && (reply->elements % 2 == 0) ) 
	{
		for (size_t i = 0; i < reply->elements; i += 2) 
		{
			redisReply* field_reply = reply->element[i];
			redisReply* value_reply = reply->element[i + 1];

			string field(field_reply->str, field_reply->len);
			string value(value_reply->str, value_reply->len);
			ret_value.insert(make_pair(field, value));
		}
		freeReplyObject(reply);
		return REDIS_OPER_OK;
	}

	freeReplyObject(reply);
	return REDIS_OPER_NODATE;
}

REDIS_OPER_CODE CacheConn::hgetAllBykeys(const vector<string>& keys, map<string,map<string,string>>& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	for(auto& it : keys)
	{
		redisAppendCommand(m_pContext, "HGETALL %s", it.c_str());
	}

	
	redisReply* reply;
	for(auto& it : keys)
	{
		
		redisGetReply(m_pContext,(void**)(&reply));

		map<string,string> fields;
		
		if (!reply) {
			log("key no exist:%s", it.c_str());
			ret_value.insert(make_pair(it,fields));
			
		}

		if ( (reply->type == REDIS_REPLY_ARRAY) && (reply->elements % 2 == 0) ) {
			for (size_t i = 0; i < reply->elements; i += 2) {
				redisReply* field_reply = reply->element[i];
				redisReply* value_reply = reply->element[i + 1];

				string field(field_reply->str, field_reply->len);
				string value(value_reply->str, value_reply->len);
				fields.insert(make_pair(field, value));
			}
			ret_value.insert(make_pair(it,fields));
		}
		else
		{
			ret_value.insert(make_pair(it,fields));
		}

		freeReplyObject(reply);
	}

	DbgLog("ret_value size[%d]",ret_value.size());
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::hmgetAllBykeys(std::vector<string> keys, std::vector<string> fields, map<string, vector<string>>& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;
	
	std::string strField = "";
	for(auto& itor : fields)
		strField = strField + " " + itor;


	char buf[256];
	for(auto& itor : keys)
	{
		snprintf(buf, 256, "HMGET %s%s", itor.c_str(), strField.c_str());
		redisAppendCommand(m_pContext, buf);
		memset(buf, 0, 256);
	}
	

	redisReply* reply = NULL;
	for(auto& itor : keys)
	{
		redisGetReply(m_pContext,(void**)(&reply));
		if(!reply)
		{
			redisFree(m_pContext);
			m_pContext = NULL;
			return REDIS_OPER_DISCONN;
		}
		
		vector<string> field_value;
		if (reply->type == REDIS_REPLY_ARRAY) 
		{
			for (size_t i = 0; i < reply->elements; i++)
			{
				redisReply* value_reply = reply->element[i];
				string value(value_reply->str, value_reply->len);
				field_value.emplace_back(value);
			}
			ret_value.emplace(itor,field_value);
		}
		freeReplyObject(reply);
		reply = NULL;
	}
	
	DbgLog("ret_value size[%d]",ret_value.size());
	return REDIS_OPER_OK;
}


REDIS_OPER_CODE CacheConn::hset(const string& key, const string& field, string value, long& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::hincrBy(const string& key, const string& field, long value, long& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HINCRBY %s %s %ld", key.c_str(), field.c_str(), value);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::incrBy(const string& key, long value, long& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply*)redisCommand(m_pContext, "INCRBY %s %ld", key.c_str(), value);
	if(!reply)
	{
		log("redis Command failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}
	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::hmset(const string& key, map<string, string>& hash, string& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	int argc = hash.size() * 2 + 2;
	const char** argv = new const char* [argc];
	if (!argv) 
		return REDIS_OPER_PARAM_EXCEPT;

	argv[0] = "HMSET";
	argv[1] = key.c_str();
	int i = 2;
	for (map<string, string>::iterator it = hash.begin(); it != hash.end(); it++) {
		argv[i++] = it->first.c_str();
		argv[i++] = it->second.c_str();
	}

	redisReply* reply = (redisReply *)redisCommandArgv(m_pContext, argc, argv, NULL);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		delete [] argv;

		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value.append(reply->str, reply->len);

	delete [] argv;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}


REDIS_OPER_CODE CacheConn::hmget(const string& key, const vector<string>& fields, vector<string>& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	int argc = fields.size() + 2;
	const char** argv = new const char* [argc];
	if (!argv) 
	{
		return REDIS_OPER_PARAM_EXCEPT;
	}

	argv[0] = "HMGET";
	argv[1] = key.c_str();
	int i = 2;
	for (auto& it : fields)
	{
		argv[i++] = it.c_str();
	}

	redisReply* reply = (redisReply *)redisCommandArgv(m_pContext, argc, (const char**)argv, NULL);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		delete [] argv;
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
		delete [] argv;
		freeReplyObject(reply);
		return REDIS_OPER_OK;
	}

	delete [] argv;
	freeReplyObject(reply);
	return REDIS_OPER_NODATE;
}

REDIS_OPER_CODE CacheConn::incr(const string& key, long& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply*)redisCommand(m_pContext, "INCR %s", key.c_str());
	if(!reply)
	{
		log("redis Command failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}
	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::decr(const string& key, long& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply*)redisCommand(m_pContext, "DECR %s", key.c_str());
	if(!reply)
	{
		log("redis Command failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}
	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::lpush(const string& key, const string& value, long& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LPUSH %s %s", key.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::lpop(const string& key,string& value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LPOP %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	if (reply->type == REDIS_REPLY_STRING) 
	{
		value.append(reply->str, reply->len);
		freeReplyObject(reply);
		return REDIS_OPER_OK;
	}
	
	freeReplyObject(reply);
	return REDIS_OPER_NODATE;
}

REDIS_OPER_CODE CacheConn::lrem(const string& key, const string& value, long& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LREM %s -1 %s", key.c_str(),value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::rpush(const string& key, const string& value, long& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "RPUSH %s %s", key.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::rmpush(const string& key, vector<string>& values, long& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	for(auto& it : values)
	{
		redisAppendCommand(m_pContext, "RPUSH %s %s", key.c_str(), it.c_str());
	}
	redisReply* reply;
	redisGetReply(m_pContext,(void**)(&reply));

	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;

}

REDIS_OPER_CODE CacheConn::llen(const string& key, long& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LLEN %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}
	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::lrange(const string& key, long start, long end, vector<string>& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LRANGE %s %d %d", key.c_str(), start, end);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.emplace_back(value);
		}
		freeReplyObject(reply);
		return REDIS_OPER_OK;
	}

	freeReplyObject(reply);
	return REDIS_OPER_NODATE;
}

REDIS_OPER_CODE CacheConn::sadd(const std::string& key, const std::vector<string>& values, long& ret_value)
{
	if(Init())
		return REDIS_OPER_CONNFAIL;

	std::string strValue = "SADD " + key;
	for(auto& itor : values)
		strValue = strValue + " " + itor;
	DbgLog("value:%s", strValue.c_str());
	redisReply* reply = (redisReply *)redisCommand(m_pContext, strValue.c_str());
	if (!reply)
	{
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::sadd(const std::string& key, const std::string& value, long& ret_value)
{
	if(Init())
		return REDIS_OPER_CONNFAIL;
	
	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SADD %s %s", key.c_str(), value.c_str());
	if (!reply)
	{
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}


// 获取集合元素的个数
REDIS_OPER_CODE CacheConn::scard(const std::string& key, long& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SCARD %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}
	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

// 获取集合所有元素
REDIS_OPER_CODE CacheConn::smembers(const std::string& key, vector<string>& ret_value)
{
	if (Init())
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SMEMBERS %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.emplace_back(value);
		}
		freeReplyObject(reply);
		return REDIS_OPER_OK;
	}

	freeReplyObject(reply);
	return REDIS_OPER_NODATE;
}

// 删除元素
REDIS_OPER_CODE CacheConn::srem(const std::string& key, const std::vector<string>& values, long& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	std::string strValue = "SREM " + key;
	for(auto& itor : values)
		strValue = strValue + " " + itor;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, strValue.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::srem(const std::string& key, const std::string& value, long& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SREM %s %s", key.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}

REDIS_OPER_CODE CacheConn::smove(const std::string& srcKey, const std::string& destKey, const std::string& value, long& ret_value)
{
	if (Init()) 
		return REDIS_OPER_CONNFAIL;

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SMOVE %s %s %s", srcKey.c_str(), destKey.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return REDIS_OPER_DISCONN;
	}

	ret_value = reply->integer;
	freeReplyObject(reply);
	return REDIS_OPER_OK;
}




///////////////
CachePool::CachePool(const char* pool_name, const char* server_ip, int server_port, int db_num, int max_conn_cnt,
			string password)
{
	m_pool_name = pool_name;
	m_server_ip = server_ip;
	m_server_port = server_port;
	m_db_num = db_num;
	m_max_conn_cnt = max_conn_cnt;
	m_cur_conn_cnt = MIN_CACHE_CONN_CNT;
	m_password = password;
}

CachePool::~CachePool()
{
	m_free_notify.Lock();
	for (list<CacheConn*>::iterator it = m_free_list.begin(); it != m_free_list.end(); it++) {
		CacheConn* pConn = *it;
		delete pConn;
	}

	m_free_list.clear();
	m_cur_conn_cnt = 0;
	m_free_notify.Unlock();
}

int CachePool::Init()
{
	for (int i = 0; i < m_cur_conn_cnt; i++) {
		CacheConn* pConn = new CacheConn(this);
		if (pConn->Init()) {
			delete pConn;
			return 1;
		}

		m_free_list.push_back(pConn);
	}

	log("cache pool: %s, list size: %lu", m_pool_name.c_str(), m_free_list.size());
	return 0;
}

CacheConn* CachePool::GetCacheConn()
{
	m_free_notify.Lock();

	while (m_free_list.empty()) 
	{
		if (m_cur_conn_cnt >= m_max_conn_cnt) 
		{
			m_free_notify.Wait();
		} 
		else 
		{
			CacheConn* pCacheConn = new CacheConn(this);
			int ret = pCacheConn->Init();
			if (ret) 
			{
				log("Init CacheConn failed");
				delete pCacheConn;
				m_free_notify.Unlock();
				return NULL;
			} 
			else
			{
				m_free_list.push_back(pCacheConn);
				m_cur_conn_cnt++;
				log("new cache connection: %s, conn_cnt: %d", m_pool_name.c_str(), m_cur_conn_cnt);
			}
		}
	}

	CacheConn* pConn = m_free_list.front();
	m_free_list.pop_front();

	m_free_notify.Unlock();

	return pConn;
}

void CachePool::RelCacheConn(CacheConn* pCacheConn)
{
	m_free_notify.Lock();

	list<CacheConn*>::iterator it = m_free_list.begin();
	for (; it != m_free_list.end(); it++) {
		if (*it == pCacheConn) {
			break;
		}
	}

	if (it == m_free_list.end()) {
		m_free_list.push_back(pCacheConn);
	}

	m_free_notify.Signal();
	m_free_notify.Unlock();
}

///////////
CacheManager::CacheManager()
{

}

CacheManager::~CacheManager()
{

}

CacheManager* CacheManager::getInstance()
{
	if (!s_cache_manager) {
		s_cache_manager = new CacheManager();
		if (s_cache_manager->Init()) {
			delete s_cache_manager;
			s_cache_manager = NULL;
		}
	}

	return s_cache_manager;
}

int CacheManager::Init()
{
	CConfigFileReader config_file("server.conf");

	char* cache_instances = config_file.GetConfigName("CacheInstances");
	if (!cache_instances) {
		log("not configure CacheIntance");
		return 1;
	}

	char host[64];
	char port[64];
	char db[64];
    char maxconncnt[64];
	char password[64];
	CStrExplode instances_name(cache_instances, ',');
	for (uint32_t i = 0; i < instances_name.GetItemCnt(); i++) {
		char* pool_name = instances_name.GetItem(i);
		//printf("%s", pool_name);
		snprintf(host, 64, "%s_host", pool_name);
		snprintf(port, 64, "%s_port", pool_name);
		snprintf(db, 64, "%s_db", pool_name);
        snprintf(maxconncnt, 64, "%s_maxconncnt", pool_name);
		snprintf(password, 64, "%s_password", pool_name);
		
		char* cache_host = config_file.GetConfigName(host);
		char* str_cache_port = config_file.GetConfigName(port);
		char* str_cache_db = config_file.GetConfigName(db);
        char* str_max_conn_cnt = config_file.GetConfigName(maxconncnt);
		char* str_password = config_file.GetConfigName(password);
		
		if (!cache_host || !str_cache_port || !str_cache_db || !str_max_conn_cnt) {
			log("not configure cache instance: %s", pool_name);
			return 2;
		}

		CachePool* pCachePool = new CachePool(pool_name, cache_host, atoi(str_cache_port),
				atoi(str_cache_db), atoi(str_max_conn_cnt),str_password);
		if (pCachePool->Init()) {
			log("Init cache pool failed");
			return 3;
		}

		m_cache_pool_map.insert(make_pair(pool_name, pCachePool));
	}

	return 0;
}

CacheConn* CacheManager::GetCacheConn(const char* pool_name)
{
	map<string, CachePool*>::iterator it = m_cache_pool_map.find(pool_name);
	if (it != m_cache_pool_map.end()) {
		return it->second->GetCacheConn();
	} else {
		return NULL;
	}
}

void CacheManager::RelCacheConn(CacheConn* pCacheConn)
{
	if (!pCacheConn) {
		return;
	}

	map<string, CachePool*>::iterator it = m_cache_pool_map.find(pCacheConn->GetPoolName());
	if (it != m_cache_pool_map.end()) {
		return it->second->RelCacheConn(pCacheConn);
	}
}

