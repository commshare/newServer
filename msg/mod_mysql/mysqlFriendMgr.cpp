/******************************************************************************
Filename: mysqlFriendMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/16
Description: 
******************************************************************************/
#include "mysqlFriendMgr.h"
#include "mysqlPool.h"
#include "acl_cpp/lib_acl.hpp"

#define FRIENDSHIP_TABLE_NAME				"`friend`"
#define FRIENDSHIP_FRIENDD_USERID			"`userId`"
#define FRIENDSHIP_FRIENDD_FRIENDID			"`friendId`"

#define FRIENDSHIP_FRIENDD_USERID2			"userId2"
#define FRIENDSHIP_FRIENDD_FRIENDID2		"friendId2"

#define FRIENDSHIP_FRIENDD_MEMONAME			"`memoName`"
#define FRIENDSHIP_FRIENDD_STATE			"`state`"
#define FRIENDSHIP_FRIENDD_STATE2			"`state2`"
#define FRIENDSHIP_FRIENDD_SOUNDOFF			"`sound_off`"
#define FRIENDSHIP_FRIENDD_HIDEFLAG			"`hide_flag`"
#define FRIENDSHIP_FRIENDD_SENDMSGREAD		"`sendMsgRead`"
#define FRIENDSHIP_FRIENDD_DESC				"`desc`"
#define FRIENDSHIP_FRIENDD_PACKETID			"`packetId`"
#define FRIENDSHIP_FRIENDD_SELFINTRODUCE	"`selfIntroduce`"
#define FRIENDSHIP_FRIENDD_CREATETIME		"`created_at`"
#define FRIENDSHIP_FRIENDD_MODIFYTIME		"`updated_at`"

#define FRIENDSHIP_FRIENDD_MOBILES		"`ext_mobiles`"
#define FRIENDSHIP_FRIENDD_EMAILS		"`ext_emails`"

CMysqlFriendMgr::CMysqlFriendMgr()
{

}

CMysqlFriendMgr::~CMysqlFriendMgr()
{

}

