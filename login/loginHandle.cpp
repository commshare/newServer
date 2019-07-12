#include "loginHandle.h"
#include <algorithm>
#include "encdec.h"
#include "thread_pool_manager.h"
#include "redis_manager.h"
//static const regex rUserIdFilters("^[0-9]{5,10}$"); 

CLoginHandle::CLoginHandle(CConfigFileReader* pConfigReader,int nNumOfInst)
	: m_pConfigReader(pConfigReader)
	, m_sCheckAuthUrl("")
	, m_sAppSecretChecking("")
	, m_nCheckInterval(0)
	, m_pCache(nullptr)
	, m_nNumberOfInst(nNumOfInst)
	, m_sUserChannelsUrl("")
	, m_sUserInfoUrl("")
	
{

}

CLoginHandle::~CLoginHandle()
{
	if(m_pCache)
	{
		delete m_pCache;
	}
}

bool CLoginHandle::Initialize(void)  
{
	if(SetCheckAuth()==false)				//Load config parameter from config file.
		return false;
	m_pCache = new CLoginCache(m_pConfigReader);		// Redis cache instance definition
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

bool CLoginHandle::SetCheckAuth()
{
	if(!m_pConfigReader)
		return false;
	char* pAuth = m_pConfigReader->GetConfigName("check_interval"); 
	if(pAuth == nullptr)
		return false;
	m_nCheckInterval = atoi(pAuth);

	pAuth = m_pConfigReader->GetConfigName("check_appsecret"); 
	if(pAuth == nullptr)
		return false;
	m_sAppSecretChecking = pAuth;
	pAuth = m_pConfigReader->GetConfigName("check_url");
	if(pAuth == nullptr)
		return false;
	m_sCheckAuthUrl = pAuth;
	pAuth = m_pConfigReader->GetConfigName("userChannlsUrl");
	if(pAuth == nullptr)
		return false;
	m_sUserChannelsUrl = pAuth;
	pAuth = m_pConfigReader->GetConfigName("usrInfoUrl");
	if(pAuth == nullptr)
		return false;
	m_sUserInfoUrl = pAuth;
	return true;
}

bool CLoginHandle::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(im::CM_LOGIN_TRANS,m_nNumberOfInst, CommandProc(&CLoginHandle::OnLogin));
	CmdRegist(im::CM_LOGOUT, m_nNumberOfInst,CommandProc(&CLoginHandle::OnLogout));
	CmdRegist(im::CM_LOGIN_NOTIFY, m_nNumberOfInst,CommandProc(&CLoginHandle::OnCmLoginNotify));
	CmdRegist(im::CM_PHP_LOGIN_NOTIFY, m_nNumberOfInst,CommandProc(&CLoginHandle::OnPHPLoginNotify));
	CmdRegist(im::LOGIN_CM_NOTIFY_ACK, m_nNumberOfInst,CommandProc(&CLoginHandle::OnLoginCMNotifyAck));
	CmdRegist(im::SVR_RADIO_PUSHSET_NOTIFY, m_nNumberOfInst,CommandProc(&CLoginHandle::OnUserPushSetNotify));
	CmdRegist(im::CM_PUSHTOKENSYNC, m_nNumberOfInst,CommandProc(&CLoginHandle::OnPushTokenSync));
	return true;
}

string  CLoginHandle::RandStr(void)
{
	char buf[64] = {0};
	string str;
	
	srand( (unsigned)time( NULL ) );
	sprintf(buf, "%d",rand());
	str = buf;

	return str;
}

string  CLoginHandle::MD5Sign(string sNonceStr, string sToken, string userid)
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

void CLoginHandle::UrlSign(string sUserId,string sToken,string& sSignUrl)
{
	string sNonceStr = RandStr();
	string sign_str;
	char url_sign_buf[1024] = {0};

    sign_str = MD5Sign(sNonceStr,sToken, sUserId);
    sprintf(url_sign_buf,"app_id=1&format=json&nonce=%s&sign=%s&sign_method=md5&token=%s&userid=%s",
		sNonceStr.c_str(),sign_str.c_str(),sToken.c_str(),sUserId.c_str());
	sSignUrl = url_sign_buf;
}

bool CLoginHandle::VerifyDeviceToken(string sDeviceToken)
{
	DbgLog("device token %s", sDeviceToken.c_str());
	if(sDeviceToken.empty())
		return false;
	
	return true;
}

bool CLoginHandle::CheckCacheDevice(string sDevice, string sCacheDevice)
{
//	int nDeviceIndex = sDevice.find(':',0);
//	int nCacheDeviceIndex = sCacheDevice.find(':',0);

//	string sPrefixDevice = sDevice.substr(0,nDeviceIndex);
//	string sPrefixCacheDevice = sCacheDevice.substr(0,nCacheDeviceIndex);

//	if(sPrefixDevice.compare(sPrefixCacheDevice.c_str()) == 0)
	if(sDevice.compare(sCacheDevice.c_str()) == 0)
		return true;

	return false;
}

