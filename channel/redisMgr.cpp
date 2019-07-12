/******************************************************************************

******************************************************************************/
#include <json/json.h>
#include "redisMgr.h"
#include "AutoPool.h"
#include "util.h"
#include "configfilereader.h"
#include "common.h"
#include "HttpClient.h"
#include "encdec.h"


const string CACHE_NAME_CHNL = "channelDb";
const string CACHE_NAME_LOGIN = "loginDb";
const string PRE_CM_ = "CM_";
const string PRE_CHNN_ = "chnn_";
const std::string PRE_CHNN_USER_ = "CHNN_USER_";
const std::string PRE_CHNN_OFFLINE_USER_ = "CHNN_OFFLINE_USER_";
const std::string PRE_USER_CHNN_ = "USER_CHNNSTATUS_";
const std::string PRE_CHNN_INFO_ = "CHNN_INFO_";
const std::string PRE_CHNN_ADMIN_ = "CHNN_ADMIN_";



const string CM_STATUS_ = "status";
const string CM_IP_ = "cm_ip";
const string CM_PORT_ = "cm_port";
const string CM_PUSHTYPE_ = "push_type";
const string CM_PUSHTOKEN_ = "push_token";


const string CHNN_INFO_STATUS = "status";
const string CHNN_INFO_SPEAK = "unspeak";


const string JSON_ID_ = "id";
const string JSON_FROMID_ = "fromId";
const string JSON_RADIOID_ = "radioId";
const string JSON_MSGID_ = "msgId";
const string JSON_MSGTIME_ = "msgTime";
const string JSON_ENCRYPT_ = "encrypt";
const string JSON_CONTENT_ = "content";
const string JSON_EXTEND_ = "extend";





CRedisMgr::CRedisMgr()
	: m_nMsgCacheSzie(40)
{

}

CRedisMgr::~CRedisMgr()
{

}

bool CRedisMgr::init()
{
	CConfigFileReader config_file("server.conf");
	char* cache_size = config_file.GetConfigName("msg_cache_size");
	if(cache_size)
		m_nMsgCacheSzie = atoi(cache_size);
	CacheManager::getInstance()->Init();

	

	return true;
}

bool CRedisMgr::getUserLoginInfo(const std::string& userId, USER_LOGIN_INIFO_& userInfo)
{
	CAutoCache cache(CACHE_NAME_LOGIN);
	return getLoginInfo(cache.getCacheConn(), userId, userInfo);
}

bool CRedisMgr::getUserLoginInfoList(const std::vector<std::string>& users, std::vector<USER_LOGIN_INIFO_>& userInfoList)
{
	CAutoCache cache(CACHE_NAME_LOGIN);
	CacheConn* conn = cache.getCacheConn();
	
	std::vector<std::string> vecKey;
	std::string strKey = "";
	for(auto& itor : users)
	{
		strKey = getLoginInfoKey(itor);
		vecKey.emplace_back(strKey);
	}		

	std::vector<string> fields;
	fields.push_back(CM_STATUS_);
	fields.push_back(CM_IP_);
	fields.push_back(CM_PORT_);
	fields.push_back(CM_PUSHTYPE_);
	fields.push_back(CM_PUSHTOKEN_);
	
	map<string, vector<string>> ret_value;
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->hmgetAllBykeys(vecKey, fields, ret_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	USER_LOGIN_INIFO_ userInfo;
	for(auto& itor : ret_value)
	{
		int nPos = itor.first.rfind("_");
		int nLen = itor.first.length();
		userInfo.userId = itor.first.substr(nPos+1, nLen);
		userInfo.status = atoi(itor.second[0].c_str());
		userInfo.strIp = itor.second[1].c_str();
		userInfo.nPort = atoi(itor.second[2].c_str());
		userInfo.nPushType = atoi(itor.second[3].c_str());
		userInfo.pushToken = itor.second[4].c_str();
		userInfoList.emplace_back(userInfo);
	}
	return true;
}

bool CRedisMgr::getLoginInfo(CacheConn* conn, const std::string& userId, USER_LOGIN_INIFO_& userInfo)
{
	userInfo.userId = userId;
	std::string key = getLoginInfoKey(userId);
	std::vector<string> fields;
	fields.push_back(CM_STATUS_);
	fields.push_back(CM_IP_);
	fields.push_back(CM_PORT_);
	fields.push_back(CM_PUSHTYPE_);
	fields.push_back(CM_PUSHTOKEN_);
	std::vector<string> res_value;
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->hmget(key, fields, res_value)) && nTick < 3)
		nTick++;
	if(nTick >= 3)
		return false;
	if(REDIS_OPER_OK != code)
		return false;
	userInfo.status = atoi(res_value[0].c_str());
	userInfo.strIp = res_value[1].c_str();
	userInfo.nPort = atoi(res_value[2].c_str());
	userInfo.nPushType = atoi(res_value[3].c_str());
	userInfo.pushToken = res_value[4].c_str();
	return true;
}

