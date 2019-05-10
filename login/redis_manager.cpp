#include "redis_manager.h"
#include "AutoPool.h"


const std::string PRE_CHNN_USER_ = "CHNN_USER_";
const std::string PRE_USER_CHNN_ = "USER_CHNN_";
const std::string PRE_CHNN_OFFLINE_USER_ = "CHNN_OFFLINE_USER_";


const std::string CACHE_NAME_CHNN_ = "channelDb";


CRedisManager::CRedisManager()
{

}

CRedisManager::~CRedisManager()
{

}


bool CRedisManager::initManager()
{
	int ret = CacheManager::getInstance()->Init();
	return ret == 0 ? true : false;
}

bool CRedisManager::addUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	for(auto& itor : vecRadio)
	{
		std::string key = getChannelUserKey(itor);
		if(!addMemberToSet(conn, key, userId))
			continue;
	}
	return true;
}

bool CRedisManager::removeUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	for(auto& itor : vecRadio)
	{
		std::string key = getChannelUserKey(itor);
		if(!removeMemberFromSet(conn, key, userId))
			continue;
	}
	return true;
}

bool CRedisManager::addChannelToUser(const std::string& userId, const std::vector<std::string>& vecRadio)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getUserChannelKey(userId);
	return addMemberListToSet(conn, key, vecRadio);
}

bool CRedisManager::getUserChannel(const std::string& userId, std::vector<std::string>& vecRadio)
{
	std::string key = getUserChannelKey(userId);
	return getSetMembers(key, vecRadio);

}

bool CRedisManager::moveOnlineUserToChannel(const std::string& radioId, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string srcKey = getChannelOfflineUserKey(radioId);
	std::string destKey = getChannelUserKey(radioId);
	
	return moveSetMember(conn, srcKey, destKey, userId);
}

bool CRedisManager::moveOnlineUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string srcKey = "";
	std::string destKey = "";
	for(auto& itor : vecRadio)
	{
		srcKey = getChannelOfflineUserKey(itor);
		destKey = getChannelUserKey(itor);
		if(!moveSetMember(conn, srcKey, destKey, userId))
			addMemberToSet(conn, destKey, userId);
	}
	return true;
}

bool CRedisManager::moveOfflineUserToChannel(const std::string& radioId, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelOfflineUserKey(radioId);
	return removeMemberFromSet(conn, key, userId);
}

bool CRedisManager::moveOfflineUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string srcKey = "";
	std::string destKey = "";
	for(auto& itor : vecRadio)
	{
		srcKey = getChannelUserKey(itor);
		destKey = getChannelOfflineUserKey(itor);
		if(!moveSetMember(conn, srcKey, destKey, userId))
			addMemberToSet(conn, destKey, userId);
	}
	return true;
}


bool CRedisManager::getSetMembers(const std::string& key, std::vector<std::string>& vecMember)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = cache.getCacheConn()->smembers(key, vecMember)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::addMemberToSet(CacheConn* conn, const std::string& key, const std::string& member)
{
	long ret_value = 0;
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->sadd(key, member, ret_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::addMemberListToSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members)
{
	if(members.empty())
		return false;
	long ret_value = 0;
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->sadd(key, members, ret_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::removeMemberFromSet(CacheConn* conn, const std::string& key, const std::string& member)
{
	long ret_value = 0;
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->srem(key, member, ret_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::removeMemberListFromSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members)
{
	if(members.empty())
		return false;
	long ret_value = 0;
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->srem(key, members, ret_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::moveSetMember(CacheConn* conn, const std::string& srcKey, const std::string& destKey, const std::string& member)
{
	long ret_value = 0;
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->smove(srcKey, destKey, member, ret_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || ret_value < 1 || nTick >= 3)
		return false;
	return true;
}



string CRedisManager::getUserChannelKey(const std::string& userId)
{
	char key[128];
	snprintf(key, sizeof(key), "%s%s", PRE_USER_CHNN_.c_str(), userId.c_str());
	return key;
}

string CRedisManager::getChannelUserKey(const std::string& radioId)
{
	char key[128];
	snprintf(key, sizeof(key), "%s%s", PRE_CHNN_USER_.c_str(), radioId.c_str());
	return key;
}

string CRedisManager::getChannelOfflineUserKey(const std::string& radioId)
{
	char key[128];
	snprintf(key, sizeof(key), "%s%s", PRE_CHNN_OFFLINE_USER_.c_str(), radioId.c_str());
	return key;
}




