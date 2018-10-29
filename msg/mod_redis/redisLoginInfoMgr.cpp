/******************************************************************************
Filename: redisLoginInfoMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description: 
******************************************************************************/
#include "im_loginInfo.h"
#include "redisLoginInfoMgr.h"
#include "redisPool.h"
#include "util.h"

#define  LOGININFO_FIELD_CMIP		"cmIPAddr"
#define  LOGININFO_FIELD_CMPORT		"cmIPPort"
#define  LOGININFO_FIELD_ROLE		"role"
#define  LOGININFO_FIELD_STATUS		"status"
#define  LOGININFO_FIELD_DEVTYPE	"deviceType"
#define  LOGININFO_FIELD_DEVTOKEN	"deviceToken"
#define  LOGININFO_FIELD_DEVVOIPTOKEN	"deviceVoipToken"
#define  LOGININFO_FIELD_CALLSTATE	"callState"
#define  DEVICEID_FIELD_SUBSCRIBER	"subscriber"

CLoginInfoMgr::CLoginInfoMgr()
{

}

CLoginInfoMgr::~CLoginInfoMgr()
{

}

std::shared_ptr<CLoginInfo> CLoginInfoMgr::GetLoginInfo(const std::string& userId)
{
	std::string cmUserId = "CM_" + userId;
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::vector<const char*> fields;
	fields.push_back(LOGININFO_FIELD_ROLE);
	fields.push_back(LOGININFO_FIELD_STATUS);
	fields.push_back(LOGININFO_FIELD_CMIP);
	fields.push_back(LOGININFO_FIELD_CMPORT);
	fields.push_back(LOGININFO_FIELD_DEVTYPE);
	fields.push_back(LOGININFO_FIELD_DEVTOKEN);
	fields.push_back(LOGININFO_FIELD_DEVVOIPTOKEN);
	fields.push_back(LOGININFO_FIELD_CALLSTATE);

	std::vector<acl::string> result;
	if (conn.hmget(cmUserId.c_str(), fields, &result))
	{
		if (!result[1].empty() && !result[2].empty())
		{
			DbgLog("user %s Login info ,status = %s, cmip = %s, cmport = %s, devtype = %s, devtoken = %s, devvoipToken=%s",
				userId.c_str(), result[1].c_str(), result[2].c_str(), result[3].c_str(), result[4].c_str(), result[5].c_str(), result[6].c_str());
			return std::shared_ptr<CLoginInfo>(new CLoginInfo(result[0].c_str(), result[1].c_str(), result[2].c_str(),
				result[3].c_str(), atoi(result[4].c_str()), result[5].c_str(), result[6].c_str(), USER_CALL_STATE(result[7].empty() ? 0 : atoi(result[7].c_str()))));
		}
	}
	DbgLog("user %s not login yet or not exist", cmUserId.c_str());
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

std::string CLoginInfoMgr::getDeviceLastUserID(const std::string& deviceID)
{
	std::string deviceIDKey = "CMDT_" + deviceID;
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


