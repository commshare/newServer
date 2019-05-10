/******************************************************************************

******************************************************************************/
#include <json/json.h>
#include "redisMgr.h"
#include "AutoPool.h"
#include "util.h"
#include "common.h"

#define CACHE_NAME_CUST "custDb"

CRedisMgr::CRedisMgr()
{

}

CRedisMgr::~CRedisMgr()
{

}

bool CRedisMgr::init(uint32_t num)
{
	m_sessionListNum = num;
	CacheManager::getInstance()->Init();
}

bool CRedisMgr::getCurrentCustInfoByUser(const string userId,CUST::USER_PROCESSING_INFO& info)
{
	#if 0
	CAutoCache conn(CACHE_NAME_CUST);

	char keyUserCurrentIn[128];
	snprintf(keyUserCurrentIn,47,"%s%s",KEY_USER_CURRENT_IN_,userId.c_str());

	map<string,string> retCurrentIn;
	if(false == conn.getCacheConn()->hgetAll(keyUserCurrentIn,ret))
	{
		DbgLog("get key[%s] false",keyUserCurrentIn);
		return false;
	}

	info.custId = retCurrentIn[VALUE_CUST_ID];
	info.sessionType = retCurrentIn[VALUE_SESSION_TYPE];

	char keyCustInfo[128];
	snprintf(keyCustInfo,47,"%s%s",KEY_CUST_INFO_,info.custId.c_str());

	map<string,string> retCustInfo;
	if(false == conn.getCacheConn()->hgetAll(keyCustInfo,retCustInfo))
	{
		DbgLog("get key[%s] false",keyCustInfo);
		return false;
	}

	int32_t custStatus = string2int(retCustInfo[VALUE_CUST_STATUS]);
	
	if(CUST::STATUS_ONLINE != custStatus)
	{
		DbgLog("cust[%s] is not onLine",info.custId.c_str());
		return false;
	}
	
	info.wsServerIPPort = retCustInfo[VALUE_CUST_IN_WSSERVER_IP_PORT];

	return true;
	#endif
		
}



bool CRedisMgr::userToCustCurrent(const string userId,const string& custId)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];
	char key1[128];
		
	snprintf(key,127,"%s%s",KEY_CUSTOMER_SESSION_CURRENT_,custId.c_str());
	snprintf(key1,127,"%s%s",KEY_USER_CURRENT_IN_,userId.c_str());

	
	map<string,string> retVule;

	conn.getCacheConn()->hgetAll(key1,retVule);
	string custIdUserIn = retVule[VALUE_CUST_ID];
	uint32_t sessionType = string2int(retVule[VALUE_SESSION_TYPE]);

	if(custId.compare(custIdUserIn) == 0 && sessionType == CUST::TYPE_CURRENT)
	{
		DbgLog("User[%s] haved in cust current [%s] ",userId.c_str(),custId.c_str());
		return true;
	}
	
	if(-1 == conn.getCacheConn()->rpush(key,userId))
	{
		return false;
	}
	else
	{
		//设置用户所在队列信息
		map<string,string> fields;
		fields[VALUE_CUST_ID] = custId;
		fields[VALUE_SESSION_TYPE] = CUST::TYPE_CURRENT;
		
		conn.getCacheConn()->hmset(key1,fields);	
	}
	
	return true;
	
}

bool CRedisMgr::usersToCustCurrent(list<string>&users,const string& custId)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];

	snprintf(key,127,"%s%s",KEY_CUSTOMER_SESSION_CURRENT_,custId.c_str());
	if(-1 == conn.getCacheConn()->rmpush(key,users))
	{
		return false;
	}
	else
	{
		//设置用户所在队列信息
		list<string>::iterator pUser = users.begin();

		for(; pUser != users.end(); pUser++)
		{
			char key1[128];
			snprintf(key1,127,"%s%s",KEY_USER_CURRENT_IN_,(*pUser).c_str());
			map<string,string> fields;
			fields[VALUE_CUST_ID] = custId;
			fields[VALUE_SESSION_TYPE] = CUST::TYPE_CURRENT;
			
			conn.getCacheConn()->hmset(key1,fields);
		}
	}
	
	return true;
}

