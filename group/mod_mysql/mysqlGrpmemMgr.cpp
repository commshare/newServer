/******************************************************************************
Filename: mysqlGrpMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/02
Description: 
******************************************************************************/
#include "mysqlGrpmemMgr.h"
#include "mysqlPool.h"
#include "acl_cpp/lib_acl.hpp"

#define GRPMEM_TABLE_NAME				"`groupmember_t`"
#define GRPMEM_FIELD_GRPID				"`GROUP_ID`"
#define GRPMEM_FIELD_MEMID				"`MEMBER_ID`"
#define GRPMEM_FIELD_STATE				"`MEMBER_STATUS`"
#define GRPMEM_FIELD_DESC				"`MEMBER_REMARKS`"

#define GRPMEM_FIELD_CREATETIME			"`MEMBER_CREATETIME`"
#define GRPMEM_FIELD_MODIFYTIME			"`MEMBER_UPDATETIME`"


CMysqlGrpmemMgr::CMysqlGrpmemMgr()
{

}

CMysqlGrpmemMgr::~CMysqlGrpmemMgr()
{

}

bool CMysqlGrpmemMgr::InsertGrpmem(const CGrpMem& grpmem)
{
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	acl::query query;
	query.create_sql("insert into " GRPMEM_TABLE_NAME"("
		GRPMEM_FIELD_GRPID", "
		GRPMEM_FIELD_MEMID", "
		GRPMEM_FIELD_STATE", "
		GRPMEM_FIELD_DESC", "
		GRPMEM_FIELD_CREATETIME
		") values("
		":" GRPMEM_FIELD_GRPID ", "
		":" GRPMEM_FIELD_MEMID ","
		":" GRPMEM_FIELD_STATE ","
		":" GRPMEM_FIELD_DESC ","
		"now()"
		") ON DUPLICATE KEY UPDATE "
		GRPMEM_FIELD_STATE"=VALUES(" GRPMEM_FIELD_STATE"),"
		GRPMEM_FIELD_MODIFYTIME"=now();"
		)
		.set_parameter(GRPMEM_FIELD_GRPID, grpmem.GetGrpId().c_str())
		.set_parameter(GRPMEM_FIELD_MEMID, grpmem.GetMemId().c_str())
		.set_parameter(GRPMEM_FIELD_STATE, (int)grpmem.GetState())
		.set_parameter(GRPMEM_FIELD_DESC, grpmem.GetDesc().c_str());

	//DbgLog("exec_update %s", query.to_string().c_str());
	if (false == pConn->exec_update(query))
		return (false);

	pConn->free_result();
	return true;
}


//void CMysqlGrpmemMgr::GrpMemValToQuery(const CGrpMem& grpmem, acl::query& query)
//{
//	query.reset();
//	query.create_sql("(:" GRPMEM_FIELD_GRPID", :" GRPMEM_FIELD_MEMID", :" GRPMEM_FIELD_STATE
//		", :" GRPMEM_FIELD_DESC")")
//		.set_parameter(GRPMEM_FIELD_GRPID, grpmem.GetGrpId().c_str())
//		.set_parameter(GRPMEM_FIELD_MEMID, grpmem.GetMemId().c_str())
//		.set_parameter(GRPMEM_FIELD_STATE, grpmem.GetState())
//		.set_parameter(GRPMEM_FIELD_DESC, grpmem.GetDesc().c_str());
//}

CGrpMem CMysqlGrpmemMgr::rowToGrpmem(const acl::db_row* pRow)
{
	//ASSERT(pRow != NULL);
	const acl::db_row& row = *pRow;

	string grpId(row[(size_t)0]);
	string memId(row[1]);
	int state = atoi(row[2]);
	string desc(row[3]);

	return CGrpMem(grpId, memId, state, desc);
}

bool CMysqlGrpmemMgr::UpdateGrpmem(const CGrpMem& grpmem)
{
	return InsertGrpmem(grpmem);
}


bool CMysqlGrpmemMgr::UpdateGrpmemState(const std::string& grpId, const std::string& memId, int state)
{
	return InsertGrpmem(CGrpMem(grpId, memId, state));
}

std::map<std::string, CGrpMem> CMysqlGrpmemMgr::GetGrpmems(const string& grpId) throw(int)
{
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		WarnLog("get mysql connection for msgSvr_mysql_slave failed");
		throw - 1;
	}

	/* generate query string */
	char queryStr[512] = { '\0' };
	int queryStrLen = 0;
	queryStrLen = sprintf(queryStr + queryStrLen, "SELECT " GRPMEM_FIELD_GRPID", "
		GRPMEM_FIELD_MEMID", "
		GRPMEM_FIELD_STATE", "
		GRPMEM_FIELD_DESC
		" FROM " GRPMEM_TABLE_NAME " WHERE("
		GRPMEM_FIELD_GRPID"= :" GRPMEM_FIELD_GRPID
		" AND "	GRPMEM_FIELD_STATE "!= 0 "
		");");

	acl::query query;
	query.create_sql(queryStr)
		.set_parameter(GRPMEM_FIELD_GRPID, grpId.c_str());

	//DbgLog("%s", query.to_string().c_str());
	if (pConn->exec_select(query) == false)
	{
		throw (-1);
	}

	//get result
	std::map<std::string, CGrpMem> grpmems;
	const acl::db_rows* result = pConn->get_result();
	if (result)
	{
		const std::vector<acl::db_row*>& rows = result->get_rows();
		for (size_t i = 0; i < rows.size(); i++)
		{
			const acl::db_row* pRow = rows[i];
			if (pRow)
			{
				grpmems.insert(std::pair<std::string, CGrpMem>((*pRow)[1], rowToGrpmem(pRow)));
			}
		}
	}

	return grpmems;
}

std::shared_ptr<CGrpMem> CMysqlGrpmemMgr::GetGrpmem(const std::string& grpId, const std::string& memId) throw(int)
{
	std::unique_ptr<CDBConn> pConn = CDBManager::getInstance()->GetDBConn();
	if (!pConn)
	{
		WarnLog("get mysql connection for msgSvr_mysql_slave failed");
		throw - 1;
	}

	std::vector<CGrpMem> grpmems;

	/* generate query string */
	char queryStr[512] = { '\0' };
	int queryStrLen = 0;
	queryStrLen = sprintf(queryStr + queryStrLen, "SELECT " GRPMEM_FIELD_GRPID", "
		GRPMEM_FIELD_MEMID", "
		GRPMEM_FIELD_STATE", "
		GRPMEM_FIELD_DESC
		" FROM " GRPMEM_TABLE_NAME " WHERE("
		GRPMEM_FIELD_GRPID"= :" GRPMEM_FIELD_GRPID
		" AND "	GRPMEM_FIELD_MEMID"= :" GRPMEM_FIELD_MEMID ");");

	acl::query query;
	query.create_sql(queryStr)
		.set_parameter(GRPMEM_FIELD_GRPID, grpId.c_str())
		.set_parameter(GRPMEM_FIELD_MEMID, memId.c_str());

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
		const acl::db_row* pRow = rows[0];
		if (pRow)
		{
			return std::shared_ptr<CGrpMem>(new CGrpMem(rowToGrpmem(pRow)));
		}
	}
	return std::shared_ptr<CGrpMem>(NULL);
}

