/*****************************************************************************************
Filename: usermanage.h
Author: jack			Version: im-1.0 		Date:2017/06/14
Description: 	用户登录/登出、权限检查、用户信息管理类实现
*****************************************************************************************/

#include "usermanage.h"

//string gSessionTestId;
static const regex rUserIdFilters("^[0-9]{5,10}$"); 
CConfigFileReader* m_pConfigReader; 
string m_sCheckAuthUrl;
string m_sAppSecretChecking;  // App secret code for checking. 
uint32_t m_nCheckInterval;
CCache* m_pCache;		

CUserManage::CUserManage(CConfigFileReader* pConfigReader,int nNumOfInst)
	: m_pConfigReader(pConfigReader),m_sCheckAuthUrl(""),m_sAppSecretChecking(""),
	 m_nCheckInterval(0),m_pCache(0),m_nNumberOfInst(nNumOfInst)
	
{

}

CUserManage::~CUserManage()
{
	if(m_pCache)
	{
		delete m_pCache;
	}
}

bool CUserManage::Initialize(void)  
{
	//Add your statment to load config parameter about user manage.
	if(SetCheckAuth()==false)				//Load config parameter from config file.
		return false;
	
	m_pCache = new CCache(m_pConfigReader);		// Redis cache instance definition
	if(!m_pCache)
		return false;
	if(false == m_pCache->Initialize())
	{
		ErrLog("Err of redis connection!");
		return false;
	}

	RegistPacketExecutor();
	StartThread();
	
	return true;
}

bool CUserManage::SetCheckAuth()
{
	if(!m_pConfigReader)
		return false;

	
	char* pAuth =	m_pConfigReader->GetConfigName("check_interval"); 
	if(pAuth==NULL)
		return false;
	m_nCheckInterval = atoi(pAuth);

	pAuth =	m_pConfigReader->GetConfigName("check_appsecret"); 
	if(pAuth==NULL)
		return false;
	m_sAppSecretChecking = pAuth;

	
	pAuth = 	m_pConfigReader->GetConfigName("check_url");
	if(pAuth==NULL)
		return false;
	m_sCheckAuthUrl = pAuth;

	return true;
}

bool CUserManage::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(CM_LOGIN,m_nNumberOfInst, CommandProc(&CUserManage::OnLogin));
	CmdRegist(CM_LOGOUT, m_nNumberOfInst,CommandProc(&CUserManage::OnLogout));
	//CmdRegist(CM_LOGOUT_CONFIRM, m_nNumberOfInst, CommandProc(&CUserManage::OnLogoutConfirm)); //cancel CM_LOGOUT_CONFIRM
	CmdRegist(SYSTEM_TIMEOUT_NOTIFICATION, m_nNumberOfInst,CommandProc(&CUserManage::OnClose));
	CmdRegist(CM_DEVICETOKENSYNC, m_nNumberOfInst,CommandProc(&CUserManage::OnDeviceTokenSync));

	return true;
}

string  CUserManage::RandStr(void)
{
	char buf[64] = {0};
	string str;
	
	srand( (unsigned)time( NULL ) );
	sprintf(buf, "%d",rand());
	str = buf;

	return str;
}

string  CUserManage::MD5Sign(string sNonceStr, string sToken, string userid)
{
	string sVerifyCode = m_sAppSecretChecking + "app_id1" + "formatjson" + "nonce" + \
            sNonceStr + "sign_methodmd5" + "token" + sToken + "userid" + userid + m_sAppSecretChecking;
//app_id=1&nonce=%s&sign=%s&sign_method=md5&token=%s&userid=%s
	unsigned char md5_buf[128] = {0};
	char md5_str_buf[128] = {0};
	string md5_str;
	int i;
	
	md5((unsigned char*)sVerifyCode.c_str(), sVerifyCode.size(), md5_buf);
	for (i = 0; i < 16; i++) 
	{
        sprintf(md5_str_buf + 2 * i, "%02X", md5_buf[i]);
	}
	md5_str = md5_str_buf;
    DbgLog("md5_str=<%s>", md5_str.c_str());
    return md5_str;
}

void CUserManage::UrlSign(string sUserId,string sToken,string& sSignUrl)
{
	string sNonceStr = RandStr();
	string sign_str;
	char url_sign_buf[1024] = {0};

    sign_str = MD5Sign(sNonceStr,sToken, sUserId);
    sprintf(url_sign_buf,"app_id=1&format=json&nonce=%s&sign=%s&sign_method=md5&token=%s&userid=%s",
		sNonceStr.c_str(),sign_str.c_str(),sToken.c_str(),sUserId.c_str());
	sSignUrl = url_sign_buf;
}

