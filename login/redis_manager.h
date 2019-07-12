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
	
	bool addChannelToUser(const std::string& userId, const std::map<std::string, int>& mapRadio);

	bool getUserChannel(const std::string& userId, std::map<std::string, int>& mapRadio);

	bool moveOnlineUserToChannel(const std::string& radioId, const std::string& userId);
	bool moveOnlineUserToChannel(const std::map<std::string, int>& mapRadio, const std::string& userId);
	bool moveOfflineUserToChannel(const std::string& radioId, int nStatus, const std::string& userId, bool bNewMsg, bool bHideSound);
	bool moveOfflineUserToChannel(const std::map<std::string, int>& mapRadio, const std::string& userId, bool bNewMsg = 1, bool bHideSound = 1);
	// 获取用户所在频道用户设置的状态
	bool getChannelSetOnStatus(const std::string& userId, const std::string& radioId, int nStatus);

private:
	bool getSetMembers(const std::string& key, std::vector<std::string>& vecMember);
	bool addMemberToSet(CacheConn* conn, const std::string& key, const std::string& member);
	bool addMemberListToSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members);
	bool removeMemberFromSet(CacheConn* conn, const std::string& key, const std::string& member);
	bool removeMemberListFromSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members);
	bool moveSetMember(CacheConn* conn, const std::string& srcKey, const std::string& destKey, const std::string& member);

	// sorted set
	bool getSortedSetMember(const std::string& key, std::vector<std::string>& vecMember);
	bool getSortedSetMember(const std::string& key, std::map<std::string, int>& mapMember);
	bool addMemberToSortedSet(CacheConn* conn, const std::string& key, const std::string& member, int nSocre);
	bool addMemberListToSortedSet(CacheConn* conn, const std::string& key, const std::map<std::string, int>& mapMember);
	bool removeMemberFromSortedSet(CacheConn* conn, const std::string& key, const std::string& member);
	bool removeMemberListFromSortedSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members);
	bool getSortedSetMemberSocrt(CacheConn* conn, const std::string& key, const std::string& member, int& nSocre);

private:
	string getUserChannelKey(const std::string& userId);
	string getChannelUserKey(const std::string& radioId);
	string getChannelOfflineUserKey(const std::string& radioId);
	
};


#endif // __REDIS_MANAGER_H__
