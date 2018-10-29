#include "redisUserGrpCfgMgr.h"
#include "redisPool.h"
#include "util.h"

#define  USERGRPCFG_FIELD_MSGSOUNDOFF		"_sound_off"
#define  USERGRPCFG_FIELD_HIDEFLAG			"_hide_flag"

#define  USERGRPCFG_FIELD_LENGTH			64
CUserGrpCfgMgr::CUserGrpCfgMgr()
{

}

CUserGrpCfgMgr::~CUserGrpCfgMgr()
{

}

static const std::shared_ptr<CUserGrpCfg> pDefaultUserGrpCfg = std::shared_ptr<CUserGrpCfg>(new CUserGrpCfg(0,0));
const std::shared_ptr<CUserGrpCfg> CUserGrpCfgMgr::GetUserGrpCfg(const std::string& userId, const std::string& grpId)
{
	std::string cmUserId = "GROUP_USER_" + grpId;
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::vector<const char*> fields;

	char userGrpMsgSoundOff[USERGRPCFG_FIELD_LENGTH];
	snprintf(userGrpMsgSoundOff, USERGRPCFG_FIELD_LENGTH, "%s%s", userId.c_str(), USERGRPCFG_FIELD_MSGSOUNDOFF);

	char userGrpHideFlag[USERGRPCFG_FIELD_LENGTH];
	snprintf(userGrpHideFlag, USERGRPCFG_FIELD_LENGTH, "%s%s", userId.c_str(), USERGRPCFG_FIELD_HIDEFLAG);

	fields.push_back(userGrpMsgSoundOff);
	fields.push_back(userGrpHideFlag);

	std::vector<acl::string> result;
	if (conn.hmget(cmUserId.c_str(), fields, &result))
	{
		int userGrpMsgSoundOff = 0;
		int userGrpHideFlag = 0;
		if (!result[0].empty())
		{
			userGrpMsgSoundOff = atoi(result[0].c_str());
		}
		if (!result[1].empty())
		{
			userGrpHideFlag = atoi(result[1].c_str());
		}
		return  std::shared_ptr<CUserGrpCfg>(new CUserGrpCfg(userGrpMsgSoundOff, userGrpHideFlag));
	}
	return pDefaultUserGrpCfg;
}

