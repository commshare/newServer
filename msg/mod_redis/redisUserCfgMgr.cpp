#include "redisUserCfgMgr.h"
#include "redisPool.h"
#include "util.h"

#define  USERCFG_FIELD_MSGSOUNDOFF		"msg_off"
#define  USERCFG_FIELD_HIDEMSGSOUNDON	"set_ordinary_3"
#define  USERCFG_FIELD_CALLMSGOFF		"call_msg_off"


CUserCfgMgr::CUserCfgMgr()
{

}

CUserCfgMgr::~CUserCfgMgr()
{

}

static const std::shared_ptr<CUserCfg> pDefaultUserCfg = std::shared_ptr<CUserCfg>(new CUserCfg(0,0));
const std::shared_ptr<CUserCfg> CUserCfgMgr::GetUserCfg(const std::string& userId)
{
	std::string cmUserId = "USER_" + userId;
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::vector<const char*> fields;
	fields.push_back(USERCFG_FIELD_MSGSOUNDOFF);
	fields.push_back(USERCFG_FIELD_HIDEMSGSOUNDON);
	fields.push_back(USERCFG_FIELD_CALLMSGOFF);

	std::vector<acl::string> result;
	if (conn.hmget(cmUserId.c_str(), fields, &result))
	{
		if (!result[0].empty() && !result[1].empty())
		{
			//DbgLog("user %s Cfg info ,msg_off = %s, hide_msg_sound_on = %s",
			//	userId.c_str(), result[0].c_str(), result[1].c_str());
			return std::shared_ptr<CUserCfg>(new CUserCfg(atoi(result[0].c_str()), atoi(result[1].c_str()), atoi(result[2].c_str())));
		}
	}
	return pDefaultUserCfg;
}

