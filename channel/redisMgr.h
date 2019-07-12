/******************************************************************************
Filename: redisMgr.h
autho:bob
date:2018-12-18
description:定义了WS server相关的redis操作
******************************************************************************/
#ifndef REDISMGR_H_
#define REDISMGR_H_

#include "common.h"
#include "singleton.h"
#include <string>
#include <list>
#include <map>
#include <im.mes.pb.h>
#include "CachePool.h"

class CRedisMgr:public Singleton<CRedisMgr>
{
public:
	CRedisMgr();
	~CRedisMgr();

public:
	bool init();

/////////////////////////////////////////////////////////
public:
	bool getUserLoginInfo(const std::string& userId, USER_LOGIN_INIFO_& userInfo);
	bool getUserLoginInfoList(const std::vector<std::string>& users, std::vector<USER_LOGIN_INIFO_>& userInfoList);

private:
	bool getLoginInfo(CacheConn* conn, const std::string& userId, USER_LOGIN_INIFO_& userInfo);
	string getLoginInfoKey(const string& userId);

//////////////////////////////////////////////////////////
public:
	bool getChannelUser(const std::string& radioId, std::vector<std::string>& vecUser);
	bool getChannelOfflineUser(const std::string& radioId, std::vector<std::string>& vecUser);
	bool getUserChannel(const std::string& userId, std::vector<std::string>& vecRadio);
	bool getChannelAdmin(const std::string& radioId, std::vector<std::string>& vecAdmin);

	bool addUserToChannel(const std::string& radioId, const std::string& userId);
	bool addUserToChannel(const std::string& radioId, const std::vector<std::string>& vecUser);
	bool addUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId);
	bool removeUserToChannel(const std::string& radioId, const std::string& userId);
	bool removeUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId);
	bool removeUserToChannel(const std::string& radioId, const std::vector<std::string>& vecUser);
	bool moveOnlineUserToChannel(const std::string& radioId, const std::string& userId);
	bool deleteChannelUserKey(const std::string& radioId);

	bool addChannelToUser(const std::string& userId, const std::map<std::string, int>& mapRadio);
	bool addChannelToUser(const std::string& userId, const std::string& radioId, int nStatus = 0);
	bool addChannelToUser(const std::vector<std::string>& vecUser, const std::string& radioId, int nStatus = 0);
	bool removeChannelToUser(const std::string& userId, const std::string& radioId);
	bool removeChannelToUser(const std::string& userId, const std::vector<std::string>& vecRadio);
	bool removeChannelToUser(const std::vector<std::string>& vecUser, const std::string& radioId);
	// 获取用户所在频道用户设置的状态
	bool getChannelSetOnStatus(const std::string& userId, const std::string& radioId, int nStatus);
	

	bool addChannelAdmin(const std::string& radioId, const std::string& admin);
	bool addChannelAdminList(const std::string& radioId, const std::vector<std::string>& vecAdmin);
	bool removeChannelAdmin(const std::string& radioId, const std::string& admin);
	bool removeChannelAdminList(const std::string& radioId, const std::vector<std::string>& vecAdmin);

	bool addOfflineUserToChannel(const std::string& radioId, const std::string& userId);
	bool moveOfflineUserToChannel(const std::string& radioId, const std::string& userId);
	bool removeOfflineUserToChannel(const std::string& radioId, const std::string& userId);
	bool removeOfflineUserToChannel(const std::string& radioId, const std::vector<std::string>& vecUser);
	bool deleteChannelOfflineUserKey(const std::string& radioId);

private:
	// set
	bool getSetMembers(const std::string& key, std::vector<std::string>& vecMember);
	bool addMemberToSet(CacheConn* conn, const std::string& key, const std::string& member);
	bool addMemberListToSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members);
	bool removeMemberFromSet(CacheConn* conn, const std::string& key, const std::string& member);
	bool removeMemberListFromSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members);
	bool moveSetMember(CacheConn* conn, const std::string& srcKey, const std::string& destKey, const std::string& member);
	bool delKey(CacheConn* conn, const std::string& key);

	// sorted set
	bool getSortedSetMember(const std::string& key, std::vector<std::string>& vecMember);
	bool getSortedSetMember(const std::string& key, std::map<std::string, int>& mapMember);
	bool addMemberToSortedSet(CacheConn* conn, const std::string& key, const std::string& member, int nSocre);
	bool addMemberListToSortedSet(CacheConn* conn, const std::string& key, const std::map<std::string, int>& mapMember);
	bool removeMemberFromSortedSet(CacheConn* conn, const std::string& key, const std::string& member);
	bool removeMemberListFromSortedSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members);
	bool getSortedSetMemberSocrt(CacheConn* conn, const std::string& key, const std::string& member, int& nSocre);
	
	
	string getUserChannelKey(const std::string& userId);
	string getChannelUserKey(const std::string& radioId);
	string getChannelAdminKey(const std::string& radioId);
	string getChannelOfflineUserKey(const std::string& radioId);

//////////////////////////////////////////////////////////
public:
	bool getChannelInfo(const std::string& radioId, CHANNEL_INIFO_& chnnInfo);
	bool setChannelInfo(const CHANNEL_INIFO_& chnnInfo);
	bool setChannelInfoList(const std::vector<CHANNEL_INIFO_>& vecChnnInfo);
	bool setChannelStatus(const std::string& radioId, CHNN_STATUS_ status);
	bool setChannelSpeak(const std::string& radioId, int unSpeak);

private:
	std::string getChannelInfoKey(const std::string& radioId);

//////////////////////////////////////////////////////////
public:
	bool pushMsgToMsgQueue(const im::RadioChat& msg, const string& mongo_id);
	bool removeMsgFromMsgQueue(const std::string& radioId, const std::string& value);

/////////////////////////////////////////////////////////
public:
	bool getChannelMemberList(const std::string& url, const std::string& Secret, const std::string& strRadioId, std::vector<string>& vecMember, string& strCode);
	bool parseChannelMemberList(const std::string& strRadioId,  const std::string& strData, std::vector<string>& vecMember, string& strCode);

	bool getChannelInfoList(const std::string& url, const std::string& Secret, std::vector<CHANNEL_INIFO_>& vecChnnInfo, string& strCode);
	bool parseChannelInfoList(const std::string& strData, std::vector<CHANNEL_INIFO_>& vecChnnInfo, string& strCode);
	

private:
	string getMsgCacheKey(const string& radioId);

private:
	int m_nMsgCacheSzie;
	
};

#endif //REDISMGR_H_
