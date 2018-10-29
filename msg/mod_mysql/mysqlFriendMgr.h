/******************************************************************************
Filename: mysqlFriendMgr.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/16
Description: 好友关系
******************************************************************************/
#ifndef __FRIENDRELATION_H__
#define __FRIENDRELATION_H__

#include <vector>
#include <string>
#include "acl_cpp/lib_acl.hpp"
#include "im_friend.h"

class CMysqlFriendMgr
{
public:
	CMysqlFriendMgr();
	~CMysqlFriendMgr();
	static bool InsertFriendShip(const CFriendRelation& friendShip);
	static bool UpdateFriendShip(const CFriendRelation& friendShip);
	//static bool UpdateMemoName(const CFriendRelation& friendShip);

	static bool UpdateFriendShipState(const std::string& userId, const std::string& friendId, int state);

	static bool DelFriendShip(const std::string& userId, const std::string& friendId);

	//查询userId的好友关系
	//friendShips:返回参数，返回查询结果（a-b,b-a）
	static std::vector<CFriendRelation> GetFriendShips(const std::string& userId, const std::string& friendId) throw(int);
	static std::vector<CFriendRelation> GetFriendShips(const std::string& userId) throw(int);
private:
	static void FriendValToQuery(const CFriendRelation& rel, acl::query& query);
	static CFriendRelation rowToFriendShip(const acl::db_row* pRow);
};

#endif