bool CLoginHandle::CheckAuth(UserAuth_t& auth) //Checking user 
{
	auth.errCode = im::ERR_CM_PHP_AUTH;

	if(m_sCheckAuthUrl.empty())
	{
		ErrLog("The check url can not be null!");
		return false;
	}
	string str = m_sAppSecretChecking + "app_id2token" + auth.sLoginToken + "user_id" + auth.sUserId + m_sAppSecretChecking;
	std::string strMD5 = MD5Str(str);
	transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
	string strPost = "app_id=2&sign=" + strMD5 + "&token=" + auth.sLoginToken + "&user_id=" + auth.sUserId;

//	string strPost = "user_id=" + auth.sUserId + "&token=" + auth.sLoginToken;
	DbgLog("login auth post data %s", strPost.c_str());
	string sChkResult;
	CHttpClient httpClient;
	if(httpClient.Post(m_sCheckAuthUrl, strPost, sChkResult) != CURLE_OK)
	{
		ErrLog("http request fail! url=%s post=%s userid=%s", m_sCheckAuthUrl.c_str(), strPost.c_str(), auth.sUserId.c_str());
		return false;
	}

	InfoLog("The auth check result is :%s ", sChkResult.c_str());
	Json::Reader reader;
	Json::Value root;
	if(!reader.parse(sChkResult, root, false))
	{
		ErrLog("http response parse fail! url=%s post=%s userid=%s", m_sCheckAuthUrl.c_str(), strPost.c_str(), auth.sUserId.c_str());
		return false;
	}
	string sReturnCode = root["code"].asString();
	if(strcmp(sReturnCode.c_str(),"200") == 0)
	{
		auth.errCode = NON_ERR;
		return true;
	}
	else if(strcmp(sReturnCode.c_str(),"501") == 0) // 在其他地方登陆，被踢
	{
		ErrLog("auth fail! code=%s, msg=%s userid=%s", sReturnCode.c_str(), root["msg"].asString().c_str(), auth.sUserId.c_str());
		auth.errCode = im::ERR_CM_AUTH_KICKEDOUT;
		return false;
	}
	else if(strcmp(sReturnCode.c_str(),"30907") == 0) // token验证失败（不属于该用户）
	{
		ErrLog("auth fail! code=%s, msg=%s userid=%s", sReturnCode.c_str(), root["msg"].asString().c_str(), auth.sUserId.c_str());
		auth.errCode = im::ERR_LOGIN_AUTH;
		return false;
	}
	else if(strcmp(sReturnCode.c_str(),"401") == 0) // token 过期
	{
		ErrLog("auth fail! code=%s, msg=%s userid=%s", sReturnCode.c_str(), root["msg"].asString().c_str(), auth.sUserId.c_str());
		auth.errCode = im::ERR_LOGIN_AUTH;
		return false;
	}
	else	//失败
	{
		ErrLog("auth fail! code=%s, msg=%s", sReturnCode.c_str(), root["msg"].asString().c_str());
		auth.errCode = im::ERR_LOGIN_AUTH;
		return false;
	}
	

    return false;

}
	
bool CLoginHandle::CheckAuthInterval(UserCache_t userCache,string sTokens) //Check whether or not need to auth user this time. 
{
	int currTime = time(0);
	int preTime;
	
	if(sTokens.compare(userCache.sLoginToken.c_str()) != 0)  // the token is diffrent with cache sToken, 
	{												  //  the cache sToken maybe expired time. 	
		WarnLog("Failed to match token");
		return false;
	}
	
	preTime = (userCache.bRelogin) ? atoi(userCache.sReloginTime) : atoi(userCache.sLoginTime);
	
	return ((currTime-preTime)>=m_nCheckInterval) ? false : true;
}

// kickout the same user when logining
bool CLoginHandle::Kickout(UidCode_t currentSessionId, UserCache_t userCache)
{
	string sKickUserId = userCache.sUserId.c_str();
	im::OnLoginResult loginRes;
	loginRes.set_suserid(sKickUserId.substr(POSTFIX_CM_POS));
	loginRes.set_shost(userCache.sSessionId.c_str());
	loginRes.set_nerr(im::NON_ERR);
	loginRes.set_type(im::LOGIN_KICKOUT);

	CImPdu kickoutPDU;
	kickoutPDU.SetPBMsg(&loginRes);
	kickoutPDU.SetCommandId(im::SVR_LOGIN_RESULT);
	kickoutPDU.SetSessionId(currentSessionId);
	
	DbgLog("kickout userid %s at cm server ...",loginRes.suserid().c_str());
	// 发送到上次登录ip和port的CM服务器
	SendPdu(userCache.sIPAddr.c_str(), userCache.nIPPort, &kickoutPDU);
	
	return true;
}

