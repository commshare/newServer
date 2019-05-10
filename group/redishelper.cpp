/*****************************************************************************************
Filename: cache.cpp
Author: jack			Version: im-1.0 		Date:2017/06/14
Description: 	实现用户登录信息缓存、更新、缓存记录失效管理。
*****************************************************************************************/

#include "redishelper.h"

const char* sLoginUserFields[LOGIN_FIELDS_COUNT] =
{
	"role",
	"appid",
	"grouplimit",
	"groupnumber",
	"status",
	"deviceType",
	"relogin",
	"loginToken",
	"deviceToken",
	"deviceVersion",
	"cmIPAddr",
	"cmIPPort",
	"reloginTime",
	"loginTime"
};
const char* sGroupFields[GROUP_FIELDS_COUNT] = 
{
	"master",
	"count",
	"status",
	"permit",
	"createTime",
};

const char* sGroupMemberFields[GROUPMEMBER_FIELDS_COUNT] = 
{
	"status"
};
CRedisHelper::CRedisHelper()
{
}
CRedisHelper::~CRedisHelper()
{
}
bool CRedisHelper::Initialize(CConfigFileReader* pConfigReader)
{
	if(!pConfigReader)
		return false;
	
	m_pConfigReader = pConfigReader;

	SetRedisPara();
	acl::acl_cpp_init();
	acl::log::stdout_open(false);

	m_cluster.set_retry_inter(m_redisPara.nRetryInterval); //Default value: 1s.

	m_cluster.set_redirect_max(m_redisPara.nRetryLimit);

	m_cluster.set_redirect_sleep(m_redisPara.nRetrySleep);

	m_cluster.init(NULL, m_redisPara.sRedisAddrs.c_str(), m_redisPara.nPoolSize, \
				m_redisPara.nConnTimeout, m_redisPara.nRwTimeout);

	if(!m_redisPara.sPassword.empty())
		m_cluster.set_password("default", m_redisPara.sPassword.c_str());
	
	return true;
}

bool CRedisHelper::SetRedisPara(void)
{
	char* pRedis=m_pConfigReader->GetConfigName("redis_cluster");
	if(NULL==pRedis) 
		return false;
	m_redisPara.sRedisAddrs=pRedis;
	
	pRedis=m_pConfigReader->GetConfigName("redis_conntimeout");
	if(NULL==pRedis) 
		return false;
	m_redisPara.nConnTimeout=atoi(pRedis);

	pRedis=m_pConfigReader->GetConfigName("redis_rwtimeout");
	if(NULL==pRedis)
		return false;
	m_redisPara.nRwTimeout=atoi(pRedis);

	pRedis=m_pConfigReader->GetConfigName("redis_retryinterval");
	if(NULL==pRedis)
		return false;
	m_redisPara.nRetryInterval=atoi(pRedis);

	pRedis=m_pConfigReader->GetConfigName("redis_retrysleep");
	if(NULL==pRedis)
		return false;
	m_redisPara.nRetrySleep=atoi(pRedis);
	
	pRedis=m_pConfigReader->GetConfigName("redis_retrylimit");
	if(NULL==pRedis) 
		return false;
	m_redisPara.nRetryLimit=atoi(pRedis);

	pRedis=m_pConfigReader->GetConfigName("redis_password");
	if(NULL==pRedis)
		return false;
	m_redisPara.sPassword=pRedis;

	pRedis=m_pConfigReader->GetConfigName("redis_poolsize");
	if(NULL==pRedis)
		return false;
	m_redisPara.nPoolSize=atoi(pRedis);	
	
	pRedis=m_pConfigReader->GetConfigName("redis_poolsize");
	if(NULL==pRedis)
		return false;
	m_redisPara.nPoolSize=atoi(pRedis);	

	pRedis=m_pConfigReader->GetConfigName("redis_key_expiretime");
	if(NULL==pRedis) 
		return false;
	m_redisPara.nKeyExpireTime=atoi(pRedis);
		
	return true;

}


bool CRedisHelper::GetUserStatus(acl::redis_hash&      hashRedis,acl::string sUserId,uint8_t &bStatus)
{
	const char* field[1];
	std::vector<acl::string> result;

	field[0] = sLoginUserFields[STATUS_FIELD];

	result.clear();
	hashRedis.clear();
	if (hashRedis.hmget(sUserId, field, 1, &result) == false)
	{
		ErrLog("hmget error: %s\r\n", hashRedis.result_error());
		return false;
	}
	const char* val = hashRedis.result_value(0);
	bStatus = val ? atoi(val) : 0;

	return true;
}