string CRedisMgr::getLoginInfoKey(const string& userId)
{
	char key[128];
	snprintf(key, sizeof(key), "%s%s", PRE_CM_.c_str(), userId.c_str());
	return key;
}

bool CRedisMgr::pushMsgToMsgQueue(const im::RadioChat& msg, const string& mongo_id)
{
	string radioId = msg.sradioid();
	Json::Value msgValue;
	msgValue[JSON_ID_] = mongo_id;
	msgValue[JSON_FROMID_] = msg.sfromid();
	msgValue[JSON_RADIOID_] = radioId;
	msgValue[JSON_MSGID_] = msg.smsgid();
	msgValue[JSON_MSGTIME_] = (long long)msg.msgtime();
	msgValue[JSON_ENCRYPT_] = (int)msg.encrypt();
	msgValue[JSON_CONTENT_] = msg.scontent();
	msgValue[JSON_EXTEND_] = msg.extend();
	
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	string key = getMsgCacheKey(radioId);
	int nTick = 0;
	long ret_value = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->rpush(key, msgValue.toStyledString(), ret_value)) && nTick < 3)
		nTick++;
	if(nTick >= 3)
	{
		ErrLog("isert channel msg to redis fail! useId:%s, msgId:%s, radioId:%s", msg.sfromid().c_str(), msg.smsgid().c_str(), msg.sradioid().c_str());
		return false;
	}
	conn->llen(key, ret_value);
	if(ret_value > m_nMsgCacheSzie)
	{
		string value = "";
		conn->lpop(key, value);
	}
	return true;
}

bool CRedisMgr::removeMsgFromMsgQueue(const std::string& radioId, const std::string& value)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	string key = getMsgCacheKey(radioId);
	int nTick = 0;
	long ret_value = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->lrem(key, value, ret_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code ||nTick >= 3)
	{
		ErrLog("delete channel msg to redis fail! radioId:%s value:%s", radioId.c_str(), value.c_str());
		return false;
	}
	return true;
}


bool CRedisMgr::getChannelMemberList(const std::string& url, const std::string& Secret,const std::string& strRadioId, std::vector<string>& vecMember, string& strCode)
{
	std::string strReponse = "";

	string str = Secret + "app_id2radio_id" + strRadioId + Secret;
	std::string strMD5 = MD5Str(str);
	std::transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
	string strPost = "app_id=2&radio_id=" + strRadioId + "&sign=" + strMD5;

	CHttpClient httpClient;
	CURLcode code = CURLE_OK;
	code = httpClient.Post(url, strPost, strReponse);
	if(CURLE_OK != code || strReponse.empty())
	{
		ErrLog("http request fail! radio_id=%s, code=%d", strRadioId.c_str(), code);
		return false;
	}
	DbgLog("url:%s post:%s response:%s", url.c_str(), strPost.c_str(), strReponse.c_str());
	if(!parseChannelMemberList(strRadioId, strReponse, vecMember, strCode))
		return false;
	
	return true;
}