bool CLoginHandle::SendMsgToCM(const google::protobuf::MessageLite* msg, const string& strIp, uint16_t nPort, im::CmdId cmdId)
{
	CImPdu pdu;
	pdu.SetPBMsg(msg);
	CClientLink* pLink = CClientLinkMgr::GetInstance()->GetLinkByHost(strIp.c_str(), nPort);
	if(nullptr != pLink)
	{
		UidCode_t sessionId = pLink->GetSessionId();
		pdu.SetSessionId(sessionId);
		pLink->ReleaseRef();
	}				
	pdu.SetCommandId(cmdId);
	int nLen = SendPdu(strIp.c_str(), nPort, &pdu);
	if (nLen <= 0)
		return false;
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Save user newest information into redis cache when the user login at the first time.  
//////////////////////////////////////////////////////////////////////////////////////////////////

bool CLoginHandle::SaveUserInfo(im::CMLoginTrans login, string sUserId, const string& strIp, uint16_t nPort) //Save the logining user information to redis cache. 
{
	UserCache_t userCache;
	userCache.sUserId = sUserId.c_str();
	userCache.bStatus = 1;
	userCache.sIPAddr = strIp.c_str();
	userCache.nIPPort = nPort;
  	userCache.sLoginToken = login.slogintoken().c_str();
	userCache.sDeviceToken = login.sdevicetoken().c_str();
	userCache.sSessionId = login.shost().c_str();
	userCache.nPushType = login.npushtype();
	userCache.sPushToken = login.spushtoken().c_str();

	if(m_pCache->InsertUserRec(userCache)==false)      //Now insert cache information .
	{
		ErrLog("Failed to insert userinfo to redis!");
		return false;
	}

	DeviceCache_t  deviceCache;
	deviceCache.sDeviceToken = PREFIX_CMDT_STR;
	deviceCache.sDeviceToken += login.sdevicetoken().c_str();

//	if(true == m_pCache->GetDeviceRecord(deviceCache.sDeviceToken,deviceCache))
//	{
//		string sCacheUser = PREFIX_CM_STR;
//		sCacheUser += deviceCache.sSubscriptId.c_str();
//		if(sCacheUser.compare(sUserId.c_str()) != 0)  // Means other user using the device last time. 
//		{
//			DbgLog("Delete cache user %s",sCacheUser.c_str());
//			m_pCache->DelUserRec(sCacheUser.c_str());   //Delete the cached user information since a new user loged in with the same device . 
//		}
//	}	
	m_pCache->DelDeviceRec(deviceCache.sDeviceToken.c_str());
	deviceCache.sSubscriptId = sUserId.substr(POSTFIX_CM_POS).c_str();
	m_pCache->InsertDeviceRec(deviceCache);

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// Update user newest information into redis cache. 
//////////////////////////////////////////////////////////////////////////////////////////////////
bool CLoginHandle::UpdateUserInfo(im::CMLoginTrans login,string sUserId,string sCacheDeviceId, const string& strIp, uint16_t nPort)
{
	UserCache_t userCache;
	userCache.sUserId = sUserId.c_str();
	userCache.bStatus = 1;
	userCache.sIPAddr = strIp.c_str();
	userCache.nIPPort = nPort;
	userCache.sLoginToken = login.slogintoken().c_str();
	userCache.sDeviceToken = login.sdevicetoken().c_str();
	userCache.sSessionId = login.shost().c_str();
	userCache.nPushType = login.npushtype();
	userCache.sPushToken = login.spushtoken().c_str();

	if(m_pCache->UpdateUserRec(userCache)==false)
	{
		ErrLog("Failed to update userinfo to redis!");
		return false;
	}
	
	if(!CheckCacheDevice(sCacheDeviceId, userCache.sDeviceToken.c_str()))
	{
		DeviceCache_t  deviceCache;
		string sCacheDeviceToken = PREFIX_CMDT_STR;
		sCacheDeviceToken += sCacheDeviceId.c_str();
//		if(m_pCache->GetDeviceRecord(sCacheDeviceToken.c_str(),deviceCache))
//		{
//			string sCacheUser = PREFIX_CM_STR;
//			sCacheUser += deviceCache.sSubscriptId.c_str();
//			if(sCacheUser.compare(sUserId.c_str()) != 0)  // Means other user using the device last time. 
//			{
//				DbgLog("Delete cache user %s",sCacheUser.c_str());
//				m_pCache->DelUserRec(sCacheUser.c_str());   //Delete the cached user information since a new user loged in with the same device . 
//			}
//		}		
		m_pCache->DelDeviceRec(sCacheDeviceToken.c_str());		// Delete the cached device token at last time login . 	
	}

	DeviceCache_t  deviceCache;
	deviceCache.sDeviceToken = PREFIX_CMDT_STR;
	deviceCache.sDeviceToken += login.sdevicetoken().c_str();
	deviceCache.sSubscriptId = sUserId.substr(POSTFIX_CM_POS).c_str();
	m_pCache->InsertDeviceRec(deviceCache);						// Update the device token cache. 
	return true;
}

bool CLoginHandle::OnLogin(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing user login!");
		return false;
	}

	bool bRest = false;
	im::ErrCode err = im::ERR_LOGIN_FORBIDDEN;
	std::string strUserId = "";
	UidCode_t sessionId = pPdu->GetSessionId();
	uint64_t nLastLoginTime = 0;
	std::string linkHost = "";
	bool newLogin = false;
	std::string sUserId = "";
	do
	{
		
		// 参数校验
		im::CMLoginTrans loginTrans;
		if(!loginTrans.ParseFromArray(pPdu->GetBodyData(),pPdu->GetBodyLength()))
		{
			ErrLog("client login parse protocol fail");
			break;
		}
		if(loginTrans.suserid().empty()/* || !regex_match(loginTrans.suserid(),rUserIdFilters)*/)
		{
			ErrLog("client login userid is null or regex_match userid fail");
			break;
		}
		strUserId = loginTrans.suserid();
		linkHost = loginTrans.shost();
		DbgLog("login userId=%s loginToken=%s deviceToken=%s pushType=%d pushToken=%s", 
			loginTrans.suserid().c_str(), loginTrans.slogintoken().c_str(), loginTrans.sdevicetoken().c_str(), loginTrans.npushtype(), loginTrans.spushtoken().c_str());
		if(!VerifyDeviceToken(loginTrans.sdevicetoken()))
		{
			ErrLog("client login device token fromat error");
			break;
		}
		// 登陆验证
		bool bAuthCheck = false;
		UserAuth_t  userAuth;
		userAuth.sUserId = loginTrans.suserid();			//Pure userid ,
		userAuth.sLoginToken = loginTrans.slogintoken();
		userAuth.bRole = 0;
		
		UserCache_t userCache;
		err = im::NON_ERR;
		sUserId = PREFIX_CM_STR + strUserId; //UserId with 'CM' plugin to use in redis cache. 
		if(m_pCache->GetUserRecord(sUserId.c_str(),userCache))
		{
			if(loginTrans.sdevicetoken().compare(userCache.sDeviceToken.c_str()) != 0 || !(bAuthCheck = CheckAuthInterval(userCache,userAuth.sLoginToken)))
			{
				//token is different need to check Auth
				if(!CheckAuth(userAuth))
				{
					ErrLog("Failed to re-check auth : %s,%s! errcode:%lu",userAuth.sUserId.c_str(),userAuth.sLoginToken.c_str(),userAuth.errCode);
					err = userAuth.errCode;
					nLastLoginTime = userAuth.nLastLoginTime;
					break;
				}
			}
			DbgLog("Current user %s status %d",userCache.sUserId.c_str(),userCache.bStatus);

			if(userCache.bStatus && !CheckCacheDevice(loginTrans.sdevicetoken(),userCache.sDeviceToken.c_str()))
			{
				Kickout(pPdu->GetSessionId(),userCache);
			}

			if(!UpdateUserInfo(loginTrans,sUserId,userCache.sDeviceToken.c_str(), loginTrans.sloginip(), loginTrans.nloginport()))
			{
				WarnLog("User %s authorized successfully when relogin ,but encounter resource exception!",userAuth.sUserId.c_str());
				err = im::ERR_CM_EXCEPTION;
				break;
			}
			bRest = true;
			DbgLog("user %s login successfully! host=%s", strUserId.c_str(), linkHost.c_str());
		}
		else		//New user login  or User record is abnormal.
		{
			InfoLog("user %s is a new login member", userAuth.sUserId.c_str());
			if(!CheckAuth(userAuth))
			{
				WarnLog("Failed to new-authorize : %s errcode:%lu",userAuth.sUserId.c_str(),userAuth.errCode);
				err = userAuth.errCode;
				nLastLoginTime = userAuth.nLastLoginTime;
				break;
			}
			
			if(!SaveUserInfo(loginTrans, sUserId, loginTrans.sloginip(), loginTrans.nloginport()))
			{
				WarnLog("User %s authorized successfully but encounter resource exception!",userAuth.sUserId.c_str());
				err = im::ERR_CM_EXCEPTION;
				break;
			}
			bRest = true;
			newLogin = true;
			DbgLog("new user %s login successfully! host=%s", strUserId.c_str(), linkHost.c_str()); 
		}
	
	}while(0);
	
	LoginRsp(strUserId, sessionId, err, linkHost);

	// 新登录预热频道相关数据
	if(newLogin)
		CThreadPoolManager::getInstance()->getLoginCacheMgrPool()->add_task(&CLoginHandle::loginChannelPush, this, strUserId);

	if(im::NON_ERR == err)
		CThreadPoolManager::getInstance()->getLoginCacheMgrPool()->add_task(&CLoginHandle::addLoginCacheInfo, this, strUserId);

	return bRest;
}

enum USER_LOG_STATUS
{
	USER_LOG_STATUS_LOGOUT = 0,
	USER_LOG_STATUS_LOGIN = 1,
	USER_LOG_STATUS_PENDING = 2
};

bool CLoginHandle::OnLogout(std::shared_ptr<CImPdu> pPdu)
{
	bool bRest = false;
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing user logout!");
		return bRest;
	}

	std::string strUserId = "";
	UidCode_t sessionId = pPdu->GetSessionId();
	do
	{
		CMLogout logout;
		uchar_t* pContent = pPdu->GetBodyData();
		if(nullptr == pContent || !logout.ParseFromArray(pContent,pPdu->GetBodyLength()))
		{
			ErrLog("Logout parameter error!");
			break;
		}
		strUserId = logout.suserid();
		string sUserId = PREFIX_CM_STR + logout.suserid();
		InfoLog("User %s left! cmdid = 0x%x,size=%d,InstId=%d", sUserId.c_str(), pPdu->GetCommandId(), pPdu->GetBodyLength(), m_nNumberOfInst);
		
		UserCache_t userCache;
		if(m_pCache->GetUserRecord(sUserId.c_str(),userCache))
		{
			if (USER_LOG_STATUS_LOGIN == userCache.bStatus)
				InfoLog("The user %s logout successfully", sUserId.c_str());
			else
				WarnLog("The user %s logout failed, current status %d", sUserId.c_str(), userCache.bStatus);
			
			//del cache data record
			string sCacheDeviceId = userCache.sDeviceToken.c_str();
			string sCacheDevice = string(PREFIX_CMDT_STR) + sCacheDeviceId.substr(0, sCacheDeviceId.find(':', 0)).c_str();
			m_pCache->DelUserRec(sUserId.c_str());
			m_pCache->DelDeviceRec(sCacheDevice.c_str());
			bRest = true;
		}
		else
		{
			WarnLog("The user %s logout failed, get status failed", sUserId.c_str());
		}
	}while(0);

	LogoutRsp(strUserId, sessionId, im::NON_ERR);
	return bRest;

}

