/******************************************************************************
Filename: redisGrpMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/05
Description: 
******************************************************************************/
#include "redisGrpMgr.h"
#include "redisPool.h"

CReidsGrpMgr::CReidsGrpMgr()
{

}

CReidsGrpMgr::~CReidsGrpMgr()
{

}

std::map<std::string, CGrpMem> CReidsGrpMgr::GetGrpMems(const string& grpId)
{
	std::map<std::string, CGrpMem> grpMems;

	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::vector<const char*> useIds;
	std::vector<const char*> useStates;
	if (conn.hgetAll(generateGrpKey(grpId), useIds, useStates))
	{
		for (unsigned int i = 0; i < useIds.size(); ++i)
		{
			grpMems.insert(std::pair<std::string, CGrpMem>(useIds[i], CGrpMem(grpId, useIds[i], atoi(useStates[i]))));
		}
	}
	return grpMems;
}

bool CReidsGrpMgr::InsertGrpMem(const CGrpMem& grpMem)
{
	return InsertGrpMem(grpMem.GetGrpId(), grpMem.GetMemId(), grpMem.GetState());
}

bool CReidsGrpMgr::InsertGrpMem(const string& grpId, const string& memId, int state)
{
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::map<string, string> hashVals;
	char state_str[12] = { '\0' };
	sprintf(state_str, "%d", state);
	hashVals.insert(std::pair<string, string>(memId, string(state_str)));
	return conn.hmset(generateGrpKey(grpId), hashVals);
}

bool CReidsGrpMgr::InsertGrpMem(const string& grpId, const std::map<std::string, CGrpMem>& grpMems, bool bInsertQuitedMem)
{
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	std::map<string, string> hashVals;

	std::map<std::string, CGrpMem>::const_iterator iter = grpMems.begin();
	char state_str[12] = { '\0' };
	while (iter != grpMems.end())
	{
		if (grpId != iter->second.GetGrpId() || (false == bInsertQuitedMem && 0 == iter->second.GetState()))
		{
			++iter;
			continue;
		}
		memset(state_str, 0, sizeof(state_str));
		sprintf(state_str, "%d", iter->second.GetState());
		hashVals.insert(std::pair<string, string>(iter->second.GetMemId(), string(state_str)));
		++iter;
	}

	return conn.hmset(generateGrpKey(grpId), hashVals);
}

bool CReidsGrpMgr::UpdateGrpMem(const string& grpId, const string& memId, int state)
{
	return InsertGrpMem(grpId, memId, state);
}

string CReidsGrpMgr::generateGrpKey(const string& grpId)
{
	return "GRPM_" + grpId;
}

bool CReidsGrpMgr::IsGrpMem(const string& grpId, const string& memId)
{
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	string key = generateGrpKey(grpId);
	std::vector<const char*> fields;
	fields.push_back(memId.c_str());

	std::vector<acl::string> result;
	if (conn.hmget(key.c_str(), fields, &result))
	{
		if (!result.empty())
		{
			return true;
		}
	}

	return false;
}