bool CUserManage::VerifyDeviceToken(string sDeviceToken)
{
	//DbgLog("device verify length = %d",sDeviceToken.find(':',0));
	if(sDeviceToken.empty() || (int) sDeviceToken.find(':',0) < PREFIX_DEVICELEN_REF) // == -1)
		return false;
	
	return true;
}

bool CUserManage::CheckCacheDevice(string sDevice, string sCacheDevice)
{
	int nDeviceIndex = sDevice.find(':',0);
	int nCacheDeviceIndex = sCacheDevice.find(':',0);

	string sPrefixDevice = sDevice.substr(0,nDeviceIndex);
	string sPrefixCacheDevice = sCacheDevice.substr(0,nCacheDeviceIndex);

	if(!sPrefixDevice.compare(sPrefixCacheDevice.c_str()))
		return true;

	return false;
}

bool CUserManage::CheckAuth(UserAuth_t& auth) //Checking user 
{
	auth.errCode = ERR_LOGIN_AUTH;

	if(m_sCheckAuthUrl=="")
	{
		ErrLog("The check url can not be null!");
		return false;
	}
	
	CHttpClient httpClient;
	Json::Reader reader;
    Json::Value root;
	
	string sChkUrl = m_sCheckAuthUrl;
	string sSignStr;
	string sChkResult;
	string sReturnCode;
	
	
	UrlSign(auth.sUserId,auth.sLoginToken,sSignStr);
	sChkUrl  += sSignStr; 
    //InfoLog("The auth check request is :%s ", sChkUrl.c_str());
	if(httpClient.Get(sChkUrl, sChkResult) != CURLE_OK)
		return false;

	InfoLog("The auth check result is :%s ", sChkResult.c_str());
    if(!reader.parse(sChkResult, root, false))
   		return false;
   
    sReturnCode = root["status_code"].asString();
    if(!strcmp(sReturnCode.c_str(),"200"))
    {
    	//DbgLog("check auth return data :%d,%d",root["data"]["userid"].asUInt(),root["data"]["role_id"].asUInt());
    	auth.bRole = root["data"]["role_id"].asUInt();
		auth.sAppId = root["data"]["appid"].asString();  
		auth.nGroupLimit = MAX_GROUPLIMIT; // 
		auth.nGroupNumber = 0;
		auth.errCode = NON_ERR;
	   	return true;
    }
	else if(!strcmp(sReturnCode.c_str(),"501"))
	{
		DbgLog("Auth result %s",root.toStyledString().c_str());
		auth.errCode = ERR_CM_AUTH_KICKEDOUT;
		auth.nLastLoginTime = root["last_login_time"].asUInt64();
		return false;
	}

    return false;

}
	
bool CUserManage::CheckAuthInterval(UserCache_t userCache,string sTokens) //Check whether or not need to auth user this time. 
{
	int currTime = time(0);
	int preTime;
	
	if(sTokens.compare(userCache.sLoginToken.c_str()))  // the token is diffrent with cache sToken, 
	{												  //  the cache sToken maybe expired time. 	
		WarnLog("Failed to match token");
		return false;
	}
	
	preTime = (userCache.bRelogin) ? atoi(userCache.sReloginTime) : atoi(userCache.sLoginTime);
	
	return ((currTime-preTime)>=m_nCheckInterval) ? false : true;
}