bool CLoginHandle::OnCmLoginNotify(std::shared_ptr<CImPdu> pPdu)
{	
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing user overtime event!");
		return false;
	}
	bool bRest = false;
	std::string  strUserId = "";
	UidCode_t sessionId = pPdu->GetSessionId();
	do{
		im::CMLoginNotify cmloginNotify;
		uchar_t* pContent = pPdu->GetBodyData();
		if (!pContent || !cmloginNotify.ParseFromArray(pContent, pPdu->GetBodyLength()))
		{
			ErrLog("CmLoginNotify parameter error!");
			break;
		}
		strUserId = cmloginNotify.suserid();
		string sUserId = PREFIX_CM_STR + strUserId;
		UserCache_t userRec;
		if(!m_pCache->GetUserRecord(sUserId.c_str(),userRec))
		{
			ErrLog("get user cache error!");
			break;
		}
		DbgLog("cm_server send login_server notify!userid=%s, type=%d", strUserId.c_str(), cmloginNotify.notifytype());
//		string sLocalIP = "";
//		CClientLink* pCurrentLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
//		if(nullptr != pCurrentLink)
//		{
//			sLocalIP = pCurrentLink->GetPeerIp();
//			pCurrentLink->ReleaseRef();
//		}
		if(cmloginNotify.notifytype() == im::CMLOGIN_CLOSELINK)
		{
			// 登陸在同一個CM服務器上，且狀態為1，將其狀態更改爲0
 			if(strcmp(cmloginNotify.sip().c_str(),userRec.sIPAddr.c_str()) == 0 && cmloginNotify.nport() == userRec.nIPPort && userRec.bStatus == 1 && strcmp(cmloginNotify.shost().c_str(), userRec.sSessionId.c_str()) == 0)
			{
				DbgLog("user %s login same cm server ip %s port %d , need to set status to 0. linkhost=%s",sUserId.c_str(),userRec.sIPAddr.c_str(), userRec.nIPPort, userRec.sSessionId.c_str()); 
				if(m_pCache->SetUserStatus(sUserId.c_str(),0)==false)
				{
					ErrLog("Failed to set user %s redis status",sUserId.c_str());
					break;
				}
				CThreadPoolManager::getInstance()->getLoginCacheMgrPool()->add_task(&CLoginHandle::removeLoginCacheInfo, this, strUserId);
			}
			else
				break;
		}
		bRest = true;
	}while(0);

	im::CMLoginNotifyAck notifyAck;
	notifyAck.set_suserid(strUserId);
	notifyAck.set_nerr(im::NON_ERR);
	sendAck(&notifyAck, im::CM_LOGIN_NOTIFY_ACK, sessionId);
	
	return bRest; 
}