bool CRedisHelper::GetMemberStatus(acl::redis_hash&     hashRedis,acl::string sGroupKey, string sMemberId,uint8_t &bStatus)
{
	std::vector<const char*> fields;
	std::vector<acl::string> result;

	result.clear();
	hashRedis.clear();

	fields.push_back(sMemberId.c_str());
	if (hashRedis.hmget(sGroupKey, fields, &result) == false)
	{
		ErrLog("hmget error: %s", hashRedis.result_error());
		return false;
	}
	
	const char* val = hashRedis.result_value(0);
	if(!val)
	{
		WarnLog("The group %s member %s is not exist!",sGroupKey.c_str(),sMemberId.c_str());
		return false;
	}
	bStatus = atoi(val);

	return true;
}
#if 0
bool CRedisHelper::GetGroupMaster(acl::redis_hash&     hashRedis,acl::string sGroupKey,
											string &sMasterId, uint8_t &bPermit)
{
	const char* field[2];
	std::vector<acl::string> result;

	field[0] = sGroupFields[GROUPMASTER_FIELD];
	field[1] = sGroupFields[GROUPPERMIT_FIELD];

	result.clear();
	hashRedis.clear();
	if (hashRedis.hmget(sGroupKey, field, 2, &result) == false)
	{
		ErrLog("hmget error: %s", hashRedis.result_error());
		return false;
	}
		
	const char* val = hashRedis.result_value(0);
	if(!val)
	{
		WarnLog("The group %s is not exist in cache!",sGroupKey.c_str());
		return false;
	}
	sMasterId = val;
	
	val = hashRedis.result_value(1);
	bPermit = val ? atoi(val) : 0;

	return true;
}
#endif
bool CRedisHelper::GetUserInfo(acl::redis_hash&     hashRedis,acl::string sUserId,UserCache_t &info)
{
	std::map<acl::string, acl::string> result;
	std::map<acl::string, acl::string>::const_iterator item;
	int valid_inx = 0;


	result.clear();
	hashRedis.clear();
	if (hashRedis.hgetall(sUserId.c_str(), result) == false)
	{
		ErrLog("hgetall key: %s error!", sUserId.c_str());
		return false;
	}

	if(result.empty()||result.size()<LOGIN_FIELDS_COUNT)
	{
		ErrLog("User %s is not exist or the record is not full!", 
			sUserId.c_str(),result.size());
		return false;
	}

	for(item = result.begin(); item != result.end(); item++)
	{
		if(item->first.equal(sLoginUserFields[ROLE_FIELD]))
		{
			info.bRole = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[APPID_FIELD]))
		{
			//printf("bCustomService\n");
			info.sAppId = item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[GROUPLIMIT_FIELD]))
		{
			info.nGroupLimit = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[GROUPNUMBER_FIELD]))
		{
			info.nGroupNumber = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[STATUS_FIELD]))
		{
			info.bStatus = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[DEVICETYPE_FIELD]))
		{

			//printf("bDeviceType\n");
			info.bDeviceType = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[RELOGIN_FIELD]))
		{
			//printf("bRelogin\n");
			info.bRelogin = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[LOGINTOKEN_FIELD]))
		{
			info.sLoginToken = item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[DEVICETOKEN_FIELD]))
		{
			info.sDeviceToken= item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[DEVICEVERSION_FIELD]))
		{
			info.sDeviceVersion= item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[IP_FIELD]))
		{
			info.sIPAddr= item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[PORT_FIELD]))
		{
			info.nIPPort= atoi(item->second.c_str());	
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[RELOGINTIME_FIELD]))
		{
			info.sReloginTime= item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[LOGINTIME_FIELD]))			
		{
			info.sLoginTime= item->second.c_str(); 
			valid_inx++;
		}
		else if(item->first.equal(sLoginUserFields[SESSION_ID_FIELD]))			
		{
			info.sSessionId= item->second.c_str(); 
			valid_inx++;
		}
	}

	if(valid_inx<LOGIN_FIELDS_COUNT)
	{
		ErrLog("Error field in this record, key is=%s",sUserId.c_str());
		return false;
	}

	info.sUserId = sUserId.c_str();
	
	return true;
}