bool CMysqlFriendMgr::InsertFriendShip(const CFriendRelation& friendShip)
{
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	if(nullptr == pConn)
		return false;
	acl::query query;
	static const string baseSqlStr = "insert into " FRIENDSHIP_TABLE_NAME"("
		FRIENDSHIP_FRIENDD_USERID", "
		FRIENDSHIP_FRIENDD_FRIENDID", "
		FRIENDSHIP_FRIENDD_MEMONAME", "
		FRIENDSHIP_FRIENDD_DESC", "
		FRIENDSHIP_FRIENDD_PACKETID", "
		FRIENDSHIP_FRIENDD_SELFINTRODUCE", "
		FRIENDSHIP_FRIENDD_STATE","
		FRIENDSHIP_FRIENDD_CREATETIME
		") values("
		":" FRIENDSHIP_FRIENDD_USERID ", "
		":" FRIENDSHIP_FRIENDD_FRIENDID ","
		":" FRIENDSHIP_FRIENDD_MEMONAME ","
		":" FRIENDSHIP_FRIENDD_DESC ","
		":" FRIENDSHIP_FRIENDD_PACKETID ","
		":" FRIENDSHIP_FRIENDD_SELFINTRODUCE ","
		":" FRIENDSHIP_FRIENDD_STATE ","
		"now()"
		") ON DUPLICATE KEY UPDATE "
		//FRIENDSHIP_FRIENDD_USERID"=VALUES("FRIENDSHIP_FRIENDD_USERID"),"
		//FRIENDSHIP_FRIENDD_FRIENDID"=VALUES("FRIENDSHIP_FRIENDD_FRIENDID"),"
		//FRIENDSHIP_FRIENDD_MEMONAME"=VALUES("FRIENDSHIP_FRIENDD_MEMONAME"),"
		//FRIENDSHIP_FRIENDD_DESC"=VALUES("FRIENDSHIP_FRIENDD_DESC"),"
		//FRIENDSHIP_FRIENDD_PACKETID"=VALUES("FRIENDSHIP_FRIENDD_PACKETID"),"
		FRIENDSHIP_FRIENDD_STATE"=VALUES(" FRIENDSHIP_FRIENDD_STATE"),"
		FRIENDSHIP_FRIENDD_SELFINTRODUCE"=VALUES(" FRIENDSHIP_FRIENDD_SELFINTRODUCE"),"
		FRIENDSHIP_FRIENDD_MODIFYTIME"=now()";

	static const string updateMemoNameSqlStr =  "," FRIENDSHIP_FRIENDD_MEMONAME"=VALUES(" FRIENDSHIP_FRIENDD_MEMONAME")";
	static const string insertWithOutUpdateMemoNameSqlStr = baseSqlStr + ";";
	static const string insertWithUpdateMemoNameSqlStr = baseSqlStr + updateMemoNameSqlStr + ";";


	const string& sqlStr = friendShip.GetMemoName().empty() ? insertWithOutUpdateMemoNameSqlStr : insertWithUpdateMemoNameSqlStr;

	query.create_sql(sqlStr.c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_USERID, friendShip.GetUserId().c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_FRIENDID, friendShip.GetFriendId().c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_MEMONAME, friendShip.GetMemoName().c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_DESC, friendShip.GetDesc().c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_PACKETID, friendShip.GetPacketId())
		.set_parameter(FRIENDSHIP_FRIENDD_SELFINTRODUCE, friendShip.GetSelfIntroduce().c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_STATE, (int)friendShip.GetState());

	//DbgLog("exec_update %s", query.to_string().c_str());
	if (false == pConn->exec_update(query))
		return (false);
	pConn->free_result();

	if ((friendShip.GetState() & CFriendRelation::FRIEND_RELATION_STATE_DELETED) 
		|| (friendShip.GetState() & CFriendRelation::FRIEND_RELATION_STATE_BLOCKED))
	{
		acl::query queryOnDel;
		queryOnDel.create_sql("insert into " FRIENDSHIP_TABLE_NAME"("
			FRIENDSHIP_FRIENDD_USERID", "
			FRIENDSHIP_FRIENDD_FRIENDID", "
			FRIENDSHIP_FRIENDD_STATE","
			FRIENDSHIP_FRIENDD_SOUNDOFF","
			FRIENDSHIP_FRIENDD_HIDEFLAG ","
			FRIENDSHIP_FRIENDD_SENDMSGREAD ","
			FRIENDSHIP_FRIENDD_SELFINTRODUCE ","
			FRIENDSHIP_FRIENDD_MOBILES ","
			FRIENDSHIP_FRIENDD_EMAILS
			") values("
			":" FRIENDSHIP_FRIENDD_USERID ", "
			":" FRIENDSHIP_FRIENDD_FRIENDID ","
			":" FRIENDSHIP_FRIENDD_STATE ","
			":" FRIENDSHIP_FRIENDD_SOUNDOFF ","
			":" FRIENDSHIP_FRIENDD_HIDEFLAG ","
			":" FRIENDSHIP_FRIENDD_SENDMSGREAD ","
			":" FRIENDSHIP_FRIENDD_SELFINTRODUCE ","
			":" FRIENDSHIP_FRIENDD_MOBILES ","
			":" FRIENDSHIP_FRIENDD_EMAILS
			") ON DUPLICATE KEY UPDATE "
			FRIENDSHIP_FRIENDD_SOUNDOFF"=VALUES(" FRIENDSHIP_FRIENDD_SOUNDOFF"),"
			FRIENDSHIP_FRIENDD_HIDEFLAG"=VALUES(" FRIENDSHIP_FRIENDD_HIDEFLAG"),"
			FRIENDSHIP_FRIENDD_SENDMSGREAD"=VALUES(" FRIENDSHIP_FRIENDD_SENDMSGREAD"),"
			FRIENDSHIP_FRIENDD_SELFINTRODUCE"=VALUES(" FRIENDSHIP_FRIENDD_SELFINTRODUCE"),"
			FRIENDSHIP_FRIENDD_MOBILES"=VALUES(" FRIENDSHIP_FRIENDD_MOBILES"),"
			FRIENDSHIP_FRIENDD_EMAILS"=VALUES(" FRIENDSHIP_FRIENDD_EMAILS");"
			)
			.set_parameter(FRIENDSHIP_FRIENDD_USERID, friendShip.GetUserId().c_str())
			.set_parameter(FRIENDSHIP_FRIENDD_FRIENDID, friendShip.GetFriendId().c_str())
			.set_parameter(FRIENDSHIP_FRIENDD_STATE, (int)friendShip.GetState())
			.set_parameter(FRIENDSHIP_FRIENDD_SOUNDOFF, 0)
			.set_parameter(FRIENDSHIP_FRIENDD_HIDEFLAG, 0)
			.set_parameter(FRIENDSHIP_FRIENDD_SENDMSGREAD, 1)
			.set_parameter(FRIENDSHIP_FRIENDD_SELFINTRODUCE, "")
			.set_parameter(FRIENDSHIP_FRIENDD_MOBILES, "")
			.set_parameter(FRIENDSHIP_FRIENDD_EMAILS, "");

		if (false == pConn->exec_update(queryOnDel))
			return (false);
		pConn->free_result();
	}
	
	return true;
}


