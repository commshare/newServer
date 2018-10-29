/******************************************************************************
Filename: mysqlGrpmemMgr.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/02
Description: 
******************************************************************************/
#ifndef __MYSQLGRPMEMMGR_H__
#define __MYSQLGRPMEMMGR_H__

#include <memory>
#include <map>
#include <string>
#include "acl_cpp/lib_acl.hpp"
#include "im_grpmem.h"

class CMysqlGrpmemMgr
{
public:
	CMysqlGrpmemMgr();
	~CMysqlGrpmemMgr();
	static bool InsertGrpmem(const CGrpMem& grpmem);
	static bool UpdateGrpmem(const CGrpMem& grpmem);

	static bool UpdateGrpmemState(const std::string& grpId, const std::string& memId, int state);

	//查询userId的好友关系
	//grpmems:返回参数，返回查询结果（a-b,b-a）
	static std::map<std::string, CGrpMem> GetGrpmems(const std::string& grpId) throw(int);
	static std::shared_ptr<CGrpMem> GetGrpmem(const std::string& grpId, const std::string& memId) throw(int);
private:
	//static void GrpMemValToQuery(const CGrpMem& rel, acl::query& query);
	static CGrpMem rowToGrpmem(const acl::db_row* pRow);
};

#endif // __MYSQLGRPMEMMGR_H__