// kickout the same user when logining
bool CUserManage::Kickout(UidCode_t currentSessionId,UserCache_t userCache)
{
	CClientLink* pCacheLink = NULL;
	CClientLink* pCurLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(currentSessionId);
	DbgLog("kickout checking ...");

	
	if(!pCurLink)
	{
		DbgLog("Failed to kickout , cachelink = %p,currentlink = %p",pCacheLink, pCurLink);
		return false;
	}
	
	#if 0
	//if((pCacheLink==pCurLink)||!pCacheLink||!pCurLink ||(pCacheLink && pCurLink && 
	//	!sCurrentUserId.compare(userCache.sUserId.c_str()) && 
	//	!pCurLink->GetPeerIp().compare(pCacheLink->GetPeerIp().c_str())))
	//{
	//	WarnLog("Encounter an unnecessary relogin when processing a user logining!");
	//	return false;
	//}
	#endif
	
	CImPdu kickoutPDU;
	CMKickoutNotification kickoutUser;
	string sIP="";
	uint16_t nPort = 0;
	string sKickUserId = userCache.sUserId.c_str();
	
	//kickoutUser.set_suserid(userCache.sUserId.c_str());
	kickoutUser.set_suserid(sKickUserId.substr(POSTFIX_CM_POS));
	kickoutUser.set_ndevicetype(userCache.bDeviceType);
	kickoutUser.set_ip(userCache.sIPAddr.c_str());
	kickoutUser.set_port(userCache.nIPPort);
	kickoutPDU.SetPBMsg(&kickoutUser);
	kickoutPDU.SetCommandId(CM_KICKOUT_NOTIFICATION);
	//kickoutPDU.SetSessionId(pCacheLink->GetSessionId());
	kickoutPDU.SetSessionId(currentSessionId);
	CClientLinkMgr::GetInstance()->GetLocalHost(sIP,nPort);
	
	if(!sIP.compare(userCache.sIPAddr.c_str())&&(nPort == userCache.nIPPort))  
	{ //Directly kickout the old (cached)link since the old link logined in the same CM server. 
		pCacheLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(userCache.sUserId.c_str());
		if(pCacheLink)
		{
			kickoutPDU.SetSessionId(pCacheLink->GetSessionId());
			DbgLog("kickout userid %s at the same cm server ...",sKickUserId.substr(3).c_str());
			SendPdu(userCache.sUserId.c_str(),&kickoutPDU); 
			DbgLog("closing old %p ... ",pCacheLink); 
		//CAutoLock autolock(CClientLinkMgr::GetInstance()->GetCliLock());
			pCacheLink->CloseLink(); //Close the old link since it has aleady kickout. 
			DbgLog("close ok!");
		}
	} 
	else
	{ // Send the kick out notification by msg server since the old link logined at other CM server. 
		DbgLog("kickout userid %s at other CM server ... ",sKickUserId.substr(POSTFIX_CM_POS).c_str());
		kickoutPDU.SetSessionId(currentSessionId);
		SendPdu(MSG,&kickoutPDU);
	}

	if(pCurLink) {pCurLink->ReleaseRef();}
	if(pCacheLink) {pCacheLink->ReleaseRef();}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Save user newest information into redis cache when the user login at the first time.  
//////////////////////////////////////////////////////////////////////////////////////////////////

bool CUserManage::SaveUserInfo(CMLogin login,string sUserId,UserAuth_t userAuth) //Save the logining user information to redis cache. 
{
	UserCache_t userCache;
	DeviceCache_t  deviceCache;
	string sCacheUser = PREFIX_CM_STR;
	string sIP = "";
	string sDeviceId = "";
	uint16_t nPort = 0;

	userCache.sUserId = sUserId.c_str();
	userCache.bRole = userAuth.bRole;
	userCache.sAppId = userAuth.sAppId.c_str();
	userCache.nGroupLimit = userAuth.nGroupLimit;
	userCache.nGroupNumber = userAuth.nGroupNumber;
	userCache.bDeviceType = login.ndevicetype();
	userCache.bStatus = 1;
	CClientLinkMgr::GetInstance()->GetLocalHost(sIP,nPort);
	userCache.sIPAddr = sIP.c_str();
	userCache.nIPPort = nPort;
  	userCache.sLoginToken = login.susertoken().c_str();
	userCache.sDeviceToken = login.sdevicetoken().c_str();
	userCache.sDeviceVersion = login.sdeviceversion().c_str();
	userCache.sDeviceVoipToken = login.sdevicevoiptoken().c_str();
	
	if(m_pCache->InsertUserRec(userCache)==false)      //Now insert cache information .
	{
		ErrLog("Failed to insert userinfo to redis!");
		return false;
	}

	sDeviceId = login.sdevicetoken().substr(0,login.sdevicetoken().find(':',0));
	deviceCache.sDeviceToken = PREFIX_CMDT_STR;
	deviceCache.sDeviceToken += sDeviceId.c_str();

	if(true == m_pCache->GetDeviceRecord(deviceCache.sDeviceToken,deviceCache))
	{
		sCacheUser += deviceCache.sSubscriptId.c_str();
		if(sCacheUser.compare(sUserId.c_str()) != 0)  // Means other user using the device last time. 
		{
			DbgLog("Delete cache user %s",sCacheUser.c_str());
			m_pCache->DelUserRec(sCacheUser.c_str());   //Delete the cached user information since a new user loged in with the same device . 
			//m_pCache->
		}
	}	

	deviceCache.sSubscriptId = sUserId.substr(POSTFIX_CM_POS).c_str();
 	m_pCache->InsertDeviceRec(deviceCache);

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// Update user newest information into redis cache. 
//////////////////////////////////////////////////////////////////////////////////////////////////
bool CUserManage::UpdateUserInfo(CMLogin login,string sUserId,string sCacheDeviceId,UserAuth_t userAuth,bool bCheck)
{
	UserCache_t userCache;
	DeviceCache_t  deviceCache;
	string sCacheUser = PREFIX_CM_STR;
	string sCacheDeviceToken = PREFIX_CMDT_STR;
	string sDeviceId;
	string sIP = "";
	uint16_t nPort = 0;
	
	userCache.sUserId = sUserId.c_str();
	userCache.bRole = userAuth.bRole;
	userCache.sAppId = userAuth.sAppId.c_str();
	userCache.nGroupLimit = userAuth.nGroupLimit;
	userCache.nGroupNumber = userAuth.nGroupNumber;
	userCache.bDeviceType = login.ndevicetype();
	userCache.bStatus = 1;
	CClientLinkMgr::GetInstance()->GetLocalHost(sIP,nPort);
	userCache.sIPAddr = sIP.c_str();
	userCache.nIPPort = nPort;
	userCache.sLoginToken = login.susertoken().c_str();
	userCache.sDeviceToken = login.sdevicetoken().c_str();
	userCache.sDeviceVersion = login.sdeviceversion().c_str();
	userCache.sDeviceVoipToken = login.sdevicevoiptoken().c_str();
	//int nDeviceIndex =  login.sdevicetoken().IndexOf(":");
	
	


	if(m_pCache->UpdateUserRec(userCache,bCheck)==false)
	{
		ErrLog("Failed to update userinfo to redis!");
		return false;
	}

	
	//if(sCacheDeviceId.compare(userCache.sDeviceToken.c_str())!=0)
	if(!CheckCacheDevice(sCacheDeviceId, userCache.sDeviceToken.c_str()))
	{
		//sCacheDeviceToken += sCacheDeviceId.c_str();
		sCacheDeviceToken += sCacheDeviceId.substr(0,sCacheDeviceId.find(':',0)).c_str();
		m_pCache->DelDeviceRec(sCacheDeviceToken.c_str());		// Delete the cached device token at last time login . 	
	}

	sDeviceId = login.sdevicetoken().substr(0,login.sdevicetoken().find(':',0));
	
	deviceCache.sDeviceToken = PREFIX_CMDT_STR;
	deviceCache.sDeviceToken += sDeviceId.c_str();
	deviceCache.sSubscriptId = sUserId.substr(POSTFIX_CM_POS).c_str();
	m_pCache->InsertDeviceRec(deviceCache);						// Update the device token cache. 
	return true;
}

void CUserManage::UpdateUserLink(UidCode_t currentSessionId,string sUserId)
{
	CClientLink* pLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(currentSessionId);
	if(pLink)
	{
		pLink->UserArrived(sUserId);
		pLink->ReleaseRef();
	}
}


bool CUserManage::OnLogin(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing user login!");
		return false;
	}

	string sUserId = PREFIX_CM_STR;
	//string sCachedDevice = PREFIX_CMDT_STR;
	string sCachedDevice;
	UidCode_t sessionId;
	uchar_t* pContent = NULL;
	bool bAuthCheck = false;
	CMLogin 	login;
	UserAuth_t  userAuth;
	UserCache_t userCache;
	CClientLink* pCacheLink = NULL;
	CClientLink* pCurrentLink = NULL;
	
	
	pContent = pPdu->GetBodyData();
	sessionId = pPdu->GetSessionId();
	pCurrentLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
	
	if(!pContent || !login.ParseFromArray(pContent,pPdu->GetBodyLength()) ||
	(login.suserid().empty() || !regex_match(login.suserid(),rUserIdFilters)) || 
	!VerifyDeviceToken(login.sdevicetoken()))
	{
		ErrLog("Login parameter error!");
		LoginRsp("",sessionId,ERR_LOGIN_FORBIDDEN);
		if(pCurrentLink) {pCurrentLink->CloseLink();pCurrentLink->ReleaseRef();}
		return false;
	}

	userAuth.sUserId = login.suserid();			//Pure userid ,
	userAuth.sLoginToken = login.susertoken();
	userAuth.bRole = 0;
	sUserId += userAuth.sUserId;				//UserId with 'CM' plugin to use in redis cache. 
	
	
	
	InfoLog("User %s arrived ! cmd = 0x%x ,size = %d,InstId=%d",sUserId.c_str(),
		pPdu->GetCommandId(),pPdu->GetBodyLength(),m_nNumberOfInst);
	
	
	if(m_pCache->GetUserRecord(sUserId.c_str(),userCache)==true)
	{
		pCacheLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(userCache.sUserId.c_str());
	 

		
		//if(pCurrentLink && pCacheLink && userCache.bStatus && (!sUserId.compare(userCache.sUserId.c_str()) && 
		//	 !pCurrentLink->GetPeerIp().compare(pCacheLink->GetPeerIp().c_str()))&& (pCacheLink!=pCurrentLink))
		#if 0   //masked at 2018.1.13 for optimizing user login system. 
		if(pCurrentLink && pCacheLink && (pCacheLink!=pCurrentLink)&&userCache.bStatus && 
			CheckCacheDevice(login.sdevicetoken(),userCache.sDeviceToken.c_str()))
			//(!login.sdevicetoken().compare(userCache.sDeviceToken.c_str())))
		
		{
			ErrLog("Encounter an unnecesary login when processing user %s login!",userAuth.sUserId.c_str());
			//LoginRsp(login.suserid(),sessionId,ERR_LOGIN_FORBIDDEN);
			//CAutoLock autolock(CClientLinkMgr::GetInstance()->GetCliLock());
			pCurrentLink->CloseLink(false);

			return false;
		}
			
		else if( pCurrentLink && userCache.bStatus && (pCurrentLink == pCacheLink))			
		{
			WarnLog("The user %s has already logined in the system,Unnecessary login again!",sUserId.c_str()); 
			pCurrentLink->ResetLogin();
			return false;
		}
		#else
		if( pCurrentLink && userCache.bStatus && (pCurrentLink == pCacheLink))			
		{
			WarnLog("The user %s has already logined in the system,Unnecessary login again!",sUserId.c_str()); 
			pCurrentLink->ResetLogin();
			pCurrentLink->ReleaseRef();
			if(pCacheLink){pCacheLink->ReleaseRef();}
			return false;
		}

		
		#endif
		if(login.sdevicetoken().compare(userCache.sDeviceToken.c_str()))
		{	
			//token is different need to check Auth
			if(false == CheckAuth(userAuth))
			{
				//if(userCache.bStatus!=USER_OFFLINE)
				//{
				//	m_pCache->SetUserStatus(sUserId.c_str(),0);
				//}
				LoginRsp(login.suserid(),sessionId,userAuth.errCode,userAuth.nLastLoginTime);
				ErrLog("Failed to re-check auth : %s,%s! errcode:%lu",userAuth.sUserId.c_str(),userAuth.sLoginToken.c_str(),userAuth.errCode);
				//pCurrentLink->ResetLogin();
				if(pCurrentLink) {pCurrentLink->CloseLink();pCurrentLink->ReleaseRef();}
				if(pCacheLink){pCacheLink->ReleaseRef();}
				return false;
			}
		}
		else if((bAuthCheck=CheckAuthInterval(userCache,userAuth.sLoginToken))==false)
		{	
			//token is same but CheckAuthInterval timeout need to check Auth
			if(false == CheckAuth(userAuth))
			{
				//if(userCache.bStatus!=USER_OFFLINE)
				//{
				//	m_pCache->SetUserStatus(sUserId.c_str(),0);
				//}
				LoginRsp(login.suserid(),sessionId,userAuth.errCode,userAuth.nLastLoginTime);
				ErrLog("Failed to re-check auth : %s,%s! errcode:%lu",userAuth.sUserId.c_str(),userAuth.sLoginToken.c_str(),userAuth.errCode);
				//pCurrentLink->ResetLogin();
				if(pCurrentLink) {pCurrentLink->CloseLink();pCurrentLink->ReleaseRef();}
				if(pCacheLink){pCacheLink->ReleaseRef();}
				return false;
			}
		}
		DbgLog("Current user %s status %d",userCache.sUserId.c_str(),userCache.bStatus);

		
		if(pCurrentLink && (pCacheLink!=pCurrentLink)&&userCache.bStatus && 
			!CheckCacheDevice(login.sdevicetoken(),userCache.sDeviceToken.c_str()))
		//if(	userCache.bStatus!=USER_OFFLINE) 
		{
			Kickout(pPdu->GetSessionId(),userCache);
		}
			
		if(!UpdateUserInfo(login,sUserId,userCache.sDeviceToken.c_str(),userAuth,bAuthCheck))
		{
			LoginRsp(login.suserid(),sessionId,ERR_CM_EXCEPTION);
			WarnLog("User %s authorized successfully when relogin ,but encounter resource exception!",userAuth.sUserId.c_str());
			//pCurrentLink->ResetLogin();
			if(pCurrentLink) {pCurrentLink->CloseLink();pCurrentLink->ReleaseRef();}
			if(pCacheLink){pCacheLink->ReleaseRef();}
			return false;
		}
	}
	else   //New user login  or User record is abnormal. 
	{
		InfoLog("user %s is a new login member", userAuth.sUserId.c_str());

		if(false == CheckAuth(userAuth))
		{
			LoginRsp(login.suserid(),sessionId,userAuth.errCode,userAuth.nLastLoginTime);
			WarnLog("Failed to new-authorize : %s errcode:%lu",userAuth.sUserId.c_str(),userAuth.errCode);
			//pCurrentLink->ResetLogin();
			if(pCurrentLink) {pCurrentLink->CloseLink();pCurrentLink->ReleaseRef();}
			if(pCacheLink){pCacheLink->ReleaseRef();}
			return false;
		}
		if(!SaveUserInfo(login,sUserId,userAuth))
		{
			LoginRsp(login.suserid(),sessionId,ERR_CM_EXCEPTION);
			WarnLog("User %s authorized successfully but encounter resource exception!",userAuth.sUserId.c_str());
			//pCurrentLink->ResetLogin();
			if(pCurrentLink) {pCurrentLink->CloseLink();pCurrentLink->ReleaseRef();}
			if(pCacheLink){pCacheLink->ReleaseRef();}
			return false;
		}
	}
	
	if(pCurrentLink){ pCurrentLink->ReleaseRef();}
	if(pCacheLink){pCacheLink->ReleaseRef();}
	UpdateUserLink(sessionId,sUserId);
	LoginRsp(login.suserid(),sessionId,NON_ERR);
	InfoLog("user %s login successfully!",userAuth.sUserId.c_str());
	return true;
}

enum USER_LOG_STATUS
{
	USER_LOG_STATUS_LOGOUT = 0,
	USER_LOG_STATUS_LOGIN = 1,
	USER_LOG_STATUS_PENDING = 2
};

bool CUserManage::OnLogout(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing user logout!");
		return false;
	}

	CMLogout logout;
	uchar_t* pContent = pPdu->GetBodyData();
	if(!pContent || !logout.ParseFromArray(pContent,pPdu->GetBodyLength()))
	{
		ErrLog("Logout parameter error!");
		return false;
	}
	string sUserId = PREFIX_CM_STR + logout.suserid();
	InfoLog("User %s left! cmdid = 0x%x,size=%d,InstId=%d", sUserId.c_str(), pPdu->GetCommandId(),
		pPdu->GetBodyLength(), m_nNumberOfInst);

	//CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(logout.suserid().c_str());
	//if(pUserLink)
	//{
	//	pUserLink->CloseLink();
	//}

	//判断状态是不是1，修改状态为2	

	UserCache_t userCache;
	if(m_pCache->GetUserRecord(sUserId.c_str(),userCache)==true)
	{
		if (USER_LOG_STATUS_LOGIN == userCache.bStatus)
		{
			LogoutRsp(pPdu->GetSessionId(), NON_ERR);
			//m_pCache->SetUserStatus(sUserId.c_str(), (int8_t)USER_LOG_STATUS_PENDING);
			InfoLog("The user %s logout successfully", sUserId.c_str());
		}
		else
		{
			LogoutRsp(pPdu->GetSessionId(), NON_ERR);
			WarnLog("The user %s logout failed, current status %d", sUserId.c_str(), userCache.bStatus);
		}
		
		//del cache data record
		string sCacheDeviceId = userCache.sDeviceToken.c_str();
		string sCacheDevice = string(PREFIX_CMDT_STR) + sCacheDeviceId.substr(0, sCacheDeviceId.find(':', 0)).c_str();
		m_pCache->DelUserRec(sUserId.c_str());
		m_pCache->DelDeviceRec(sCacheDevice.c_str());

		CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId.c_str());
		if (pUserLink)
		{
			pUserLink->CloseLink();
			pUserLink->ReleaseRef();
		}
		
		
		return true;
	}
	else
	{
		LogoutRsp(pPdu->GetSessionId(), NON_ERR);

		WarnLog("The user %s logout failed, get status failed", sUserId.c_str());

		//close socket link,client relogin to correct the userdata in cache
		CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId.c_str());
		if (pUserLink)
		{
			pUserLink->CloseLink();
			pUserLink->ReleaseRef();
		}
		
		return false;
	}

	
	//if (m_pCache->DelUserRec(sUserId.c_str()) == false)
	//{
	//	LogoutRsp(sessionId, ERR_LOGOUT);
	//	ErrLog("Fail to logout the user %s for the redis exception", sUserId.c_str());
	//	return false;
	//}
	
	

	return false;

}
bool CUserManage::OnDeviceTokenSync(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing device synchronize!");
		return false;
	}

	CMDeviceTokenSync tokenSync;
	DeviceCache_t  deviceCache;
	string sUserId=PREFIX_CM_STR;
	uint8_t bStatus = 0;
	UidCode_t sessionId = pPdu->GetSessionId();
	uchar_t* pContent = pPdu->GetBodyData();

	if(!pContent || !tokenSync.ParseFromArray(pContent,pPdu->GetBodyLength()))
	{
		ErrLog("DeviceSync parameter error!");
		DeviceTokenSyncRsp("",sessionId,ERR_DEVICESYNC_PARAMETER);
		return false;
	}

	sUserId += tokenSync.suserid();
	if(!m_pCache->GetUserStatus(sUserId.c_str(),bStatus))
	{
		DeviceTokenSyncRsp(tokenSync.suserid(),sessionId,ERR_DEVICESYNC_FORBIDDEN);
		return false;
	}

	if ( 0 == tokenSync.ndevicetokentype())
	{
		if (m_pCache->DeviceSynchronize(sUserId.c_str(), tokenSync.ndevicetype(),
			tokenSync.sdeviceversion().c_str(), tokenSync.sdevicetoken().c_str()) == false)
		{
			DeviceTokenSyncRsp(tokenSync.suserid(), sessionId, ERR_DEVICESYNC_EXCEPTION);
			return false;
		}

		string sDeviceId = tokenSync.sdevicetoken().substr(0,tokenSync.sdevicetoken().find(':',0));
		deviceCache.sDeviceToken = PREFIX_CMDT_STR;
		//deviceCache.sDeviceToken += tokenSync.sdevicetoken().c_str();
		deviceCache.sDeviceToken += sDeviceId.c_str();
		deviceCache.sSubscriptId = tokenSync.suserid().c_str();
		m_pCache->InsertDeviceRec(deviceCache);						// Update the device token cache. 
	}
	else if (1 == tokenSync.ndevicetokentype())
	{
		if (m_pCache->DeviceSynchronize(sUserId.c_str(), tokenSync.sdevicetoken().c_str()) == false)
		{
			DeviceTokenSyncRsp(tokenSync.suserid(), sessionId, ERR_DEVICESYNC_EXCEPTION);
			return false;
		}
	}
	

		
	DeviceTokenSyncRsp(tokenSync.suserid(),sessionId,NON_ERR);		

	return true;
}