bool CRedisHelper::GetGroupInfo(acl::redis_hash&     hashRedis,acl::string sGroupKey,GroupInfo_t &info)
{
	std::map<acl::string, acl::string> result;
	std::map<acl::string, acl::string>::const_iterator item;
	int valid_inx = 0;

	log("----------------redis  get group info begin----------------");
	result.clear();
	hashRedis.clear();
	if (hashRedis.hgetall(sGroupKey.c_str(), result) == false)
	{
		ErrLog("hgetall key: %s error!", sGroupKey.c_str());
		return false;
	}
	log("----------------redis get group info end----------------");

	if(result.empty()||result.size()<GROUP_FIELDS_COUNT)
	{
		ErrLog("Group %s is not exist or the record is not full!", 
			sGroupKey.c_str(),result.size());
		return false;
	}

	for(item = result.begin(); item != result.end(); item++)
	{
		if(item->first.equal(sGroupFields[GROUPMASTER_FIELD]))
		{
			info.sMasterId = item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sGroupFields[GROUPCOUNT_FIELD]))
		{
			info.nGroupNumber = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sGroupFields[GROUPSTATUS_FIELD]))
		{
			info.bStatus = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sGroupFields[GROUPPERMIT_FIELD]))
		{
			info.bPermit = atoi(item->second.c_str());
			valid_inx++;
		}		
		else if (item->first.equal(sGroupFields[GROUPCREATETIME_FIELD]))
		{
			info.createTime = atoi(item->second.c_str());
			valid_inx++;
		}
	}

	if(valid_inx<GROUP_FIELDS_COUNT)
	{
		ErrLog("Encount a error when reading group information from %s cache. ",sGroupKey.c_str());
		return false;
	}

	return true;

}

bool CRedisHelper::SetUserStatus(acl::redis_hash&     hashRedis,acl::string sUserId,int8_t bStatus)
{
	std::map<acl::string, acl::string> mFields;

	mFields[sLoginUserFields[STATUS_FIELD]]= (bStatus) ? '1' : '0';

	hashRedis.clear();
	if (hashRedis.hmset(sUserId.c_str(), mFields) == false)
	{
			ErrLog("Failed to set user %s status , hmset error: %s",
				sUserId.c_str(),hashRedis.result_error());
			return false;
	}
	mFields.clear();

	return true;
}

