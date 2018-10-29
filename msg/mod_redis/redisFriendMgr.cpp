/******************************************************************************
Filename: redisFriendMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/05
Description: 
******************************************************************************/
#include "im_friend.h"
#include "redisFriendMgr.h"
#include "redisPool.h"

#define  FRIEND_FIELD_STATE				"state"
#define  FRIEND_FIELD_SENDMSGREAD		"sendMsgRead"
#define  FRIEND_FIELD_SOUNDOFF			"sound_off"
#define  FRIEND_FIELD_HIDEFALG			"hide_flag"

CReidsFriendMgr::CReidsFriendMgr()
{

}

CReidsFriendMgr::~CReidsFriendMgr()
{

}

std::vector<CFriendRelation> CReidsFriendMgr::GetFriendRelation(const string& userId, const string& friendId)
{
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	string key = generateKey(userId, friendId);
	std::vector<const char*> fields;
	fields.push_back(FRIEND_FIELD_STATE);
	fields.push_back(FRIEND_FIELD_SENDMSGREAD); 
	fields.push_back(FRIEND_FIELD_SOUNDOFF);
	fields.push_back(FRIEND_FIELD_HIDEFALG);

	std::vector<CFriendRelation> friendShips;

	std::vector<acl::string> result;
	//get useId--->friendId relation
	if (conn.hmget(key.c_str(), fields, &result))
	{
		if (!result[0].empty())
		{
			uint16_t sendMsgRead = result[1].empty() ? 1 : atoi(result[1].c_str());
			uint16_t soundOff = result[2].empty() ? 0 : atoi(result[2].c_str());
			uint16_t hideFlag = result[3].empty() ? 0 : atoi(result[3].c_str());
			friendShips.push_back(CFriendRelation(userId, friendId, atoi(result[0].c_str()), sendMsgRead, soundOff, hideFlag));
		}
	}


	//get friendId--->useId relation
	key = generateKey(friendId, userId);
	result.clear();
	if (conn.hmget(key.c_str(), fields, &result))
	{
		if (!result[0].empty())
		{
			uint16_t sendMsgRead = result[1].empty() ? 1 : atoi(result[1].c_str());
			uint16_t soundOff = result[2].empty() ? 0 : atoi(result[2].c_str());
			uint16_t hideFlag = result[3].empty() ? 0 : atoi(result[3].c_str());
			friendShips.push_back(CFriendRelation(friendId, userId, atoi(result[0].c_str()), sendMsgRead, soundOff, hideFlag));
		}
	}

	return friendShips;
}

bool CReidsFriendMgr::InsertFriendRelation(const CFriendRelation& friendrelation)
{
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::map<string, const char*> hashVals;
	char state[12] = { '\0' };
	sprintf(state, "%d", friendrelation.GetState());
	char sendMsgRead[12] = { '\0' };
	sprintf(sendMsgRead, "%d", friendrelation.GetNotifyMsgDelivered());
	char soundOff[12] = { '\0' };
	sprintf(soundOff, "%d", friendrelation.IsNoInterruption());
	char hideFlag[12] = { '\0' };
	sprintf(hideFlag, "%d", friendrelation.IsHidenModel());
	hashVals.insert(std::pair<string, const char*>(FRIEND_FIELD_STATE,state));
	hashVals.insert(std::pair<string, const char*>(FRIEND_FIELD_SENDMSGREAD, sendMsgRead));
	hashVals.insert(std::pair<string, const char*>(FRIEND_FIELD_SOUNDOFF, soundOff));
	hashVals.insert(std::pair<string, const char*>(FRIEND_FIELD_HIDEFALG, hideFlag));
	return conn.hmset(generateKey(friendrelation.GetUserId(), friendrelation.GetFriendId()), hashVals);
}

bool CReidsFriendMgr::UpdateFriendShipState(const string& userId, const string& friendId, int state)
{
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::map<string, const char*> hashVals;
	char state_str[12] = { '\0' };
	sprintf(state_str, "%d", state);
	hashVals.insert(std::pair<string, const char*>(FRIEND_FIELD_STATE, state_str));
	return conn.hmset(generateKey(userId, friendId), hashVals);
}

bool CReidsFriendMgr::RemoveFriendRelation(const string& userId, const string& friendId)
{
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	return conn.hdel(generateKey(userId, friendId)) >= 0;
}

string CReidsFriendMgr::generateKey(const string& userId, const string& friendId)
{
	return "FR_" + userId + ":" + friendId;
}