bool CUserManage::OnLogoutConfirm(std::shared_ptr<CImPdu> pPdu)
{
	if (!pPdu)
	{
		ErrLog("Recv a null pdu when processing user logout!");
		return false;
	}

	CMLogout logout;
	uchar_t* pContent = pPdu->GetBodyData();
	if (!pContent || !logout.ParseFromArray(pContent, pPdu->GetBodyLength()))
	{
		ErrLog("Logout parameter error!");
		return false;
	}

	string sUserId = PREFIX_CM_STR + logout.suserid();
	InfoLog("User %s left! cmdid = 0x%x,size=%d,InstId=%d", sUserId.c_str(), pPdu->GetCommandId(),
		pPdu->GetBodyLength(), m_nNumberOfInst);

	//判断状态是不是pending

	UserCache_t userCache;
	if (m_pCache->GetUserRecord(sUserId.c_str(), userCache) == true)
	{
		if (USER_LOG_STATUS_PENDING == userCache.bStatus)
		{
			//如果状态是挂起，直接删除链路，不用再回复ACK了
			CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId.c_str());
			if (pUserLink)
			{
				pUserLink->CloseLink();
				pUserLink->ReleaseRef();
			}
			string sCacheDeviceId = userCache.sDeviceToken.c_str();
			string sCacheDevice = string(PREFIX_CMDT_STR) + sCacheDeviceId.substr(0, sCacheDeviceId.find(':', 0)).c_str();
			m_pCache->DelUserRec(sUserId.c_str());
			m_pCache->DelDeviceRec(sCacheDevice.c_str());

			InfoLog("The user %s logout successfully", sUserId.c_str());
			return true;
		}
		else
		{
			WarnLog("The user %s logoutConfirm current status = %d", sUserId.c_str(), userCache.bStatus);
		}
		
	}
	else
	{
		LogoutConfirmRsp(pPdu->GetSessionId(), EXCEPT_ERR);
	}


	//if(m_pCache->DelUserRec(sUserId.c_str())==false)
	//{
	//	LogoutConfirmRsp(sessionId,ERR_LOGOUT);
	//	ErrLog("Fail to logout the user %s for the redis exception",sUserId.c_str());
	//	return false;
	//}



	return false;

}

