/*****************************************************************************************
Filename: cache.cpp
Author: jack			Version: im-1.0 		Date:2017/06/14
Description: 	实现用户登录信息缓存、更新、缓存记录失效管理。
*****************************************************************************************/

#include "cache.h"

const char* sRedisFields[REDIS_FIELDS_COUNT] =
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
	"loginTime",
	"deviceVoipToken",
	"callState"
};
const char* sDeviceUserFields =  "subscriber";
CCache::CCache(CConfigFileReader* pConfigReader)
	: m_pConfigReader(pConfigReader)
{
}
CCache::~CCache()
{
}
bool CCache::Initialize(void)
{

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
	
	m_hashRedis.set_cluster(&m_cluster,m_redisPara.nPoolSize); // default 100
	m_redis.set_cluster(&m_cluster, m_redisPara.nPoolSize);

	return true;
}

bool CCache::SetRedisPara(void)
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
	m_redisPara.sPassword = (NULL==pRedis) ? "" : pRedis;

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


bool CCache::GetUserStatus(acl::string sUserId,uint8_t &bStatus)
{
	const char* field[1];
	std::vector<acl::string> result;

	field[0] = sRedisFields[STATUS_FIELD];

	result.clear();
	m_hashRedis.clear();
	if (m_hashRedis.hmget(sUserId, field, 1, &result) == false)
	{
		ErrLog("hmget error: %s\r\n", m_hashRedis.result_error());
		return false;
	}
	const char* val = m_hashRedis.result_value(0);
	bStatus = val ? atoi(val) : 0;

	return true;
}

bool CCache::GetUserRecord(acl::string sUserId,UserCache_t &rec)
{
	std::map<acl::string, acl::string> result;
	std::map<acl::string, acl::string>::const_iterator item;
	int valid_inx = 0;


	result.clear();
	m_hashRedis.clear();
	if (m_hashRedis.hgetall(sUserId.c_str(), result) == false)
	{
		ErrLog("hgetall key: %s error!", sUserId.c_str());
		return false;
	}

	if(result.empty()||result.size()<REDIS_FIELDS_COUNT)
	{
		ErrLog("User %s is not exist or the record is not full!", 
			sUserId.c_str(),result.size());
		return false;
	}

	for(item = result.begin(); item != result.end(); item++)
	{
		if(item->first.equal(sRedisFields[ROLE_FIELD]))
		{
			rec.bRole = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[APPID_FIELD]))
		{
			//printf("bCustomService\n");
			rec.sAppId = item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[GROUPLIMIT_FIELD]))
		{
			rec.nGroupLimit = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[GROUPNUMBER_FIELD]))
		{
			rec.nGroupNumber = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[STATUS_FIELD]))
		{
			rec.bStatus = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[DEVICETYPE_FIELD]))
		{

			//printf("bDeviceType\n");
			rec.bDeviceType = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[RELOGIN_FIELD]))
		{
			//printf("bRelogin\n");
			rec.bRelogin = atoi(item->second.c_str());
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[LOGINTOKEN_FIELD]))
		{
			rec.sLoginToken = item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[DEVICETOKEN_FIELD]))
		{
			rec.sDeviceToken= item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[DEVICEVERSION_FIELD]))
		{
			rec.sDeviceVersion= item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[IP_FIELD]))
		{
			rec.sIPAddr= item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[PORT_FIELD]))
		{
			rec.nIPPort= atoi(item->second.c_str());	
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[RELOGINTIME_FIELD]))
		{
			rec.sReloginTime= item->second.c_str();
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[LOGINTIME_FIELD]))			
		{
			rec.sLoginTime= item->second.c_str(); 
			valid_inx++;
		}	
		else if (item->first.equal(sRedisFields[DEVICEVOIPTOKEN_FIELD]))
		{
			rec.sDeviceVoipToken = item->second.c_str();
			valid_inx++;
		}
	}

	if(valid_inx<REDIS_FIELDS_COUNT-2)	// -1：现存老的数据还没有deviceVoipToken和callState，故暂时忽略该字段的必须性
	{
		ErrLog("Error field in this record, key is=%s",sUserId.c_str());
		return false;
	}

	rec.sUserId = sUserId.c_str();
	
	return true;

}

bool CCache::GetDeviceRecord(acl::string sDeviceToken,DeviceCache_t &rec)
{
	std::map<acl::string, acl::string> result;
	//std::map<acl::string, acl::string>::const_iterator item;
	//int valid_inx = 0;


	result.clear();
	m_hashRedis.clear();
	if (m_hashRedis.hgetall(sDeviceToken.c_str(), result) == false)
	{
		ErrLog("hgetall key: %s error!", sDeviceToken.c_str());
		return false;
	}

	if(result.empty()||result.size()<1)
	{
		ErrLog("The device %s is not exist!", 
			sDeviceToken.c_str(),result.size());
		return false;
	}
	
	std::map<acl::string, acl::string>::const_iterator it;
	
	it = result.find(sDeviceUserFields);
	if(it != result.end())
	{
		rec.sSubscriptId = it->second.c_str();
		rec.sDeviceToken = sDeviceToken.c_str();
		return true;
	}

	return false;

}

