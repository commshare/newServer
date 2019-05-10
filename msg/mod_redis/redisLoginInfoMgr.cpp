/******************************************************************************
Filename: redisLoginInfoMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description: 
******************************************************************************/
#include "im_loginInfo.h"
#include "redisLoginInfoMgr.h"
#include "redisPool.h"
#include "util.h"

#define  LOGININFO_FIELD_CMIP		"cm_ip"
#define  LOGININFO_FIELD_CMPORT		"cm_port"
#define  LOGININFO_FIELD_STATUS		"status"
#define  LOGININFO_FIELD_CALLSTATE	"call_state"
#define  DEVICEID_FIELD_SUBSCRIBER	"subscriber"
#define  DEVICEID_FIELD_DEVICETOKEN	"device_token"

CLoginInfoMgr::CLoginInfoMgr()
{

}

CLoginInfoMgr::~CLoginInfoMgr()
{

}

std::shared_ptr<CLoginInfo> CLoginInfoMgr::GetLoginInfo(const std::string& userId)
{
	if(userId.empty())
	{
		WarnLog("user id is empty!");
		return std::shared_ptr<CLoginInfo>(nullptr);
	}
	
	std::string cmUserId = "CM_" + userId;
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::vector<const char*> fields;
	fields.push_back(LOGININFO_FIELD_STATUS);
	fields.push_back(LOGININFO_FIELD_CMIP);
	fields.push_back(LOGININFO_FIELD_CMPORT);
	fields.push_back(LOGININFO_FIELD_CALLSTATE);
	fields.push_back(DEVICEID_FIELD_DEVICETOKEN);

	uint64_t now = getCurrentTime_usec();
	std::vector<acl::string> result;
	if (conn.hmget(cmUserId.c_str(), fields, &result))
	{
		if (!result[1].empty() && !result[2].empty())
		{

			DbgLog("===========getlogininfo end at %lu, used time=%lu===============", getCurrentTime_usec(), getCurrentTime_usec() - now);
			DbgLog("user %s Login info ,status = %s, cmip = %s, cmport = %s device_token=%s", userId.c_str(), result[0].c_str(), result[1].c_str(), result[2].c_str(), result[4].c_str());
			return std::shared_ptr<CLoginInfo>(new CLoginInfo( result[0].c_str(), result[1].c_str(), result[2].c_str(), result[4].c_str(), USER_CALL_STATE(result[3].empty() ? 0 : atoi(result[3].c_str()))));
		}
	}
	DbgLog("user %s not login yet or not exist", cmUserId.c_str()); 
	DbgLog("===========getlogininfo end at %lu, used time=%lu===============", getCurrentTime_usec(), getCurrentTime_usec() - now);
	return std::shared_ptr<CLoginInfo>(NULL);
}

bool CLoginInfoMgr::UpdateCallBusyState(const std::string& userId, uint16_t callState)
{
	std::string cmUserId = "CM_" + userId;
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::map<string, const char*> hashVals;
	char state_str[12] = { '\0' };
	sprintf(state_str, "%d", callState);
	hashVals.insert(std::pair<string, const char*>(LOGININFO_FIELD_CALLSTATE, state_str));
	return conn.hmset(cmUserId, hashVals);
}

std::string CLoginInfoMgr::getDeviceLastUserID(const std::string& deviceToken)
{
	std::string deviceIDKey = "CMDT_" + deviceToken;
	std::string userID = "";
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::vector<const char*> fields;
	fields.push_back(DEVICEID_FIELD_SUBSCRIBER);

	std::vector<acl::string> result;
	if (conn.hmget(deviceIDKey.c_str(), fields, &result))
	{
		userID = result[0].c_str();
	}

	return userID;
	
}


