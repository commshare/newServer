#include "redis_manager.h"
#include "AutoPool.h"


const std::string PRE_CHNN_USER_ = "CHNN_USER_";
const std::string PRE_USER_CHNN_ = "USER_CHNNSTATUS_";
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

bool CRedisManager::addChannelToUser(const std::string& userId, const std::map<std::string, int>& mapRadio)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getUserChannelKey(userId);
	return addMemberListToSortedSet(conn, key, mapRadio);
}

bool CRedisManager::getUserChannel(const std::string& userId, std::map<std::string, int>& mapRadio)
{
	std::string key = getUserChannelKey(userId);
	return getSortedSetMember(key, mapRadio);

}

bool CRedisManager::moveOnlineUserToChannel(const std::string& radioId, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string srcKey = getChannelOfflineUserKey(radioId);
	std::string destKey = getChannelUserKey(radioId);
	
	return removeMemberFromSet(conn, srcKey, userId) && addMemberToSet(conn, destKey, userId);
}

bool CRedisManager::moveOnlineUserToChannel(const std::map<std::string, int>& mapRadio, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string srcKey = "";
	std::string destKey = "";
	for(auto& itor : mapRadio)
	{
		srcKey = getChannelOfflineUserKey(itor.first);
		destKey = getChannelUserKey(itor.first);
		removeMemberFromSet(conn, srcKey, userId);
		addMemberToSet(conn, destKey, userId);
	}
	return true;
}

bool CRedisManager::moveOfflineUserToChannel(const std::string& radioId, int nStatus, const std::string& userId, bool bNewMsg, bool bHideSound)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string srcKey = getChannelUserKey(radioId);
	std::string destKey = getChannelOfflineUserKey(radioId);
	bool IsHide = nStatus & 0x01 ? true : false;
	bool IsUndisturb = nStatus & 0x10 ? true : false;
	bool bPush = bNewMsg ? ( !IsUndisturb ? ( IsHide ? ( bHideSound ? true : false ) : true ) : false ) : false;
	ErrLog("channel %s user %s push status %d hide %d undisturb %d newmsg %d hideSound %d", radioId.c_str(), userId.c_str(), bPush, IsHide, IsUndisturb, bNewMsg, bHideSound);
	removeMemberFromSet(conn, srcKey, userId);
	if(bPush)
		addMemberToSet(conn, destKey, userId);
	return true;
}

bool CRedisManager::moveOfflineUserToChannel(const std::map<std::string, int>& mapRadio, const std::string& userId, bool bNewMsg, bool bHideSound)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string srcKey = "";
	std::string destKey = "";	
	for(auto& itor : mapRadio)
	{
		srcKey = getChannelUserKey(itor.first);
		destKey = getChannelOfflineUserKey(itor.first);
		bool IsHide = itor.second & 0x01 ? true : false;
		bool IsUndisturb = itor.second & 0x10 ? true : false;
		bool bPush = bNewMsg ? ( !IsUndisturb ? ( IsHide ? ( bHideSound ? true : false ) : true ) : false ) : false;
		ErrLog("channel %s user %s push status %d hide %d undisturb %d newmsg %d hideSound %d", itor.first.c_str(), userId.c_str(), bPush, IsHide, IsUndisturb, bNewMsg, bHideSound);
		removeMemberFromSet(conn, srcKey, userId);
		if(bPush)
			addMemberToSet(conn, destKey, userId);
	}
	return true;
}

bool CRedisManager::getChannelSetOnStatus(const std::string& userId, const std::string& radioId, int nStatus)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getUserChannelKey(userId);
	return getSortedSetMemberSocrt(conn, key, radioId, nStatus);
}



bool CRedisManager::getSetMembers(const std::string& key, std::vector<std::string>& vecMember)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = cache.getCacheConn()->smembers(key, vecMember)) && nTick < 3)
		nTick++;
	DbgLog("key=%s code=%d result=%d", key.c_str(), code, vecMember.size());
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
	DbgLog("key=%s code=%d result=%d", key.c_str(), code, ret_value);
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
	DbgLog("key=%s code=%d result=%d", key.c_str(), code, ret_value);
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
	DbgLog("key=%s code=%d result=%d", key.c_str(), code, ret_value);
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
	DbgLog("key=%s code=%d result=%d", key.c_str(), code, ret_value);
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
	DbgLog("srcKey=%s destKey=%s member=%s code=%d result=%d", srcKey.c_str(), destKey.c_str(), member.c_str(), code, ret_value);
	if(REDIS_OPER_OK != code || ret_value < 1 || nTick >= 3)
		return false;
	return true;
}



bool CRedisManager::getSortedSetMember(const std::string& key, std::vector<std::string>& vecMember)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = cache.getCacheConn()->zrange(key, vecMember)) && nTick < 3)
		nTick++;
	DbgLog("key=%s code=%d result=%d", key.c_str(), code, vecMember.size());
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::getSortedSetMember(const std::string& key, std::map<std::string, int>& mapMember)
{
	CAutoCache cache(CACHE_NAME_CHNN_);
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = cache.getCacheConn()->zrange(key, mapMember)) && nTick < 3)
		nTick++;
	DbgLog("key=%s code=%d result=%d", key.c_str(), code, mapMember.size());
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::addMemberToSortedSet(CacheConn* conn, const std::string& key, const std::string& member, int nSocre)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->zadd(key, member, nSocre)) && nTick < 3)
		nTick++;
	DbgLog("key=%s code=%d", key.c_str(), code);
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::addMemberListToSortedSet(CacheConn* conn, const std::string& key, const std::map<std::string, int>& mapMember)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->zadd(key, mapMember)) && nTick < 3)
		nTick++;
	DbgLog("key=%s code=%d", key.c_str(), code);
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::removeMemberFromSortedSet(CacheConn* conn, const std::string& key, const std::string& member)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->zrem(key, member)) && nTick < 3)
		nTick++;
	DbgLog("key=%s code=%d result=%s", key.c_str(), code, member.c_str());
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::removeMemberListFromSortedSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->zrem(key, members)) && nTick < 3)
		nTick++;
	DbgLog("key=%s code=%d", key.c_str(), code);
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisManager::getSortedSetMemberSocrt(CacheConn* conn, const std::string& key, const std::string& member, int& nSocre)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->zscore(key, member, nSocre)) && nTick < 3)
		nTick++;
	DbgLog("code=%d result=%d", code, nSocre);
	if(REDIS_OPER_OK != code || nTick >= 3)
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