bool CRedisMgr::parseChannelMemberList(const std::string& strRadioId,  const std::string& strData, std::vector<string>& vecMember, string& strCode)
{
	Json::Value valRepns;
	Json::Reader readRep;
	if(!readRep.parse(strData, valRepns))
		return false;
	strCode = valRepns["code"].asString();
	if(strCode != "200")
	{
		ErrLog("htpp response fail! code=%s erro=%s radio_id=%s response=%s", strCode.c_str(), valRepns["msg"].asString().c_str(), strRadioId.c_str(), strData.c_str());
		return false;
	}

	int  nSize = valRepns["data"].size();
	for(int i = 0; i < nSize; ++i)
	{
		vecMember.emplace_back(valRepns["data"][i].asString());
	}
	return true;
}


string CRedisMgr::getMsgCacheKey(const string& radioId)
{
	char key[128];
	snprintf(key, sizeof(key), "%s%s", PRE_CHNN_.c_str(), radioId.c_str());
	return key;
}

bool CRedisMgr::getChannelUser(const std::string& radioId, std::vector<std::string>& vecUser)
{
	std::string key = getChannelUserKey(radioId);
	return getSetMembers(key, vecUser);
}

bool CRedisMgr::getChannelOfflineUser(const std::string& radioId, std::vector<std::string>& vecUser)
{
	std::string key = getChannelOfflineUserKey(radioId);
	return getSetMembers(key, vecUser);
}

bool CRedisMgr::getUserChannel(const std::string& userId, std::vector<std::string>& vecRadio)
{
	std::string key = getUserChannelKey(userId);
	return getSortedSetMember(key, vecRadio);
}

bool CRedisMgr::getChannelAdmin(const std::string& radioId, std::vector<std::string>& vecAdmin)
{
	std::string key = getChannelAdminKey(radioId);
	return getSetMembers(key, vecAdmin);
}


bool CRedisMgr::addUserToChannel(const std::string& radioId, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelUserKey(radioId);
	return addMemberToSet(conn, key, userId);
}

bool CRedisMgr::addUserToChannel(const std::string& radioId, const std::vector<std::string>& vecUser)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelUserKey(radioId);
	return addMemberListToSet(conn, key, vecUser);
}


bool CRedisMgr::addUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	for(auto& itor : vecRadio)
	{
		std::string key = getChannelUserKey(itor);
		if(!addMemberToSet(conn, key, userId))
			continue;
	}
	return true;
}

bool CRedisMgr::removeUserToChannel(const std::string& radioId, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelUserKey(radioId);
	return removeMemberFromSet(conn, key, userId);
}

bool CRedisMgr::removeUserToChannel(const std::vector<std::string>& vecRadio, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	for(auto& itor : vecRadio)
	{
		std::string key = getChannelUserKey(itor);
		if(!removeMemberFromSet(conn, key, userId))
			continue;
	}
	return true;
}

bool CRedisMgr::removeUserToChannel(const std::string& radioId, const std::vector<std::string>& vecUser)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelUserKey(radioId);
	return removeMemberListFromSet(conn, key, vecUser);
}

bool CRedisMgr::moveOnlineUserToChannel(const std::string& radioId, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string srcKey = getChannelOfflineUserKey(radioId);
	std::string destKey = getChannelUserKey(radioId);
	
	return moveSetMember(conn, srcKey, destKey, userId);
}

bool CRedisMgr::deleteChannelUserKey(const std::string& radioId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelUserKey(radioId);
	return delKey(conn, key);
}



bool CRedisMgr::addChannelToUser(const std::string& userId, const std::map<std::string, int>& mapRadio)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getUserChannelKey(userId);
	return addMemberListToSortedSet(conn, key, mapRadio);
}