bool CLoginHandle::OnPHPLoginNotify(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing user overtime event!");
		return false;
	}
	bool bRest = false;
	std::string  strUserId = "";
	do
	{
		im::CMPHPLoginNotify phploginNotify;
		uchar_t* pContent = pPdu->GetBodyData();
		if (!pContent || !phploginNotify.ParseFromArray(pContent, pPdu->GetBodyLength()))
		{
			ErrLog("Logout parameter error!");
			break;
		}
		DbgLog("login operation user=%s",phploginNotify.suserid().c_str());
		strUserId = phploginNotify.suserid();
		string sUserId = PREFIX_CM_STR + strUserId;

		UserCache_t userRec;
		if(!m_pCache->GetUserRecord(sUserId.c_str(),userRec))
		{
			ErrLog("get user cache error! userid %s", sUserId.c_str());
			break;
		}

//		if(userRec.bStatus == 1)
//		{
			DbgLog("php notify login operation user=%s, device=%s, type=%d, status=%d",phploginNotify.suserid().c_str(), phploginNotify.sdevicetoken().c_str(), phploginNotify.logintype(), userRec.bStatus);
//			if(strcmp(userRec.sDeviceToken.c_str(), phploginNotify.sdevicetoken().c_str()) == 0 && phploginNotify.logintype() == im::PHP_NOTIFY_TYPE_LOGIN)
//			{
//				ErrLog("same device login",sUserId.c_str());
//				break;
//			}
			if(phploginNotify.logintype() == im::PHP_NOTIFY_TYPE_LOGIN)
			{
				DbgLog("php notify login operation user=%s, device=%s, subType=%d,status=%d",phploginNotify.suserid().c_str(), phploginNotify.sdevicetoken().c_str(), phploginNotify.loginsubtype(), userRec.bStatus);
				if(strcmp(userRec.sDeviceToken.c_str(), phploginNotify.sdevicetoken().c_str()) != 0)
				{
					if(1 == userRec.bStatus)		// 在线发踢出登陆消息
						Kickout(pPdu->GetSessionId(),userRec);
																		
					m_pCache->DelUserRec(sUserId.c_str());
					string sCacheDevice = string(PREFIX_CMDT_STR) + phploginNotify.sdevicetoken();
					m_pCache->DelDeviceRec(sCacheDevice.c_str()); 
				}
			}
			else if(phploginNotify.logintype() == im::PHP_NOTIFY_TYPE_LOGOUT)
			{
				if(!m_pCache->SetUserStatus(sUserId.c_str(),0))
				{
					ErrLog("Failed to set user %s redis status",sUserId.c_str());
					break;
				}
				
				im::LoginCMNotify cmNotify;
				cmNotify.set_suserid(strUserId);
				cmNotify.set_notifytype(im::LoginCMNotifyType::LOGINCM_CLOSELINK);
				if(!SendMsgToCM(&cmNotify, userRec.sIPAddr.c_str(), userRec.nIPPort, im::LOGIN_CM_NOTIFY))
				{
					ErrLog("close linke send cm server command 0x%x failed, user %s", im::LOGIN_CM_NOTIFY, strUserId.c_str());
				}
				m_pCache->DelUserRec(sUserId.c_str());
				string sCacheDevice = string(PREFIX_CMDT_STR) + phploginNotify.sdevicetoken();
				m_pCache->DelDeviceRec(sCacheDevice.c_str());
				
				CThreadPoolManager::getInstance()->getLoginCacheMgrPool()->add_task(&CLoginHandle::removeLoginCacheInfo, this, strUserId);
			}
//		}
		
		bRest = true;
	}while(0);

	im::CMPHPLoginNotifyACK notifyAck;
	notifyAck.set_suserid(strUserId);
	notifyAck.set_msgtime(getCurrentTime());
	notifyAck.set_nerr(im::NON_ERR);
	UidCode_t sessionId = pPdu->GetSessionId();
	sendAck(&notifyAck, im::CM_PHP_LOGIN_NOTIFY_ACK, sessionId);
	return bRest;
}

