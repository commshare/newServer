#include "loginCache.h"

const char* sRedisFields[REDIS_FIELDS_COUNT] =
{
	"cm_ip",
	"cm_port",
	"status",
	"login_token",
	"device_token",
	"relogin",
	"login_time",
	"relogin_time",
	"call_status",
	"session_id",
	"push_type",
	"push_token",
	"is_push"
};
const char* sDeviceUserFields =  "subscriber";
CLoginCache::CLoginCache(CConfigFileReader* pConfigReader)
	: m_pConfigReader(pConfigReader)
{
}
CLoginCache::~CLoginCache()
{
}
bool CLoginCache::Initialize(void)
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

bool CLoginCache::SetRedisPara(void)
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

	pRedis=m_pConfigReader->GetConfigName("redis_key_expiretime");
	if(NULL==pRedis) 
		return false;
	m_redisPara.nKeyExpireTime=atoi(pRedis);
		
	return true;

}


bool CLoginCache::GetUserStatus(acl::string sUserId,uint8_t &bStatus)
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

bool CLoginCache::GetUserRecord(acl::string sUserId,UserCache_t &rec)
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

	if(result.empty())
	{
		ErrLog("User %s is not exist or the record is not full! size %d", 
			sUserId.c_str(), result.size());
		return false;
	}

	for(item = result.begin(); item != result.end(); item++)
	{
		if(item->first.equal(sRedisFields[STATUS_FIELD]))
		{
			rec.bStatus = atoi(item->second.c_str());
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
		else if(item->first.equal(sRedisFields[SESSION_ID_FIELD]))			
		{
			rec.sSessionId= item->second.c_str(); 
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[PUSHTYPE_FIELD]))
		{
			rec.nPushType= atoi(item->second.c_str());	
			valid_inx++;
		}
		else if(item->first.equal(sRedisFields[PUSHTOKEN_FIELD]))
		{
			rec.sPushToken= item->second.c_str();
			valid_inx++;
		}
		
	}
	
	rec.sUserId = sUserId.c_str();
	return true;

}

bool CLoginCache::GetDeviceRecord(acl::string sDeviceToken,DeviceCache_t &rec)
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

bool CLoginCache::SetUserStatus(acl::string sUserId,int8_t bStatus)
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

bool CLoginCache::UpdateUserRec(UserCache_t &rec)
{
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];
	memset(tBuf,0,30);
	mFields[sRedisFields[STATUS_FIELD]]="1";
	mFields[sRedisFields[RELOGIN_FIELD]]='1';
	mFields[sRedisFields[LOGINTOKEN_FIELD]]=rec.sLoginToken;
	mFields[sRedisFields[DEVICETOKEN_FIELD]]=rec.sDeviceToken;
	mFields[sRedisFields[IP_FIELD]]=rec.sIPAddr;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nIPPort);
	mFields[sRedisFields[PORT_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%d",(int)time(0));
	mFields[sRedisFields[RELOGINTIME_FIELD]]=tBuf;
	mFields[sRedisFields[SESSION_ID_FIELD]]=rec.sSessionId;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nPushType);
	mFields[sRedisFields[PUSHTYPE_FIELD]]=tBuf;
	mFields[sRedisFields[PUSHTOKEN_FIELD]]=rec.sPushToken;
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

bool CLoginCache::InsertUserRec(UserCache_t &rec)							 // Insert a new user record into redis cache 
{

	acl::string key;
	std::map<acl::string, acl::string> mFields;
	char tBuf[30];
	memset(tBuf,0,30);
	mFields[sRedisFields[STATUS_FIELD]]="1";
	mFields[sRedisFields[RELOGIN_FIELD]]="0";
	mFields[sRedisFields[LOGINTOKEN_FIELD]]=rec.sLoginToken;
	mFields[sRedisFields[DEVICETOKEN_FIELD]]=rec.sDeviceToken;
	mFields[sRedisFields[IP_FIELD]]=rec.sIPAddr;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nIPPort);
	mFields[sRedisFields[PORT_FIELD]]=tBuf;
	snprintf(tBuf, sizeof(tBuf), "%d",(int)time(0));
	mFields[sRedisFields[RELOGINTIME_FIELD]]=tBuf;
	mFields[sRedisFields[LOGINTIME_FIELD]]= tBuf;
	mFields[sRedisFields[SESSION_ID_FIELD]]=rec.sSessionId;
	snprintf(tBuf, sizeof(tBuf), "%d", rec.nPushType);
	mFields[sRedisFields[PUSHTYPE_FIELD]]=tBuf;
	mFields[sRedisFields[PUSHTOKEN_FIELD]]=rec.sPushToken;

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

bool CLoginCache::InsertDeviceRec(DeviceCache_t &rec)							 // Insert a new user record into redis cache 
{

	std::map<acl::string, acl::string> mFields;
	DbgLog("insert Device %s userid %s",rec.sDeviceToken.c_str(), rec.sSubscriptId.c_str());
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

bool CLoginCache::DelUserRec(acl::string sUserId)								// Delete the specified cache record.
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

bool CLoginCache::DelDeviceRec(acl::string sDeviceToken)	
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

bool CLoginCache::SetUserPushState(acl::string sUserId, int push)
{
	m_hashRedis.clear();
	if (m_hashRedis.hset(sUserId.c_str(), sRedisFields[ISPUSH_FIELD], std::to_string(push).c_str()) == false)
	{
		ErrLog("Failed to set user %s push status , hset error: %s", sUserId.c_str(), m_hashRedis.result_error());
		return false;
	}
	return true;
}



bool CLoginCache::SetUserExpireTime(acl::string sUserId)
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
	
bool CLoginCache::RemoveUserExpireTime(acl::string sUserId)
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