bool CRedisMgr::addChannelToUser(const std::string& userId, const std::string& radioId, int nStatus)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getUserChannelKey(userId);
	return addMemberToSortedSet(conn, key, radioId, nStatus);
}

bool CRedisMgr::addChannelToUser(const std::vector<std::string>& vecUser, const std::string& radioId, int nStatus)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	for(auto& itor : vecUser)
	{
		std::string key = getUserChannelKey(itor);
		if(!addMemberToSortedSet(conn, key, radioId, nStatus))
			continue;
	}
	return true;
}


bool CRedisMgr::removeChannelToUser(const std::string& userId, const std::vector<std::string>& vecRadio)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getUserChannelKey(userId);
	return removeMemberListFromSortedSet(conn, key, vecRadio);
}

bool CRedisMgr::removeChannelToUser(const std::string& userId, const std::string& radioId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getUserChannelKey(userId);
	return removeMemberFromSortedSet(conn, key, radioId);
}

bool CRedisMgr::removeChannelToUser(const std::vector<std::string>& vecUser, const std::string& radioId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	for(auto& itor : vecUser)
	{
		std::string key = getUserChannelKey(itor);
		if(!removeMemberFromSortedSet(conn, key, radioId))
			continue;
	}
	return true;
}

bool CRedisMgr::getChannelSetOnStatus(const std::string& userId, const std::string& radioId, int nStatus)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getUserChannelKey(userId);
	return getSortedSetMemberSocrt(conn, key, radioId, nStatus);
}




bool CRedisMgr::addChannelAdmin(const std::string& radioId, const std::string& admin)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelAdminKey(radioId);
	return addMemberToSet(conn, key, admin);
}

bool CRedisMgr::addChannelAdminList(const std::string& radioId, const std::vector<std::string>& vecAdmin)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelAdminKey(radioId);
	return addMemberListToSet(conn, key, vecAdmin);
}

bool CRedisMgr::removeChannelAdmin(const std::string& radioId, const std::string& admin)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelAdminKey(radioId);
	return removeMemberFromSet(conn, key, admin);
}

bool CRedisMgr::removeChannelAdminList(const std::string& radioId, const std::vector<std::string>& vecAdmin)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelAdminKey(radioId);
	return removeMemberListFromSet(conn, key, vecAdmin);
}



bool CRedisMgr::addOfflineUserToChannel(const std::string& radioId, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelOfflineUserKey(radioId);
	return addMemberToSet(conn, key, userId);
}

bool CRedisMgr::moveOfflineUserToChannel(const std::string& radioId, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string srcKey = getChannelUserKey(radioId);
	std::string destKey = getChannelOfflineUserKey(radioId);
	
	return moveSetMember(conn, srcKey, destKey, userId);
}

bool CRedisMgr::removeOfflineUserToChannel(const std::string& radioId, const std::string& userId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelOfflineUserKey(radioId);
	return removeMemberFromSet(conn, key, userId);
}

bool CRedisMgr::removeOfflineUserToChannel(const std::string& radioId, const std::vector<std::string>& vecUser)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelOfflineUserKey(radioId);
	return removeMemberListFromSet(conn, key, vecUser);
}


bool CRedisMgr::deleteChannelOfflineUserKey(const std::string& radioId)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelOfflineUserKey(radioId);
	return delKey(conn, key);
}




bool CRedisMgr::addMemberToSet(CacheConn* conn, const std::string& key, const std::string& member)
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

bool CRedisMgr::addMemberListToSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members)
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

bool CRedisMgr::removeMemberFromSet(CacheConn* conn, const std::string& key, const std::string& member)
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

bool CRedisMgr::removeMemberListFromSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members)
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

bool CRedisMgr::moveSetMember(CacheConn* conn, const std::string& srcKey, const std::string& destKey, const std::string& member)
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