bool CLoginHandle::OnLoginCMNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	return true;
}

bool CLoginHandle::OnUserPushSetNotify(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing user overtime event!");
		return false;
	}
	im::SVRRadioPushSetNotify pushNotify;
	uchar_t* pContent = pPdu->GetBodyData();
	if (!pContent || !pushNotify.ParseFromArray(pContent, pPdu->GetBodyLength()))
	{
		ErrLog("notify parameter error!");
		return false;
	}
	DbgLog("user push set msg=%s user=%s push=%d", pushNotify.smsgid().c_str(), pushNotify.suserid().c_str(), pushNotify.notifytype());
	string sUserId = PREFIX_CM_STR + pushNotify.suserid();
	if(im::SVRPUSH_NEWMSG == pushNotify.notifytype())
	{
		m_pCache->SetIsNewMsgStatus(sUserId.c_str(), (int)pushNotify.status());
	}
	else
	{
		m_pCache->SetHideMsgOnStatus(sUserId.c_str(), (int)pushNotify.status());
	}
	
	im::SVRMSGNotifyACK notifyAck;
	notifyAck.set_smsgid(pushNotify.smsgid());
	notifyAck.set_msgtime(getCurrentTime());
	notifyAck.set_errcode(im::NON_ERR);
	UidCode_t sessionId = pPdu->GetSessionId();
	sendAck(&notifyAck, im::SVR_RADIO_PUSHSET_NOTIFY_ACK, sessionId);
	return true;
}