bool CCache::SetUserStatus(acl::string sUserId,int8_t bStatus)
{
	std::map<acl::string, acl::string> mFields;

	mFields[sRedisFields[STATUS_FIELD]]= (bStatus) ? '1' : '0';

	m_hashRedis.clear();
	if (m_hashRedis.hmset(sUserId.c_str(), mFields) == false)
	{
			ErrLog("Failed to set user %s status , hmset error: %s",
				sUserId.c_str(),m_hashRedis.result_error());
			return false;
	}
	mFields.clear();
	// reset the user expire time as the status is changed . 
	(bStatus) ? RemoveUserExpireTime(sUserId.c_str()) : SetUserExpireTime(sUserId.c_str());

	return true;
}

bool CCache::UpdateUserRec(UserCache_t &rec,bool bAuthCheck)
{
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];
	
	memset(tBuf,0,30);
	if(!bAuthCheck)
	{
		snprintf(tBuf, sizeof(tBuf), "%d", rec.bRole);				  //Refresh the role to cache
		mFields[sRedisFields[ROLE_FIELD]]=tBuf;
		mFields[sRedisFields[APPID_FIELD]]=rec.sAppId;
		snprintf(tBuf, sizeof(tBuf), "%d", rec.nGroupLimit);				  //Refresh the role to cache
		mFields[sRedisFields[GROUPLIMIT_FIELD]]=tBuf;
		snprintf(tBuf, sizeof(tBuf), "%d", rec.nGroupNumber);				  //Refresh the role to cache
		mFields[sRedisFields[GROUPNUMBER_FIELD]]=tBuf;
	}
	mFields[sRedisFields[STATUS_FIELD]]="1";
	snprintf(tBuf, sizeof(tBuf), "%d", rec.bDeviceType);
	mFields[sRedisFields[DEVICETYPE_FIELD]]=tBuf;
	mFields[sRedisFields[RELOGIN_FIELD]]='1';
	mFields[sRedisFields[LOGINTOKEN_FIELD]]=rec.sLoginToken;
	mFields[sRedisFields[DEVICETOKEN_FIELD]]=rec.sDeviceToken;
	mFields[sRedisFields[DEVICEVERSION_FIELD]]=rec.sDeviceVersion;
	mFields[sRedisFields[IP_FIELD]]=rec.sIPAddr;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nIPPort);
	mFields[sRedisFields[PORT_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%d",(int)time(0));
	mFields[sRedisFields[RELOGINTIME_FIELD]]=tBuf;
	mFields[sRedisFields[DEVICEVOIPTOKEN_FIELD]] = rec.sDeviceVoipToken;
	mFields[sRedisFields[LOGININFO_CALLSTATE_FIELD]] = "0";

	m_hashRedis.clear();
	if (m_hashRedis.hmset(rec.sUserId.c_str(), mFields) == false)
	{
		ErrLog("Failed to set user %s relogin , hmset error: %s \n",
				rec.sUserId.c_str(),m_hashRedis.result_error());
		return false;
	}
	mFields.clear();

	RemoveUserExpireTime(rec.sUserId.c_str());

	return true;	
}

bool CCache::DeviceSynchronize(acl::string sUserId,uint8_t bDeviceType,\
	acl::string sDeviceVersion, acl::string sDeviceToken)
{
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];
		
	memset(tBuf,0,30);
	snprintf(tBuf, sizeof(tBuf), "%d", bDeviceType);
	mFields[sRedisFields[DEVICETYPE_FIELD]]=tBuf;
	mFields[sRedisFields[DEVICETOKEN_FIELD]]=sDeviceToken;
	mFields[sRedisFields[DEVICEVERSION_FIELD]]=sDeviceVersion;	

	m_hashRedis.clear();
	if (m_hashRedis.hmset(sUserId.c_str(), mFields) == false)
	{
		ErrLog("Failed to synchronize user %s device informationn  , hmset error: %s \n",
				sUserId.c_str(),m_hashRedis.result_error());
		return false;
	}
	mFields.clear();
	
	return true;
}

bool CCache::DeviceSynchronize(acl::string sUserId, acl::string sDeviceVoipToken)
{
	std::map<acl::string, acl::string> mFields;

	mFields[sRedisFields[DEVICEVOIPTOKEN_FIELD]] = sDeviceVoipToken;

	m_hashRedis.clear();
	if (m_hashRedis.hmset(sUserId.c_str(), mFields) == false)
	{
		ErrLog("Failed to synchronize user %s device voip informationn  , hmset error: %s \n",
			sUserId.c_str(), m_hashRedis.result_error());
		return false;
	}
	mFields.clear();

	return true;
}

