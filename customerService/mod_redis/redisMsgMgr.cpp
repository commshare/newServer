/******************************************************************************
Filename: redisFriendMgr.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/05
Description:
******************************************************************************/
#include "redisMsgMgr.h"
#include "redisPool.h"

CReidsProtoMsgMgr::CReidsProtoMsgMgr()
{

}

CReidsProtoMsgMgr::~CReidsProtoMsgMgr()
{

}

bool CReidsProtoMsgMgr::InsertProtoMsg(const std::vector<std::string>& msgs)
{
	if (msgs.empty())
	{
		return true;
	}
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();

	std::vector<acl::string> val;
	for (unsigned int i = 0; i < msgs.size(); ++i)
	{
		val.push_back(msgs[i].c_str());
	}

	conn.rpush(getMsgWriteKey(), val);
	return true;
}

string CReidsProtoMsgMgr::PopCustomerServiceMsg()
{
	CRedisConn conn = CRedisManager::getInstance()->GetCRedisConn();
	return conn.lpop(getMsgReadKey());
}



const char* CReidsProtoMsgMgr::getMsgReadKey()
{
#ifdef DEBUG
	return "IMSwooleClient_l_clientChatQueue";
#else
	return "IM_KF_TO_USER";
#endif
}

const char* CReidsProtoMsgMgr::getMsgWriteKey()
{
	return "IMSwooleClient_l_clientChatQueue";
}