bool CUserManage::OnClose(std::shared_ptr<CImPdu> pPdu)
{	
	do{
			if(!pPdu)
			{
				ErrLog("Recv a null pdu when processing user overtime event!");
				return false;
			}
			
			UidCode_t sessionId = pPdu->GetSessionId();

			CClientLink* pSessionLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
			if(!pSessionLink)
			{
				ErrLog("Find a invalid link when processing link close event");
				return false;
			}
				
			string sUserId = pSessionLink->GetUserId();
			CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId);
			bool bUserLink = (pSessionLink==pUserLink); 
			DbgLog("Close link, bUserLink=%d,sessionLink=%p,userLink=%p, user %s",bUserLink,pSessionLink,pUserLink, sUserId.c_str());
			//CAutoLock autolock(CClientLinkMgr::GetInstance()->GetCliLock());
			if(pSessionLink)
			{	
				pSessionLink->CloseLink(bUserLink);
				pSessionLink->ReleaseRef();
			}
			
			if(pUserLink){pUserLink->ReleaseRef();}

			string sLocalIP;
			uint16_t nLocalPort;
			CClientLinkMgr::GetInstance()->GetLocalHost(sLocalIP,nLocalPort);

			UserCache_t userRec;
			string sLoginIP="";
			if(m_pCache->GetUserRecord(sUserId.c_str(),userRec))
			{
				sLoginIP = userRec.sIPAddr;
			}

			if(strcmp(sLocalIP.c_str(),sLoginIP.c_str()) && userRec.bStatus == 1)
			{
				DbgLog("user %s login other cm %s ,no need to set status to 0",sUserId.c_str(),sLoginIP.c_str());
				break; //user login other cm,no need to set user status to 0
			}
			
			if(bUserLink) //Don't set user status to 0; 
			{
				if(m_pCache->SetUserStatus(sUserId.c_str(),0)==false)
				{
					ErrLog("Failed to set user %s redis status",sUserId.c_str());
					return false;
				}
			}
		}while(0);
			
	return true; 
}

