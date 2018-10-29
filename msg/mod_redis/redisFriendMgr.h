/******************************************************************************
Filename: redisFriendMgr.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/05
Description: 
******************************************************************************/
#ifndef __REDISFRIENDMGR_H__
#define __REDISFRIENDMGR_H__

#include <string>
#include <vector>
using std::string;

class CFriendRelation;

class CReidsFriendMgr
{
public:
	CReidsFriendMgr();
	~CReidsFriendMgr();
	static std::vector<CFriendRelation> GetFriendRelation(const string& userId, const string& friendId);
	static bool InsertFriendRelation(const CFriendRelation& friendrelation);
	static bool UpdateFriendShipState(const string& userId, const string& friendId, int state);
	static bool RemoveFriendRelation(const string& userId, const string& friendId);
private:
	static string generateKey(const string& userId, const string& friendId);
};

#endif // __REDISFRIENDMGR_H__