bool CRedisMgr::delKey(CacheConn* conn, const std::string& key)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->del(key)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisMgr::getSetMembers(const std::string& key, std::vector<std::string>& vecMember)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = cache.getCacheConn()->smembers(key, vecMember)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}



bool CRedisMgr::getSortedSetMember(const std::string& key, std::vector<std::string>& vecMember)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = cache.getCacheConn()->zrange(key, vecMember)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisMgr::getSortedSetMember(const std::string& key, std::map<std::string, int>& mapMember)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = cache.getCacheConn()->zrange(key, mapMember)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisMgr::addMemberToSortedSet(CacheConn* conn, const std::string& key, const std::string& member, int nSocre)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->zadd(key, member, nSocre)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisMgr::addMemberListToSortedSet(CacheConn* conn, const std::string& key, const std::map<std::string, int>& mapMember)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->zadd(key, mapMember)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisMgr::removeMemberFromSortedSet(CacheConn* conn, const std::string& key, const std::string& member)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->zrem(key, member)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisMgr::removeMemberListFromSortedSet(CacheConn* conn, const std::string& key, const std::vector<std::string>& members)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->zrem(key, members)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisMgr::getSortedSetMemberSocrt(CacheConn* conn, const std::string& key, const std::string& member, int& nSocre)
{
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->zscore(key, member, nSocre)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}






string CRedisMgr::getUserChannelKey(const std::string& userId)
{
	char key[128];
	snprintf(key, sizeof(key), "%s%s", PRE_USER_CHNN_.c_str(), userId.c_str());
	return key;
}

string CRedisMgr::getChannelUserKey(const std::string& radioId)
{
	char key[128];
	snprintf(key, sizeof(key), "%s%s", PRE_CHNN_USER_.c_str(), radioId.c_str());
	return key;
}

string CRedisMgr::getChannelAdminKey(const std::string& radioId)
{
	char key[128];
	snprintf(key, sizeof(key), "%s%s", PRE_CHNN_ADMIN_.c_str(), radioId.c_str());
	return key;
}

string CRedisMgr::getChannelOfflineUserKey(const std::string& radioId)
{
	char key[128];
	snprintf(key, sizeof(key), "%s%s", PRE_CHNN_OFFLINE_USER_.c_str(), radioId.c_str());
	return key;
}



bool CRedisMgr::getChannelInfo(const std::string& radioId, CHANNEL_INIFO_& chnnInfo)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelInfoKey(radioId);
	std::vector<string> fields;
	fields.push_back(CHNN_INFO_STATUS);
	fields.push_back(CHNN_INFO_SPEAK);
	std::vector<string> res_value;
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->hmget(key, fields, res_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	chnnInfo.radioId = radioId;
	chnnInfo.status = (CHNN_STATUS_)atoi(res_value[0].c_str());
	chnnInfo.unspeak = atoi(res_value[1].c_str());
	return true;
}