bool CLoginHandle::OnPushTokenSync(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing user overtime event!");
		return false;
	}

	im::CMPushTokenSync syncMsg;
	uchar_t* pContent = pPdu->GetBodyData();
	if (!pContent || !syncMsg.ParseFromArray(pContent, pPdu->GetBodyLength()))
	{
		ErrLog("notify parameter error!");
		return false;
	}
	DbgLog("user push set user=%s pushType=%d pushToken=%s", syncMsg.suserid().c_str(), syncMsg.npushtype(), syncMsg.spushtoken().c_str());
	string sUserId = PREFIX_CM_STR + syncMsg.suserid();
	if(syncMsg.npushtype() != 0 && !syncMsg.spushtoken().empty())
		m_pCache->SetPushTypeAndToken(sUserId.c_str(), std::to_string(syncMsg.npushtype()).c_str(), syncMsg.spushtoken().c_str());
	im::CCMPushTokenSyncAck ackMsg;
	ackMsg.set_suserid(syncMsg.suserid());
	ackMsg.set_nerr(im::NON_ERR);
	UidCode_t sessionId = pPdu->GetSessionId();
	sendAck(&ackMsg, im::CM_PUSHTOKENSYNC_ACK, sessionId);
	return true;
}



void CLoginHandle::LoginRsp(string& sUserId,UidCode_t sessionId, im::ErrCode bCode, string& linkSession)
{
	im::OnLoginResult loginRes;
	loginRes.set_suserid(sUserId);
	loginRes.set_shost(linkSession);
	loginRes.set_nerr(bCode);
	loginRes.set_type(im::LOGIN_ACK);

	sendAck(&loginRes, im::SVR_LOGIN_RESULT, sessionId);
}

void CLoginHandle::LogoutRsp(string sUserId,UidCode_t sessionId, im::ErrCode bCode)
{
	CMLogoutAck logoutAck;

	logoutAck.set_nerr(bCode);
	logoutAck.set_suserid(sUserId);
	sendAck(&logoutAck, im::CM_LOGOUT_ACK, sessionId);
}

bool CLoginHandle::sendAck(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const UidCode_t& sessionId)
{
	bool bRest = true;
	CImPdu pdu;
	pdu.SetPBMsg(pMsg);
	pdu.SetSessionId(sessionId);
	pdu.SetCommandId(command_id);
	int nLen = SendPdu(&pdu);
	if (nLen <= 0)
	{
		WarnLog("!!!send command 0x%x failed, return %d, session=%x%x%x%x%x%x%x%x%x%x%x%x", command_id, nLen,
			sessionId.Uid_Item.code[0], sessionId.Uid_Item.code[1],
			sessionId.Uid_Item.code[2], sessionId.Uid_Item.code[3], sessionId.Uid_Item.code[4],
			sessionId.Uid_Item.code[5], sessionId.Uid_Item.code[6], sessionId.Uid_Item.code[7],
			sessionId.Uid_Item.code[8], sessionId.Uid_Item.code[9], sessionId.Uid_Item.code[10],
			sessionId.Uid_Item.code[11]);
		bRest = false;
	}
	return bRest;
}

void CLoginHandle::addLoginCacheInfo(std::string userId)
{
	std::map<std::string, int> mapChnn;
	CRedisManager::getInstance()->getUserChannel(userId, mapChnn);
	if(mapChnn.empty())
	{// http 获取所在频道
		getUserRadioIds(userId, mapChnn);
		if(!mapChnn.empty())
			CRedisManager::getInstance()->addChannelToUser(userId, mapChnn);
	}
	if(!mapChnn.empty())
		CRedisManager::getInstance()->moveOnlineUserToChannel(mapChnn, userId);
	mapChnn.clear();
}

void CLoginHandle::removeLoginCacheInfo(std::string userId)
{
	std::map<std::string, int> mapChnn;
	CRedisManager::getInstance()->getUserChannel(userId, mapChnn);
	if(mapChnn.empty())
	{
		getUserRadioIds(userId,mapChnn);
		if(!mapChnn.empty())
			CRedisManager::getInstance()->addChannelToUser(userId, mapChnn);
		mapChnn.clear();
	}
	if(!mapChnn.empty())
	{
		string sUserId = PREFIX_CM_STR + userId;
		bool bNewMsg = true;
		bool bHideSound = true;
		m_pCache->GetMsgPushStatus(sUserId.c_str(), bNewMsg, bHideSound);
		CRedisManager::getInstance()->moveOfflineUserToChannel(mapChnn, userId, bNewMsg, bHideSound);
	}
	mapChnn.clear();
}

