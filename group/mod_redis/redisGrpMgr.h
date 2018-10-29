/******************************************************************************
Filename: redisGrpMgr.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/01
Description: 
******************************************************************************/
#ifndef __REDISGRPMGR_H__
#define __REDISGRPMGR_H__

#include <string>
#include <map>
#include "im_grpmem.h"
using std::string;


class CReidsGrpMgr
{
public:
	CReidsGrpMgr();
	~CReidsGrpMgr();
	static std::map<std::string, CGrpMem> GetGrpMems(const string& grpId);
	static bool InsertGrpMem(const CGrpMem& grpMem);
	static bool InsertGrpMem(const string& grpId, const std::map<std::string, CGrpMem>& grpMems, bool bInsertQuitedMem = false);
	static bool InsertGrpMem(const string& grpId, const string& memId, int state);
	static bool UpdateGrpMem(const string& grpId, const string& memId, int state);
	static bool IsGrpMem(const string& grpId, const string& memId);

private:
	static string generateGrpKey(const string& grpId);
};

#endif // __REDISGRPMGR_H__

