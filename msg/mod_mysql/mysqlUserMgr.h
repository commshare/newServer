/******************************************************************************
Filename: mysqlUserMgr.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/14
Description: 
******************************************************************************/
#ifndef __MYSQLUSERMGR_H__
#define __MYSQLUSERMGR_H__

#include <string>
#include <memory>
#include "im_userInfo.h"

namespace acl
{
	class db_row;
}

class CMysqlUsrMgr
{
public:
	CMysqlUsrMgr();
	~CMysqlUsrMgr();

	//查询userId的好友关系
	//friendID为空则表示所有的好友
	//friendShips:返回参数，返回查询结果
	static std::shared_ptr<CUserInfo> GetUserInfo(const std::string& userId) throw(int);
private:
	static std::shared_ptr<CUserInfo> rowToUserInfo(const acl::db_row* pRow);
};
#endif // __MYSQLUSERMGR_H__

