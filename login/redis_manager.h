#ifndef __REDIS_MANAGER_H__
#define __REDIS_MANAGER_H__

#include "singleton.h"
#include "CachePool.h"

class CRedisManager : public Singleton<CRedisManager>
{
public:
	virtual ~CRedisManager();

private:
	CRedisManager();
	friend class Singleton <CRedisManager>;

public:
	bool initManager();

public:
	bool addUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId);
	bool removeUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId);
	
	bool addChannelToUser(const std::string& userId, const std::vector<std::string>& vecRadio);

	bool getUserChannel(const std::string& userId, std::vector<std::string>& vecRadio);

	bool moveOnlineUserToChannel(const std::string& radioId, const std::string& userId);
	bool moveOnlineUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId);
	bool moveOfflineUserToChannel(const std::string& radioId, const std::string& userId);
	bool moveOfflineUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId);

private:
	bool getSetMembers(const std::string& key, std::vector<std::string>& vecMember);
	bool addMemberToSet(CacheConn* conn, const std::string& key, const std::string& member);
	bool addMemberListToSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members);
	bool removeMemberFromSet(CacheConn* conn, const std::string& key, const std::string& member);
	bool removeMemberListFromSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members);
	bool moveSetMember(CacheConn* conn, const std::string& srcKey, const std::string& destKey, const std::string& member);

private:
	string getUserChannelKey(const std::string& userId);
	string getChannelUserKey(const std::string& radioId);
	string getChannelOfflineUserKey(const std::string& radioId);
	
};


#endif // __REDIS_MANAGER_H__