void CUserManage::LoginRsp(string sUserId,UidCode_t sessionId,ErrCode bCode,uint64_t nLastLoginTime)
{
	CMLoginAck 	loginAck;
	CImPdu 		loginAckPdu;

	loginAck.set_suserid(sUserId);
	loginAck.set_nerr(bCode);
	loginAck.set_nlastlogintime(nLastLoginTime);
	loginAckPdu.SetPBMsg(&loginAck);
	loginAckPdu.SetCommandId(CM_LOGIN_ACK);
	loginAckPdu.SetSessionId(sessionId);
	DbgLog("LoginAck userID[%s] sessionID[%x%x%x%x%x%x%x%x%x%x%x%x] errCode[%d]",sUserId.c_str(),
				sessionId.Uid_Item.code[0],
				sessionId.Uid_Item.code[1],
				sessionId.Uid_Item.code[2],
				sessionId.Uid_Item.code[3],
				sessionId.Uid_Item.code[4],
				sessionId.Uid_Item.code[5],
				sessionId.Uid_Item.code[6],
				sessionId.Uid_Item.code[7],
				sessionId.Uid_Item.code[8],
				sessionId.Uid_Item.code[9],
				sessionId.Uid_Item.code[10],
				sessionId.Uid_Item.code[11],
				bCode);
	SendPdu(&loginAckPdu);
}