void CLoginHandle::loginChannelPush(std::string userId)
{
	std::map<std::string, int> mapChnn;
	getUserRadioIds(userId, mapChnn);
	if(!mapChnn.empty())
		CRedisManager::getInstance()->addChannelToUser(userId, mapChnn);
	// 获取用户的新消息设置和隐藏模式新消息设置
	std::string sUserId = PREFIX_CM_STR + userId;
	bool newMsg = true;
	bool hideSoundOn = true;
	if(!getUserSetOn(userId, newMsg, hideSoundOn))
		ErrLog("get user info set on fail");
	m_pCache->SetIsNewMsgStatus(sUserId.c_str(), (int)newMsg);
	m_pCache->SetHideMsgOnStatus(sUserId.c_str(), (int)hideSoundOn);
}


bool CLoginHandle::getUserRadioIds(const std::string& sUserId, std::map<std::string, int>& mapChnn)
{
	std::vector<RadioSetOn_t> vecRadioSetOn;
	if(!httpUserRadioIds(sUserId, vecRadioSetOn))
		return false;
	for(auto& itor : vecRadioSetOn)
	{
		int nSetOn = (itor.isHidden ? 0x01 : 0x00) | (itor.isUndisturb ? 0x10 : 0x00);
		mapChnn.emplace(itor.sRadioId, nSetOn);
	}
	return true;
}

bool CLoginHandle::httpUserRadioIds(const std::string& sUserId, std::vector<RadioSetOn_t>& vecRadioSetOn)
{
	string str = m_sAppSecretChecking + "app_id2user_id" + sUserId + m_sAppSecretChecking;
	std::string strMD5 = MD5Str(str);
	transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
	string strPost = "app_id=2&sign=" + strMD5 + "&user_id=" + sUserId;

	std::string strReponse = "";
	CHttpClient httpClient;
	CURLcode code = CURLE_OK;
	code = httpClient.Post(m_sUserChannelsUrl, strPost, strReponse);
	if(CURLE_OK != code || strReponse.empty())
	{
		ErrLog("http request fail! user_id=%s, code=%d", sUserId.c_str(), code);
		return false;
	}
	
	DbgLog("url:%s post:%s response:%s", m_sUserChannelsUrl.c_str(), strPost.c_str(), strReponse.c_str());
	if(!parseRadioList(sUserId, strReponse, vecRadioSetOn) && vecRadioSetOn.empty())
		return false;
	return true;
}


bool CLoginHandle::parseRadioList(const std::string& sUserId, const std::string& strData, std::vector<RadioSetOn_t>& vecRadioSetOn)
{
	Json::Value valRepns;
	Json::Reader readRep;
	if(!readRep.parse(strData, valRepns))
		return false;
	string strCode = valRepns["code"].asString();
	if(strCode != "200")
	{
		ErrLog("htpp response fail! code=%s erro=%s radio_id=%s response=%s", strCode.c_str(), valRepns["msg"].asString().c_str(), sUserId.c_str(), strData.c_str());
		return false;
	}
	RadioSetOn_t radioSetOn;
	int  nSize = valRepns["data"].size();
	for(int i = 0; i < nSize; ++i)
	{
		radioSetOn.sRadioId = valRepns["data"][i]["radio_id"].asString();
		radioSetOn.isHidden = atoi(valRepns["data"][i]["is_hidden"].asString().c_str());
		radioSetOn.isUndisturb = atoi(valRepns["data"][i]["is_undisturb"].asString().c_str());
		vecRadioSetOn.emplace_back(radioSetOn);
	}
	return true;
}

bool CLoginHandle::getUserSetOn(const std::string& sUserId, bool& newMsg, bool& hideSound)
{
	string str = m_sAppSecretChecking + "app_id2user_id" + sUserId + m_sAppSecretChecking;
	std::string strMD5 = MD5Str(str);
	std::transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
	string strPost = "app_id=2&sign=" + strMD5 +  "&user_id=" + sUserId;
	
	std::string strReponse = "";
	CHttpClient httpClient;
	CURLcode code = CURLE_OK;
	code = httpClient.Post(m_sUserInfoUrl, strPost, strReponse);
	DbgLog("url:%s post:%s response:%s", m_sUserChannelsUrl.c_str(), strPost.c_str(), strReponse.c_str());
	if(CURLE_OK != code || strReponse.empty())
	{
		ErrLog("http request fail! user_id=%s, code=%d", sUserId.c_str(), code);
		return false;
	}
	Json::Value valRepns;
	Json::Reader readRep;
	if(!readRep.parse(strReponse, valRepns))
		return false;
	std::string strCode = valRepns["code"].asString();
	if(strCode != "200")
	{
		ErrLog("htpp response fail! code=%s erro=%s user_id=%s response=%s", strCode.c_str(), valRepns["msg"].asString().c_str(), sUserId.c_str(), strReponse.c_str());
		return false;
	}
	newMsg = (valRepns["data"]["is_notice_newmsg"].asString() == "0") ? false : true;
	hideSound = (valRepns["data"]["is_hidden_msgtocomm"].asString() == "0") ? false : true;
	return true;
}