bool CRedisMgr::setChannelInfo(const CHANNEL_INIFO_& chnnInfo)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelInfoKey(chnnInfo.radioId);

	std::map<std::string, std::string> fields;
	fields[CHNN_INFO_STATUS] = std::to_string(chnnInfo.status);
	fields[CHNN_INFO_SPEAK] = std::to_string(chnnInfo.unspeak);
	std::string ret_value = "";
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->hmset(key, fields, ret_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisMgr::setChannelInfoList(const std::vector<CHANNEL_INIFO_>& vecChnnInfo)
{

	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = "";
	std::map<std::string, std::string> fields;
	std::string ret_value = "";
	for(auto& itor : vecChnnInfo)
	{
		key = getChannelInfoKey(itor.radioId);
		fields[CHNN_INFO_STATUS] = std::to_string(itor.status);
		fields[CHNN_INFO_SPEAK] = std::to_string(itor.unspeak);
		int nTick = 0;
		REDIS_OPER_CODE code = REDIS_OPER_OK;
		while(REDIS_OPER_DISCONN == (code = conn->hmset(key, fields, ret_value)) && nTick < 3)
			nTick++;
		fields.clear();
		if(REDIS_OPER_OK != code || nTick >= 3)
			continue;
	}
	return true;
}

bool CRedisMgr::setChannelStatus(const std::string& radioId, CHNN_STATUS_ status)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelInfoKey(radioId);
	long ret_value = 0;
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->hset(key, CHNN_INFO_STATUS, std::to_string(status), ret_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}

bool CRedisMgr::setChannelSpeak(const std::string& radioId, int unSpeak)
{
	CAutoCache cache(CACHE_NAME_CHNL);
	CacheConn* conn = cache.getCacheConn();
	std::string key = getChannelInfoKey(radioId);
	long ret_value = 0;
	int nTick = 0;
	REDIS_OPER_CODE code = REDIS_OPER_OK;
	while(REDIS_OPER_DISCONN == (code = conn->hset(key, CHNN_INFO_SPEAK, std::to_string(unSpeak), ret_value)) && nTick < 3)
		nTick++;
	if(REDIS_OPER_OK != code || nTick >= 3)
		return false;
	return true;
}


std::string CRedisMgr::getChannelInfoKey(const std::string& radioId)
{
	char key[128];
	snprintf(key, sizeof(key), "%s%s", PRE_CHNN_INFO_.c_str(), radioId.c_str());
	return key;
}

bool CRedisMgr::getChannelInfoList(const std::string& url, const std::string& Secret, std::vector<CHANNEL_INIFO_>& vecChnnInfo, string& strCode)
{
	std::string strReponse = "";

	string str = Secret + "app_id2" + Secret;
	std::string strMD5 = MD5Str(str);
	std::transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
	string strPost = "app_id=2&sign=" + strMD5;

	CHttpClient httpClient;
	CURLcode code = CURLE_OK;
	code = httpClient.Post(url, strPost, strReponse);
	if(CURLE_OK != code || strReponse.empty())
	{
		ErrLog("http request fail! url=%s, code=%d", url.c_str(), code);
		return false;
	}
	DbgLog("url:%s post:%s response:%s", url.c_str(), strPost.c_str(), strReponse.c_str());
	if(!parseChannelInfoList(strReponse, vecChnnInfo, strCode))
		return false;
	
	return true;
}

bool CRedisMgr::parseChannelInfoList(const std::string& strData, std::vector<CHANNEL_INIFO_>& vecChnnInfo, string& strCode)
{
	Json::Value valRepns;
	Json::Reader readRep;
	if(!readRep.parse(strData, valRepns))
		return false;
	strCode = valRepns["code"].asString();
	if(strCode != "200")
	{
		ErrLog("htpp response fail! code=%s erro=%s  response=%s", strCode.c_str(), valRepns["msg"].asString().c_str(), strData.c_str());
		return false;
	}

	std::vector<std::string> vecAdmin;
	CHANNEL_INIFO_ chnnInfo;
	int  nSize = valRepns["data"].size();
	for(int i = 0; i < nSize; ++i)
	{
		chnnInfo.radioId = valRepns["data"][i]["id"].asString().c_str();
		chnnInfo.status = (CHNN_STATUS_)atoi(valRepns["data"][i]["status"].asString().c_str());
		chnnInfo.unspeak = atoi(valRepns["data"][i]["is_unspeeking_all"].asString().c_str());
		vecChnnInfo.emplace_back(chnnInfo);
		
		vecAdmin.clear();
		int nCount = valRepns["data"][i]["admin"].size();
		for(int j = 0; j < nCount; ++j)
		{
			vecAdmin.emplace_back(valRepns["data"][i]["admin"][j].asString());
		}
		addChannelAdminList(chnnInfo.radioId, vecAdmin);
	}
	return true;
}