#if 0
bool CRedisMgr::getCustCurrentUsers(const string& custId,list<CUST::SESSION_INFO>& usersInfo)
{
	CAutoCache conn(CACHE_NAME_CUST);
	
	char key[128];
	snprintf(key,127,"%s%s",KEY_CUSTOMER_SESSION_CURRENT_,custId.c_str());

	list<string> users,userKeys;
	if(true == conn.getCacheConn()->lrange(key, 0, -1, users))
	{
		list<string>::iterator pUser = users.begin();
		for(; pUser != users.end();  pUser++)
		{
			char key1[128];
			snprintf(key1,127,"%s%s",KEY_SESSION_USER_INFO_,(*pUser).c_str());
			userKeys.push_back(key1);
		}
		
		map<string,map<string,string>> usersInfoTmp;
		if(true == conn.getCacheConn()->hgetAllBykeys(userKeys,usersInfoTmp))
		{
			map<string,map<string,string>>::iterator pUserInfo = usersInfoTmp.begin();
			for(; pUserInfo!=usersInfoTmp.end();  pUserInfo++)
			{
				CUST::SESSION_INFO userInfo;
				char userId[128];
				sscanf((pUserInfo->first).c_str(),"SESSION_USER_INFO_%s",userId);
				userInfo.userId = userId;
				userInfo.nickName = pUserInfo->second[CUST_JSON_FIELD_NICK_NAME];
				userInfo.headPixel = pUserInfo->second[CUST_JSON_FIELD_HEAD_PIXEL];
				userInfo.lastMsgContent = pUserInfo->second[CUST_JSON_FIELD_MSG_CONTENT];
				userInfo.lastMsgTime = atol(pUserInfo->second[CUST_JSON_FIELD_MSG_TIME].c_str());
				usersInfo.push_back(userInfo);
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
	  
}

#endif

bool CRedisMgr::getCustCurrentUsers(const string custId,list<string>& users)
{
	CAutoCache conn(CACHE_NAME_CUST);
	
	char key[128];
	snprintf(key,127,"%s%s",KEY_CUSTOMER_SESSION_CURRENT_,custId.c_str());

	return conn.getCacheConn()->lrange(key, 0, -1, users);
	
}


bool CRedisMgr::delUserFromCustCurrent(const string custId,const string& userId,string& queueUser)
{
	CAutoCache conn(CACHE_NAME_CUST);

	char keyCurrent[128];
	snprintf(keyCurrent,127,"%s%s",KEY_CUSTOMER_SESSION_CURRENT_,custId.c_str());
	
	long num = conn.getCacheConn()->lrem(keyCurrent, userId);
	if(num > 0)
	{
		//当前会话进历史会话
		//char keyHistory[128];
		//snprintf(keyHistory,127,"%s%s",KEY_CUSTOMER_SESSION_HISTORY_,custId.c_str());
		//conn.getCacheConn()->rpush(keyHistory,userId);

		char keyUserIn[128];
		snprintf(keyUserIn,127,"%s%s",KEY_USER_CURRENT_IN_,userId.c_str());
		conn.getCacheConn()->del(keyUserIn);
		
		//排队队列进一个到当前会话
		char keyQueue[128];
		snprintf(keyQueue,127,"%s%s",KEY_CUSTOMER_SESSION_QUEUE_,custId.c_str());

		if(0 ==conn.getCacheConn()->lpop(keyQueue,queueUser))
		{
			userToCustCurrent(queueUser,custId);
		}

	}

	return true;
}

bool CRedisMgr::userToCustQueue(const string userId,const string& custId)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];

	snprintf(key,127,"%s%s",KEY_CUSTOMER_SESSION_QUEUE_,custId.c_str());
	if(-1 == conn.getCacheConn()->rpush(key,custId))
	{
		return false;
	}
	else
	{
		//设置用户所在队列信息
		char key1[128];
		snprintf(key1,127,"%s%s",KEY_USER_CURRENT_IN_,userId.c_str());
		map<string,string> fields;
		fields[VALUE_CUST_ID] = custId;
		fields[VALUE_SESSION_TYPE] = CUST::TYPE_QUEUE;
		
		conn.getCacheConn()->hmset(key1,fields);
	}
	return true;
}

bool CRedisMgr::usersToCustQueue(list<string>& users,const string& custId)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];

	snprintf(key,127,"%s%s",KEY_CUSTOMER_SESSION_QUEUE_,custId.c_str());
	if(-1 == conn.getCacheConn()->rmpush(key,users))
	{
		return false;
	}
	else
	{
		//设置用户所在队列信息
		list<string>::iterator pUser = users.begin();

		for(; pUser != users.end(); pUser++)
		{
			char key1[128];
			snprintf(key1,127,"%s%s",KEY_USER_CURRENT_IN_,(*pUser).c_str());
			map<string,string> fields;
			fields[VALUE_CUST_ID] = custId;
			fields[VALUE_SESSION_TYPE] = CUST::TYPE_QUEUE;
			
			conn.getCacheConn()->hmset(key1,fields);
		}
	}
	return true;
	
}