bool CRedisHelper::UpdateUserRec(acl::redis_hash&     hashRedis,UserCache_t &rec,bool bAuthCheck)
{
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];
	
	memset(tBuf,0,30);
	if(!bAuthCheck)
	{
		snprintf(tBuf, sizeof(tBuf), "%d", rec.bRole);				  //Refresh the role to cache
		mFields[sLoginUserFields[ROLE_FIELD]]=tBuf;
		mFields[sLoginUserFields[APPID_FIELD]]=rec.sAppId;
		snprintf(tBuf, sizeof(tBuf), "%d", rec.nGroupLimit);				  //Refresh the role to cache
		mFields[sLoginUserFields[GROUPLIMIT_FIELD]]=tBuf;
		snprintf(tBuf, sizeof(tBuf), "%d", rec.nGroupNumber);				  //Refresh the role to cache
		mFields[sLoginUserFields[GROUPNUMBER_FIELD]]=tBuf;
	}
	mFields[sLoginUserFields[STATUS_FIELD]]="1";
	snprintf(tBuf, sizeof(tBuf), "%d", rec.bDeviceType);
	mFields[sLoginUserFields[DEVICETYPE_FIELD]]=tBuf;
	mFields[sLoginUserFields[RELOGIN_FIELD]]='1';
	mFields[sLoginUserFields[LOGINTOKEN_FIELD]]=rec.sLoginToken;
	mFields[sLoginUserFields[DEVICETOKEN_FIELD]]=rec.sDeviceToken;
	mFields[sLoginUserFields[DEVICEVERSION_FIELD]]=rec.sDeviceVersion;
	mFields[sLoginUserFields[IP_FIELD]]=rec.sIPAddr;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nIPPort);
	mFields[sLoginUserFields[PORT_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%d",(int)time(0));
	mFields[sLoginUserFields[RELOGINTIME_FIELD]]=tBuf;
	
	hashRedis.clear();
	if (hashRedis.hmset(rec.sUserId.c_str(), mFields) == false)
	{
		ErrLog("Failed to set user %s relogin , hmset error: %s \n",
				rec.sUserId.c_str(),hashRedis.result_error());
	}
	mFields.clear();

	return true;	
}

 
bool CRedisHelper::InsertUserRec(acl::redis_hash&     hashRedis,UserCache_t &rec)							 // Insert a new user record into redis cache 
{
	acl::string key;
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];

	memset(tBuf,0,30);
	snprintf(tBuf, sizeof(tBuf), "%d", rec.bRole);
	mFields[sLoginUserFields[ROLE_FIELD]]=tBuf;
	mFields[sLoginUserFields[APPID_FIELD]]=rec.sAppId;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nGroupLimit);
	mFields[sLoginUserFields[GROUPLIMIT_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nGroupNumber);
	mFields[sLoginUserFields[GROUPNUMBER_FIELD]]=tBuf;
	mFields[sLoginUserFields[STATUS_FIELD]]="1";
	snprintf(tBuf, sizeof(tBuf), "%d", rec.bDeviceType);
	mFields[sLoginUserFields[DEVICETYPE_FIELD]]=tBuf;
	mFields[sLoginUserFields[RELOGIN_FIELD]]="0";
	mFields[sLoginUserFields[LOGINTOKEN_FIELD]]=rec.sLoginToken;
	mFields[sLoginUserFields[DEVICETOKEN_FIELD]]=rec.sDeviceToken;
	mFields[sLoginUserFields[DEVICEVERSION_FIELD]]=rec.sDeviceVersion;
	mFields[sLoginUserFields[IP_FIELD]]=rec.sIPAddr;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nIPPort);
	mFields[sLoginUserFields[PORT_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%d",(int)time(0));
	mFields[sLoginUserFields[RELOGINTIME_FIELD]]=tBuf;
	mFields[sLoginUserFields[LOGINTIME_FIELD]]= tBuf;

	key=rec.sUserId;

	hashRedis.clear();
	if (hashRedis.hmset(key.c_str(), mFields) == false)			    // Now start to insert	 
	{
		ErrLog("Failt to insert user %s rec , hmset error: %s",
				key.c_str(),hashRedis.result_error());
		return false;
	}
	mFields.clear();

	//RemoveUserExpireTime(key.c_str());

	return true;

}

bool CRedisHelper::InsertGroup(acl::redis_hash&     hashRedis,GroupInfo_t &info)
{
	acl::string key;
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];
	

	mFields[sGroupFields[GROUPMASTER_FIELD]]=info.sMasterId;
	snprintf(tBuf, sizeof(tBuf), "%d", info.nGroupNumber);
	mFields[sGroupFields[GROUPCOUNT_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%d", info.bStatus);
	mFields[sGroupFields[GROUPSTATUS_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%d", info.bPermit);
	mFields[sGroupFields[GROUPPERMIT_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%lu", info.createTime);
	mFields[sGroupFields[GROUPCREATETIME_FIELD]] = tBuf;

    log("--------------redis InsertGroup begin-------------------");
	hashRedis.clear();
    if (hashRedis.hmset(info.sGroupId.c_str(), mFields) == false)			    // Now start to insert	,cost 10s
	{
		ErrLog("Failt to insert group %s , hmset error: %s",
				info.sGroupId.c_str(),hashRedis.result_error());
		return false;
	}
	mFields.clear();
    log("--------------redis InsertGroup end-------------------");
	return true;
}
bool CRedisHelper::InsertGroupMember(acl::redis_hash&      hashRedis,acl::string sGroupKey,
								string sMemberId, uint8_t bStatus)
{
	acl::string key;
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];

	memset(tBuf,0,30);
	snprintf(tBuf, sizeof(tBuf), "%d", bStatus);
	mFields.insert(std::pair<acl::string,acl::string>(sMemberId.c_str(),tBuf));

	hashRedis.clear();
	if (hashRedis.hmset(sGroupKey.c_str(), mFields) == false)			    // Now start to insert	 
	{
		ErrLog("Failed to insert group %d member  %s , hmset error: %s",
				sGroupKey.c_str(),sMemberId.c_str(),hashRedis.result_error());
		return false;
	}
	mFields.clear();	

	return true;
}

bool CRedisHelper::InsertGroupMember(acl::redis_hash&		hashRedis,acl::string sGroupKey,
								RedisMemberMap_t& memberMap)
{
	hashRedis.clear();
	if (hashRedis.hmset(sGroupKey.c_str(), memberMap) == false)			    // Now start to insert	 
	{
		ErrLog("Failed to insert group %d , hmset error: %s",sGroupKey.c_str(),hashRedis.result_error());
		return false;
	}
	memberMap.clear();	

	return true;	
}

bool CRedisHelper::UpdateGroupCount(acl::redis_hash&      hashRedis,acl::redis& redis,acl::string sGroupKey,
								uint16_t nCount,uint8_t bMethod)
{
	const char* field[1];
	std::vector<acl::string> result;

	field[0] = sGroupFields[GROUPCOUNT_FIELD];

	result.clear();
	hashRedis.clear();
	if (hashRedis.hmget(sGroupKey, field, 1, &result) == false)
	{
		ErrLog("hmget error: %s", hashRedis.result_error());
		return false;
	}

	const char* val = hashRedis.result_value(0);
	if(!val)
	{
		WarnLog("The group %s is not exist!",sGroupKey.c_str());
		return false;
	}
	/////////////////////////////////////////////////////////////////////////
	int nGroupCount = bMethod ? (atoi(val) - nCount) : (atoi(val) + nCount);
	redis.clear();
	if(nGroupCount <= 0)  // delete the group if there is no any member in group. 
	{
		if(redis.del(sGroupKey.c_str())<=0)
		{
			ErrLog("del group: %s withd err code : %s!",
				sGroupKey.c_str(), redis.result_error());
			return false;	
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	hashRedis.clear();
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];

	snprintf(tBuf, sizeof(tBuf), "%d", nGroupCount);
	mFields[sGroupFields[GROUPCOUNT_FIELD]]= tBuf;


	if (hashRedis.hmset(sGroupKey.c_str(), mFields) == false)
	{
			ErrLog("Failed to increase/decrease group %s count , hmset error: %s",
				sGroupKey.c_str(),hashRedis.result_error());
			return false;
	}
	mFields.clear();	


	return true;

}


bool CRedisHelper::UpdateGroupStatus(acl::redis_hash& hashRedis, acl::redis& redis, acl::string sGroupKey, uint16_t status)
{
	std::vector<acl::string> result;
	result.clear();

	redis.clear();
	if (0 == status)  // delete the group if there is no any member in group. 
	{
		if (redis.del(sGroupKey.c_str()) <= 0)
		{
			ErrLog("del group: %s withd err code : %s!",
				sGroupKey.c_str(), redis.result_error());
			return false;
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	hashRedis.clear();
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];

	snprintf(tBuf, sizeof(tBuf), "%d", status);
	mFields[sGroupFields[GROUPSTATUS_FIELD]] = tBuf;


	if (hashRedis.hmset(sGroupKey.c_str(), mFields) == false)
	{
		ErrLog("Failed to set group %s state , hmset error: %s",
			sGroupKey.c_str(), hashRedis.result_error());
		return false;
	}
	mFields.clear();


	return true;
}


bool CRedisHelper::UpdateGroupMaster(acl::redis_hash&      hashRedis,acl::string sGroupKey,acl::string sMasterId)
{

	std::vector<acl::string> result;
	std::map<acl::string, acl::string> mFields;
	
	result.clear();
	hashRedis.clear();

	DbgLog("updateGroupmaster");
	mFields[sGroupFields[GROUPMASTER_FIELD]] = sMasterId;

	log("-------------------------- update group master begin--------------------");
	hashRedis.clear();
	if (hashRedis.hmset(sGroupKey.c_str(), mFields) == false)
	{
			ErrLog("Failed to update group %s master , hmset error: %s",
				sGroupKey.c_str(),hashRedis.result_error());
			return false;
	}
	mFields.clear();	
	log("-------------------------- update group master end--------------------");


	return true;

}

bool CRedisHelper::RemoveGroupMember(acl::redis_hash&      hashRedis,acl::redis& redis,acl::string sGroupKey,acl::string sMemberId)
{
	hashRedis.clear();
	redis.clear();

    log("-------------------------- RemoveGroupMember delete group begin--------------------");
	if(hashRedis.hlen(sGroupKey.c_str())<=1)
	{
		redis.del(sGroupKey.c_str());
        log("-------------------------- RemoveGroupMember delete group end--------------------");
		return true;
	}
	
    log("-------------------------- RemoveGroupMember delete member  begin--------------------");
	if(hashRedis.hdel(sGroupKey.c_str(),sMemberId.c_str())<=0)
	{
		ErrLog("remove group member: %s error: %s, Failed to remove or the key is not exist!",
			sMemberId.c_str(), hashRedis.result_error());
		return false;	
	}

	DbgLog("Del Group %s ,member %s redis ok !",sGroupKey.c_str(),sMemberId.c_str());
	return true;

}