void CUserManage::LogoutRsp(UidCode_t sessionId,ErrCode bCode)
{
	CMLogoutAck logoutAck;
	CImPdu 		logoutAckPdu;

	logoutAck.set_nerr(bCode);
	logoutAckPdu.SetPBMsg(&logoutAck);
	logoutAckPdu.SetCommandId(CM_LOGOUT_ACK);
	logoutAckPdu.SetSessionId(sessionId);
	
	SendPdu(&logoutAckPdu);
}



void CUserManage::LogoutConfirmRsp(UidCode_t sessionId, ErrCode bCode)
{
	CMLogoutAck logoutConfirmAck;
	CImPdu 		logoutConfirmAckPdu;

	logoutConfirmAck.set_nerr(bCode);
	logoutConfirmAckPdu.SetPBMsg(&logoutConfirmAck);
	logoutConfirmAckPdu.SetCommandId(CM_LOGOUT_CONFIRM_ACK);
	logoutConfirmAckPdu.SetSessionId(sessionId);

	SendPdu(&logoutConfirmAckPdu);
}

void CUserManage::DeviceTokenSyncRsp(string sUserId,UidCode_t sessionId,ErrCode bCode)
{
	CMDeviceTokenSyncAck 	syncAck;
	CImPdu 		syncAckPdu;

	syncAck.set_suserid(sUserId);
	syncAck.set_nerr(bCode);
	syncAckPdu.SetPBMsg(&syncAck);
	syncAckPdu.SetCommandId(CM_DEVICETOKENSYNC_ACK);
	syncAckPdu.SetSessionId(sessionId);
	
	SendPdu(&syncAckPdu);
}


