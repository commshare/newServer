#ifndef __LOGIN_HANDLE_H__
#define __LOGIN_HANDLE_H__

#include <regex>
#include "configfilereader.h"
#include "util.h"
#include "clientlinkmgr.h"
#include "packet.h"
#include "loginCache.h"
#include "jsoncpp/json/json.h"
#include "tokenvalidator.h"
#include "HttpClient.h"
#include "im.pub.pb.h"
#include "im.cm.pb.h"
#include "im.inner.pb.h"

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

class CLoginHandle : public CPacket
{
public:
	CLoginHandle(CConfigFileReader* pConfigReader,int nNumOfInst);
	~CLoginHandle();

public:
	bool Initialize(void);
	bool OnLogin(std::shared_ptr<CImPdu> pPdu); //User login. 0xa001
	bool OnLogout(std::shared_ptr<CImPdu> pPdu);//User logout, 0xa003	
	bool OnCmLoginNotify(std::shared_ptr<CImPdu> pPdu);	//over time response or close user link , release user information. 
	bool OnPHPLoginNotify(std::shared_ptr<CImPdu> pPdu);	// php登陸通知
	bool OnLoginCMNotifyAck(std::shared_ptr<CImPdu> pPdu);
	bool OnUserPushSetNotify(std::shared_ptr<CImPdu> pPdu);		// 用户推送设置开关通知

public:
	void addLoginCacheInfo(std::string userId);
	void removeLoginCacheInfo(std::string userId);
	
protected:
	virtual bool RegistPacketExecutor(void);
	
private:
	CLoginCache* GetCache() {return m_pCache;}
	void LoginRsp(string& sUserId,UidCode_t sessionId,im::ErrCode bCode, string& linkSession);
	void LogoutRsp(string sUserId, UidCode_t sessionId,im::ErrCode bCode);
	bool Kickout(UidCode_t currentSessionId,UserCache_t userCache);
	bool SendMsgToCM(const google::protobuf::MessageLite* msg, const string& strIp, uint16_t nPort, im::CmdId cmdId);
	
	bool SaveUserInfo(im::CMLoginTrans login,string sUserId, const string& strIp, uint16_t nPort);
	bool UpdateUserInfo(im::CMLoginTrans login,string sUserId,string sCacheDeviceId, const string& strIp, uint16_t nPort);
	
	bool VerifyDeviceToken(string sDeviceToken);
	bool CheckCacheDevice(string sDevice, string sCacheDevice);
	void UrlSign(string sUserId,string sToken,string& sSignUrl);
	bool CheckAuth(UserAuth_t& auth);
	bool CheckAuthInterval(UserCache_t userCache,string sTokens);	
	virtual bool SetCheckAuth(void);	
	string RandStr(void);
    string MD5Sign(string sNonceStr, string sToken, string userid);

	bool sendAck(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const UidCode_t& sessionId);

	bool getUserRadioIds(const std::string& sUserId, std::vector<std::string>& vecChnn);
	bool parseRadioList(const std::string& sUserId, const std::string& strData, std::vector<std::string>& vecChnn);
	
	
private:
	CConfigFileReader* m_pConfigReader;	
	string m_sCheckAuthUrl;
	string m_sAppSecretChecking;  // App secret code for checking. 
	int32_t m_nCheckInterval;
	CLoginCache* m_pCache;		//Redis cache pointer . 
	int m_nNumberOfInst;
	string m_sUserChannelsUrl;

};

#endif // __LOGIN_HANDLE_H__