//bool CMysqlFriendMgr::UpdateMemoName(const CFriendRelation& friendShip)
//{
//	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
//	acl::query query;
//	query.create_sql("insert into " FRIENDSHIP_TABLE_NAME"("
//		FRIENDSHIP_FRIENDD_USERID", "
//		FRIENDSHIP_FRIENDD_FRIENDID", "
//		FRIENDSHIP_FRIENDD_STATE","
//		FRIENDSHIP_FRIENDD_MEMONAME
//		") values("
//		":" FRIENDSHIP_FRIENDD_USERID ", "
//		":" FRIENDSHIP_FRIENDD_FRIENDID ","
//		":" FRIENDSHIP_FRIENDD_STATE ","
//		":" FRIENDSHIP_FRIENDD_MEMONAME
//		") ON DUPLICATE KEY UPDATE "
//		FRIENDSHIP_FRIENDD_MEMONAME"=VALUES(" FRIENDSHIP_FRIENDD_MEMONAME");"
//		)
//		.set_parameter(FRIENDSHIP_FRIENDD_USERID, friendShip.GetUserId().c_str())
//		.set_parameter(FRIENDSHIP_FRIENDD_FRIENDID, friendShip.GetFriendId().c_str())
//		.set_parameter(FRIENDSHIP_FRIENDD_STATE, (int)friendShip.GetState())
//		.set_parameter(FRIENDSHIP_FRIENDD_MEMONAME, friendShip.GetMemoName().c_str());
//
//	//DbgLog("exec_update %s", query.to_string().c_str());
//	if (false == pConn->exec_update(query))
//		return (false);
//	pConn->free_result();
//
//	return true;
//}


void CMysqlFriendMgr::FriendValToQuery(const CFriendRelation& friendShip, acl::query& query)
{
	query.reset();
	query.create_sql("(:" FRIENDSHIP_FRIENDD_USERID", :" FRIENDSHIP_FRIENDD_FRIENDID", :" FRIENDSHIP_FRIENDD_MEMONAME
		", :" FRIENDSHIP_FRIENDD_DESC", :" FRIENDSHIP_FRIENDD_PACKETID", :" FRIENDSHIP_FRIENDD_STATE")")
		.set_parameter(FRIENDSHIP_FRIENDD_USERID, friendShip.GetUserId().c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_FRIENDID, friendShip.GetFriendId().c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_MEMONAME, friendShip.GetMemoName().c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_DESC, friendShip.GetDesc().c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_PACKETID, friendShip.GetPacketId())
		.set_parameter(FRIENDSHIP_FRIENDD_SELFINTRODUCE, friendShip.GetSelfIntroduce().c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_STATE, (int)friendShip.GetState());
}

CFriendRelation CMysqlFriendMgr::rowToFriendShip(const acl::db_row* pRow)
{
	//ASSERT(pRow != NULL);
	const acl::db_row& row = *pRow;

	string userId(row[(size_t)0]);
	string friendId(row[1]);
	int state = atoi(row[2]);
	int nofityMsgDeliver = atoi(row[3]);
	int soundOff = atoi(row[4]);
	int hideFlag = atoi(row[5]);

	return CFriendRelation(userId, friendId, (CFriendRelation::FRIEND_STATE)state, nofityMsgDeliver, soundOff, hideFlag);
}

bool CMysqlFriendMgr::UpdateFriendShip(const CFriendRelation& friendShip)
{
	return InsertFriendShip(friendShip);
}


bool CMysqlFriendMgr::UpdateFriendShipState(const string& userId, const string& friendId, int state)
{
	return InsertFriendShip(CFriendRelation(userId, friendId, (CFriendRelation::FRIEND_STATE)state));
}