#if 0
bool CRedisMgr::getCustQueueUsers(const string& custId,list<CUST::SESSION_INFO>& usersInfo)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];

	snprintf(key,127,"%s%s",KEY_CUSTOMER_SESSION_QUEUE_,custId.c_str());

	list<string> users,userKeys;
	if(true == conn.getCacheConn()->lrange(key, 0, -1, users))
	{
		list<string>::iterator pUser = users.begin();
		for(; pUser != users.end();  pUser++)
		{
			char key1[128];
			snprintf(key1,127,"%s%s",KEY_SESSION_USER_INFO_,(*pUser).c_str());
			userKeys.push_back(key1);
		}
		
		map<string,map<string,string>> usersInfoTmp;
		if(true == conn.getCacheConn()->hgetAllBykeys(userKeys,usersInfoTmp))
		{
			map<string,map<string,string>>::iterator pUserInfo = usersInfoTmp.begin();
			for(; pUserInfo!=usersInfoTmp.end();  pUserInfo++)
			{
				CUST::SESSION_INFO userInfo;
				char userId[128];
				sscanf((pUserInfo->first).c_str(),"SESSION_USER_INFO_%s",userId);
				userInfo.userId = userId;
				userInfo.nickName = pUserInfo->second[CUST_JSON_FIELD_NICK_NAME];
				userInfo.headPixel = pUserInfo->second[CUST_JSON_FIELD_HEAD_PIXEL];
				userInfo.lastMsgContent = pUserInfo->second[CUST_JSON_FIELD_MSG_CONTENT];
				userInfo.lastMsgTime = atol(pUserInfo->second[CUST_JSON_FIELD_MSG_TIME].c_str());
				usersInfo.push_back(userInfo);
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}
#endif

bool CRedisMgr::getCustQueueUsers(const string custId,list<string>& users)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];

	snprintf(key,127,"%s%s",KEY_CUSTOMER_SESSION_QUEUE_,custId.c_str());

	return conn.getCacheConn()->lrange(key, 0, -1, users);
}


bool CRedisMgr::userToCustHistory(const string userId,const string& custId)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];

	snprintf(key,127,"%s%s",KEY_CUSTOMER_SESSION_HISTORY_,custId.c_str());
	if(-1 == conn.getCacheConn()->rpush(key,custId))
	{
		return false;
	}

	return true;
}

