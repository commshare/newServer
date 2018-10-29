/******************************************************************************
Filename: mysqlFriendMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/16
Description: 
******************************************************************************/
#include "mysqlUserMgr.h"
#include "mysqlPool.h"
#include "acl_cpp/lib_acl.hpp"
#include "im_userInfo.h"

#define USERINFO_TABLE_NAME				"`users`"
#define USERINFO_USERID					"`userid`"
#define USERINFO_PHOTO					"`avatar`"
#define USERINFO_DEV_TOKEN				"`last_device_token`"
#define USERINFO_DEV_TYPE				"`last_device_type`"
#define USERINFO_LASTLOGINTIME			"`last_login_time`"


CMysqlUsrMgr::CMysqlUsrMgr()
{

}

CMysqlUsrMgr::~CMysqlUsrMgr()
{

}

std::shared_ptr<CUserInfo> CMysqlUsrMgr::rowToUserInfo(const acl::db_row* pRow)
{
	//ASSERT(pRow != NULL);
	const acl::db_row& row = *pRow;

	string userId(row[(size_t)0]);
	string photoUrl(row[1]?row[1]:"");
	string devToken(row[2]?row[2]:"");
	int devType = row[3]? atoi(row[3]) : 0;
	unsigned long loginTime = row[4] ? atoi(row[4]) : 0;

	return std::shared_ptr<CUserInfo>(new CUserInfo(userId, photoUrl, devToken, devType, loginTime));
}

std::shared_ptr<CUserInfo> CMysqlUsrMgr::GetUserInfo(const string& userId) throw(int)
{
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		ErrLog("get mysql connection for msgSvr_mysql_slave failed");
		throw - 1;
	}

	/* generate query string */
	char queryStr[512] = { '\0' };
	int queryStrLen = 0;
	queryStrLen = sprintf(queryStr + queryStrLen, "SELECT " USERINFO_USERID", "
		USERINFO_PHOTO", "
		USERINFO_DEV_TOKEN", "
		USERINFO_DEV_TYPE", "
		USERINFO_LASTLOGINTIME
		" FROM " USERINFO_TABLE_NAME" WHERE ("
		USERINFO_USERID"= :" USERINFO_USERID");");

	acl::query query;
	query.create_sql(queryStr)
		.set_parameter(USERINFO_USERID, userId.c_str());

	//DbgLog("%s", query.to_string().c_str());	
	if (pConn->exec_select(query) == false)
	{
		//ErrLog("select sql (%s) error", query.to_string().c_str());
		throw (-1);
	}

	//get result
	const acl::db_rows* result = pConn->get_result();
	if (result)
	{
		const acl::db_row* pRow = result->get_rows()[0];
		if (pRow)
		{
			return rowToUserInfo(pRow);
		}
	}

	return std::shared_ptr<CUserInfo>(NULL);
}