std::vector<CFriendRelation> CMysqlFriendMgr::GetFriendShips(const string& userId, const string& friendId) throw(int)
{
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		WarnLog("get mysql connection for msgSvr_mysql_slave failed");
		throw - 1;
	}

	std::vector<CFriendRelation> friendShips;

	/* generate query string */
	char queryStr[512] = { '\0' };
	int queryStrLen = 0;
	queryStrLen = sprintf(queryStr + queryStrLen, "SELECT " FRIENDSHIP_FRIENDD_USERID", "
		FRIENDSHIP_FRIENDD_FRIENDID", "
		FRIENDSHIP_FRIENDD_STATE", "
		FRIENDSHIP_FRIENDD_SENDMSGREAD", "
		FRIENDSHIP_FRIENDD_SOUNDOFF", "
		FRIENDSHIP_FRIENDD_HIDEFLAG
		" FROM " FRIENDSHIP_TABLE_NAME " WHERE("
		FRIENDSHIP_FRIENDD_USERID"= :" FRIENDSHIP_FRIENDD_USERID
		" AND " FRIENDSHIP_FRIENDD_FRIENDID"= :" FRIENDSHIP_FRIENDD_FRIENDID ") OR ("
		FRIENDSHIP_FRIENDD_USERID"= :" FRIENDSHIP_FRIENDD_USERID2
		" AND " FRIENDSHIP_FRIENDD_FRIENDID"= :" FRIENDSHIP_FRIENDD_FRIENDID2 ");");

	acl::query query;
	query.create_sql(queryStr)
		.set_parameter(FRIENDSHIP_FRIENDD_USERID, userId.c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_FRIENDID, friendId.c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_USERID2, friendId.c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_FRIENDID2, userId.c_str());

	//DbgLog("%s", query.to_string().c_str());
	if (pConn->exec_select(query) == false)
	{
		throw (-1);
	}

	//get result
	const acl::db_rows* result = pConn->get_result();
	if (result)
	{
		const std::vector<acl::db_row*>& rows = result->get_rows();
		for (size_t i = 0; i < rows.size(); i++)
		{
			const acl::db_row* pRow = rows[i];
			if (pRow)
			{
				friendShips.push_back(rowToFriendShip(pRow));
			}
		}
	}

	return friendShips;
}

std::vector<CFriendRelation> CMysqlFriendMgr::GetFriendShips(const std::string& userId) throw(int)
{
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		WarnLog("get mysql connection for msgSvr_mysql_slave failed");
		throw - 1;
	}

	std::vector<CFriendRelation> friendShips;

	/* generate query string */
	char queryStr[512] = { '\0' };
	int queryStrLen = 0;
	queryStrLen = sprintf(queryStr + queryStrLen, "SELECT " FRIENDSHIP_FRIENDD_USERID", "
		FRIENDSHIP_FRIENDD_FRIENDID", "
		FRIENDSHIP_FRIENDD_STATE", "
		FRIENDSHIP_FRIENDD_SENDMSGREAD", "
		FRIENDSHIP_FRIENDD_SOUNDOFF", "
		FRIENDSHIP_FRIENDD_HIDEFLAG
		" FROM " FRIENDSHIP_TABLE_NAME 
		" WHERE " FRIENDSHIP_FRIENDD_FRIENDID 
			"IN( SELECT " FRIENDSHIP_FRIENDD_USERID 
			" FROM " FRIENDSHIP_TABLE_NAME 
			" WHERE " FRIENDSHIP_FRIENDD_FRIENDID"= :" FRIENDSHIP_FRIENDD_USERID 
			" AND " FRIENDSHIP_FRIENDD_STATE"= :" FRIENDSHIP_FRIENDD_STATE
		") AND " FRIENDSHIP_FRIENDD_USERID"= :" FRIENDSHIP_FRIENDD_USERID2
		" AND " FRIENDSHIP_FRIENDD_STATE "= :" FRIENDSHIP_FRIENDD_STATE2 " ;");

	acl::query query;
	query.create_sql(queryStr)
		.set_parameter(FRIENDSHIP_FRIENDD_USERID, userId.c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_STATE, CFriendRelation::FRIEND_RELATION_STATE_NORMAL)
		.set_parameter(FRIENDSHIP_FRIENDD_USERID2, userId.c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_STATE2, CFriendRelation::FRIEND_RELATION_STATE_NORMAL);

	//DbgLog("%s", query.to_string().c_str());
	if (pConn->exec_select(query) == false)
	{
		throw (-1);
	}

	//get result
	const acl::db_rows* result = pConn->get_result();
	if (result)
	{
		const std::vector<acl::db_row*>& rows = result->get_rows();
		for (size_t i = 0; i < rows.size(); i++)
		{
			const acl::db_row* pRow = rows[i];
			if (pRow)
			{
				friendShips.push_back(rowToFriendShip(pRow));
			}
		}
	}

	return friendShips;
}

bool CMysqlFriendMgr::DelFriendShip(const std::string& userId, const std::string& friendId)
{
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	if(nullptr == pConn)
		return false;
	acl::query query;
	query.create_sql("delete from " FRIENDSHIP_TABLE_NAME" where"
		FRIENDSHIP_FRIENDD_USERID "=:" FRIENDSHIP_FRIENDD_USERID" and "
		FRIENDSHIP_FRIENDD_FRIENDID "=:" FRIENDSHIP_FRIENDD_FRIENDID";"
		)
		.set_parameter(FRIENDSHIP_FRIENDD_USERID, userId.c_str())
		.set_parameter(FRIENDSHIP_FRIENDD_FRIENDID, friendId.c_str());

	//DbgLog("exec_update %s", query.to_string().c_str());
	if (false == pConn->exec_update(query))
		return (false);
	pConn->free_result();

	return true;
}