#if 0
bool CRedisMgr::getCustHistoryUsers(const string& custId,list<CUST::SESSION_INFO>& usersInfo)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];

	snprintf(key,127,"%s%s",KEY_CUSTOMER_SESSION_HISTORY_,custId.c_str());

	list<string> users,userKeys;
	if(true == conn.getCacheConn()->lrange(key, 0, 14, users))
	{
		list<string>::iterator pUser = users.begin();
		for(; pUser != users.end();  pUser++)
		{
			char key1[128];
			snprintf(key1,127,"%s%s",KEY_SESSION_USER_INFO_,(*pUser).c_str());
			userKeys.push_back(key1);
		}
		
		map<string,map<string,string>> usersInfoTmp;
		if(true == conn.getCacheConn()->hgetAllBykeys(userKeys,usersInfoTmp))
		{
			map<string,map<string,string>>::iterator pUserInfo = usersInfoTmp.begin();
			for(; pUserInfo!=usersInfoTmp.end();  pUserInfo++)
			{
				CUST::SESSION_INFO userInfo;
				char userId[128];
				sscanf((pUserInfo->first).c_str(),"SESSION_USER_INFO_%s",userId);
				userInfo.userId = userId;
				userInfo.nickName = pUserInfo->second[CUST_JSON_FIELD_NICK_NAME];
				userInfo.headPixel = pUserInfo->second[CUST_JSON_FIELD_HEAD_PIXEL];
				userInfo.lastMsgContent = pUserInfo->second[CUST_JSON_FIELD_MSG_CONTENT];
				userInfo.lastMsgTime = atol(pUserInfo->second[CUST_JSON_FIELD_MSG_TIME].c_str());
				usersInfo.push_back(userInfo);
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}
#endif

bool CRedisMgr::getCustInfo(list<string>& custList,map<string,map<string,string>>& custInfo)
{
	CAutoCache conn(CACHE_NAME_CUST);
	list<string>::iterator pCust = custList.begin();
	list<string> keys;
	for(; pCust != custList.end(); pCust++)
	{
		char key[128];
		snprintf(key,127,"%s%s",KEY_CUST_INFO_,pCust->c_str());
		keys.push_back(key);
	}

	return conn.getCacheConn()->hgetAllBykeys(keys,custInfo);
	
}

bool CRedisMgr::getCustInfo(const string custId,CUST::CUST_INFO& custInfo)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];
	snprintf(key,127,"%s%s",KEY_CUST_INFO_,custId.c_str());

	map<string,string> retValue;
	bool ret = conn.getCacheConn()->hgetAll(key,retValue);
	if( true == ret )
	{
		custInfo.custId = custId;
		custInfo.custStatus = CUST::USER_STATUS(string2int(retValue[VALUE_CUST_STATUS]));
		custInfo.wsServerIPPort = retValue[VALUE_CUST_IN_WSSERVER_IP_PORT];
		//custInfo.wsServerPort = string2int(retValue[VALUE_CUST_IN_WSSERVER_PORT]);
	}

	return ret;
}


bool CRedisMgr::getCustStatus(const string custId,USER_STATUS& status)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];
	string value;
	snprintf(key,127,"%s%s",KEY_CUST_INFO_,custId.c_str());

	value == conn.getCacheConn()->hget(key,VALUE_CUST_STATUS);
	if(value.empty())
	{
		return false;
	}

	status = CUST::USER_STATUS(atoi(value.c_str()));
	return true;
}

bool CRedisMgr::setCustStatus(const string custId,USER_STATUS status)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];
	snprintf(key,127,"%s%s",KEY_CUST_INFO_,custId.c_str());

	if(-1 == conn.getCacheConn()->hset(key,VALUE_CUST_STATUS,int2string(status)))
	{
		return false;
	}

	return true;
	
}

bool CRedisMgr::custSessionIsExist(list<string> keys,list<string>& existKeys)
{
	CAutoCache conn(CACHE_NAME_CUST);
	return conn.getCacheConn()->isExistsKeys(keys,existKeys);	
}

bool CRedisMgr::delCustSession(list<string> keys)
{
	CAutoCache conn(CACHE_NAME_CUST);
	return conn.getCacheConn()->delKeys(keys);
}

bool CRedisMgr::getUserInfo(const string userId,CUST::USER_INFO & userInfo)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];
	string value;
	snprintf(key,127,"%s%s",KEY_USER_INFO,userId.c_str());

	map<string,string> values;
	if( true == conn.getCacheConn()->hgetAll(key,values) )
	{
		userInfo.userId = values[VALUE_USER_USER_ID];
		userInfo.nickName = values[VALUE_USER_NICK_NAME];
		userInfo.headPixel = values[VALUE_USER_HEADPIXEL];
	}
	else
	{
		return false;
	}
	
	return true;
}

bool CRedisMgr::insertUserInfo(const string userId,const CUST::USER_INFO & userInfo)
{
	CAutoCache conn(CACHE_NAME_CUST);
	char key[128];
	string value;
	snprintf(key,127,"%s%s",KEY_USER_INFO,userId.c_str());
	
	map<string,string> values;
	values[VALUE_USER_USER_ID] = userInfo.userId ;
	values[VALUE_USER_NICK_NAME] = userInfo.nickName;
	values[VALUE_USER_HEADPIXEL] = userInfo.headPixel;

	conn.getCacheConn()->hmset(key,values);

	//设置KEY过期时间--TODO

	
	return true;
}





