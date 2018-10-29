/*****************************************************************************************
Filename: usermanage.h
Author: jack			Version: im-1.0 		Date:2017/06/14
Description: 	用户登录/登出、权限检查、用户信息管理类定义
*****************************************************************************************/
#ifndef __USERMANAGE_H__
#define __USERMANAGE_H__
#include <string.h>
#include <fstream>
#include <curl/curl.h>
#include <regex>

#include "configfilereader.h"
#include "util.h"
#include "clientlinkmgr.h"
#include "packet.h"
#include "cache.h"
#include "jsoncpp/json/json.h"
#include "tokenvalidator.h"
#include "HttpClient.h"
#include "im.pub.pb.h"
#include "im.cm.pb.h"

#define PREFIX_CM_STR "CM_"					//user login cached information.
#define PREFIX_CMDT_STR "CMDT_"				//the logged in user device information. 
#define POSTFIX_CM_POS	3
#define PREFIX_DEVICELEN_REF 5

using namespace std;
using namespace im;
using namespace imsvr;

#define MAX_GROUPLIMIT 50

typedef struct _UserAuth
{
	string sUserId;			//Return by user center if auth checking success.
	string sLoginToken;		//Token  for validation .
	string sAppId;			//
	uint16_t nGroupLimit;	//
	uint16_t nGroupNumber;	//
	uint8_t bRole;			//Return by user center if auth checking success;
	ErrCode errCode;        //Return by user center if auth checking fail;
	uint64_t nLastLoginTime;  //Return by user center if auth checking fail;
} UserAuth_t;

class CUserManage : public CPacket
{
public:
	CUserManage(CConfigFileReader* pConfigReader,int nNumOfInst);
	~CUserManage();

	bool Initialize(void);
	bool OnLogin(std::shared_ptr<CImPdu> pPdu); //User login. 0xa001
	bool OnLogout(std::shared_ptr<CImPdu> pPdu);//User logout, 0xa003	
	bool OnDeviceTokenSync(std::shared_ptr<CImPdu> pPdu);//device synchronize , 0xa005
	bool OnLogoutConfirm(std::shared_ptr<CImPdu> pPdu);//User logout, 0xa007	
	bool OnClose(std::shared_ptr<CImPdu> pPdu);	//over time response or close user link , release user information. 
protected:	
	CCache* GetCache() {return m_pCache;}
	void LoginRsp(string sUserId,UidCode_t sessionId,ErrCode bCode,uint64_t nLastLoginTime = 0);
	void LogoutRsp(UidCode_t sessionId,ErrCode bCode);
	void LogoutConfirmRsp(UidCode_t sessionId, ErrCode bCode);
	void DeviceTokenSyncRsp(string sUserId,UidCode_t sessionId,ErrCode bCode);
	void UrlSign(string sUserId,string sToken,string& sSignUrl);
	void UpdateUserLink(UidCode_t currentSessionId,string sUserId);
	bool Kickout(UidCode_t currentSessionId,UserCache_t userCache);
	bool SaveUserInfo(CMLogin login,string sUserId,UserAuth_t userAuth);
	bool UpdateUserInfo(CMLogin login,string sUserId,string sCacheDeviceId,UserAuth_t userAuth,bool bCheck);
	bool VerifyDeviceToken(string sDeviceToken);
	bool CheckCacheDevice(string sDevice, string sCacheDevice);
	bool CheckAuth(UserAuth_t& auth);
	bool CheckAuthInterval(UserCache_t userCache,string sTokens);	
	virtual bool RegistPacketExecutor(void);
	virtual bool SetCheckAuth(void);	
	string RandStr(void);
    string MD5Sign(string sNonceStr, string sToken, string userid);
private:
	CConfigFileReader* m_pConfigReader;	
	string m_sCheckAuthUrl;
	string m_sAppSecretChecking;  // App secret code for checking. 
	int32_t m_nCheckInterval;
	CCache* m_pCache;		//Redis cache pointer . 
	int m_nNumberOfInst;

};

#endif