bool CCache::InsertUserRec(UserCache_t &rec)							 // Insert a new user record into redis cache 
{

	acl::string key;
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];

	memset(tBuf,0,30);
	snprintf(tBuf, sizeof(tBuf), "%d", rec.bRole);
	mFields[sRedisFields[ROLE_FIELD]]=tBuf;
	mFields[sRedisFields[APPID_FIELD]]=rec.sAppId;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nGroupLimit);
	mFields[sRedisFields[GROUPLIMIT_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nGroupNumber);
	mFields[sRedisFields[GROUPNUMBER_FIELD]]=tBuf;
	mFields[sRedisFields[STATUS_FIELD]]="1";
	snprintf(tBuf, sizeof(tBuf), "%d", rec.bDeviceType);
	mFields[sRedisFields[DEVICETYPE_FIELD]]=tBuf;
	mFields[sRedisFields[RELOGIN_FIELD]]="0";
	mFields[sRedisFields[LOGINTOKEN_FIELD]]=rec.sLoginToken;
	mFields[sRedisFields[DEVICETOKEN_FIELD]]=rec.sDeviceToken;
	mFields[sRedisFields[DEVICEVERSION_FIELD]]=rec.sDeviceVersion;
	mFields[sRedisFields[IP_FIELD]]=rec.sIPAddr;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nIPPort);
	mFields[sRedisFields[PORT_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%d",(int)time(0));
	mFields[sRedisFields[RELOGINTIME_FIELD]]=tBuf;
	mFields[sRedisFields[LOGINTIME_FIELD]]= tBuf;
	mFields[sRedisFields[DEVICEVOIPTOKEN_FIELD]] = rec.sDeviceVoipToken;
	mFields[sRedisFields[LOGININFO_CALLSTATE_FIELD]] = "0";

	key=rec.sUserId;

	m_hashRedis.clear();
	if (m_hashRedis.hmset(key.c_str(), mFields) == false)			    // Now start to insert	 
	{
		ErrLog("Failt to insert user %s rec , hmset error: %s",
				key.c_str(),m_hashRedis.result_error());
		return false;
	}
	mFields.clear();

	RemoveUserExpireTime(key.c_str());

	return true;

}

bool CCache::InsertDeviceRec(DeviceCache_t &rec)							 // Insert a new user record into redis cache 
{

	std::map<acl::string, acl::string> mFields;
	DbgLog("insert Device %s",rec.sDeviceToken.c_str());
	mFields[sDeviceUserFields] = rec.sSubscriptId;
	m_hashRedis.clear();
	if (m_hashRedis.hmset(rec.sDeviceToken.c_str(), mFields) == false)			    // Now start to insert	 
	{
		ErrLog("Failt to insert Device %s rec , hmset error: %s",
			rec.sDeviceToken.c_str(),m_hashRedis.result_error());
		return false;
	}
	mFields.clear();

	
	return true;
}

bool CCache::DelUserRec(acl::string sUserId)								// Delete the specified cache record.
{
	m_redis.clear();
	
	if(m_redis.del(sUserId.c_str())<=0)
	{
		ErrLog("del key: %s error: %s, Failed to del the key or the key is not exist!",
			sUserId.c_str(), m_hashRedis.result_error());
		return false;	
	}

	DbgLog("Del redis ok : %s",sUserId.c_str());

	return true;
}

bool CCache::DelDeviceRec(acl::string sDeviceToken)	
{
	m_redis.clear();
	DbgLog("del Device %s",sDeviceToken.c_str());
	if(m_redis.del(sDeviceToken.c_str())<=0)
	{
		ErrLog("Failed to del the key %s with error %s!",
			sDeviceToken.c_str(), m_hashRedis.result_error());
		return false;	
	}

	return true;
}


bool CCache::SetUserExpireTime(acl::string sUserId)
{
	m_redis.clear();
	int nExpireSecond;

	nExpireSecond = m_redisPara.nKeyExpireTime*3600;						//Expire interval . 
	if (m_redis.expire(sUserId.c_str(), nExpireSecond) < 0)				//The cache key will lost over expire interval. 
	{
		ErrLog("expire key: %s error: %s", sUserId.c_str(),
			m_redis.result_error());
			return false;
	}
	
	DbgLog("Set user = %s expire time = %d",sUserId.c_str(),nExpireSecond);
	return true;
}
	
bool CCache::RemoveUserExpireTime(acl::string sUserId)
{
	m_redis.clear();

	if (m_redis.persist(sUserId.c_str()) < 0)								//Convert the cache key into normal state, 
	{																		//and the expire interval will be reset
		ErrLog("expire key: %s error: %s\r\n", sUserId.c_str(),			
			m_redis.result_error());
			return false;
	}

	return true;
}

