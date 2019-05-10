#include <iostream>
#include "desktopmsgtran.h"
#include <json/json.h>
#include "im.mes.pb.h"
#include "im.desktop.pb.h"
#include "im.inner.pb.h"
#include <json/json.h>
#include "clientlinkmgr.h"
#include "encdec.h"
#include "HttpClient.h"
#include "getDataInterfaceManager.h"
//#include <uuid.h>
using namespace im;
using namespace std;

#define PREFIX_PC_STR "PC_"
#define PREFIX_CM_STR "CM_"

DesktopMsgTran::DesktopMsgTran(CConfigFileReader* pConfigReader, int nNumOfInst) 
    :CBaseHandle(pConfigReader), m_nNumberOfInst(nNumOfInst)
{
//    m_pPackeQueue = new CPackStreamQueue();
}

DesktopMsgTran::~DesktopMsgTran()
{

//    delete m_pPackeQueue;
//    m_pPackeQueue = NULL;
}

bool DesktopMsgTran::Initialize(void)
{
	char *aboutAuth = m_pConfigReader->GetConfigName("check_appsecret"); 
	if(aboutAuth == nullptr)
    {   
        ErrLog("check_appsecret lost in configfile!");
        return false;
    }
	m_sAppSecretChecking = aboutAuth;

	aboutAuth = m_pConfigReader->GetConfigName("check_url");
	if(aboutAuth == nullptr)
    {
        ErrLog("check_url lost in configfile!");
        return false;
    }
	m_sCheckAuthUrl = aboutAuth;
    
	aboutAuth = m_pConfigReader->GetConfigName("userinfo_url");
	if(aboutAuth == nullptr)
    {
        ErrLog("check_url lost in configfile!");
        return false;
    }
	m_sUserInfoUrl = aboutAuth;

	m_pCache = new CLoginCache(m_pConfigReader);		// Redis cache instance definition
	if(!m_pCache)
    {
        ErrLog("allocate memory error!");
        return false;
    }	
	if(false == m_pCache->Initialize())
	{
		ErrLog("Err of redis connection!");
		return false;
	}

    return CBaseHandle::Initialize();
}

void DesktopMsgTran::UpdateUserLink(UidCode_t currentSessionId,string sUserId)
{
    CClientLink* pLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(currentSessionId);
    if(!pLink)
    {
        ErrLog("get link by session id fail session=%x%x%x%x%x%x%x%x%x%x%x%x user=%s", currentSessionId.Uid_Item.code[0], currentSessionId.Uid_Item.code[1],
               currentSessionId.Uid_Item.code[2], currentSessionId.Uid_Item.code[3], currentSessionId.Uid_Item.code[4],
               currentSessionId.Uid_Item.code[5], currentSessionId.Uid_Item.code[6], currentSessionId.Uid_Item.code[7],
               currentSessionId.Uid_Item.code[8], currentSessionId.Uid_Item.code[9], currentSessionId.Uid_Item.code[10], 
               currentSessionId.Uid_Item.code[11], sUserId.c_str());
        return;
    }

    // 关闭之前登陆的链接 
    CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId);
    if(pUserLink && pUserLink != pLink)
    {
        DbgLog("close old link! user=%s ip=%s port=%d", sUserId.c_str(), pUserLink->GetPeerIp().c_str(), pUserLink->GetPeerPort());
        pUserLink->CloseLink(true);
        pUserLink->ReleaseRef();
    }
    DbgLog("new link info! user=%s ip=%s port=%d", sUserId.c_str(), pLink->GetPeerIp().c_str(), pLink->GetPeerPort());
    pLink->UserArrived(sUserId);
    pLink->ReleaseRef();
}     
#if 0
bool DesktopMsgTran::OnTestDesktopMsgTran(std::shared_ptr<CImPdu> pPdu)
{
    Json::Reader reader;
    Json::Value value;

    if (!pPdu->GetBodyData())
    {
        return false;
    }

    string strInput((char*)pPdu->GetBodyData(), (size_t)pPdu->GetBodyLength());
    if (!reader.parse(strInput, value)) {
        log("json parse failed: %s", strInput.c_str());
   
        return false;
    }
    
    string sToken = value["token"].asString();
    string id = value["id"].asString();
    string extend = value["extend"].asString();
    log("msg content  sToken = %s, id = %s, extend =%s", 
        sToken.c_str(), id.c_str(), extend.c_str());

    return true;
}
#endif 
void DesktopMsgTran::OnKitout(std::shared_ptr<CImPdu> pPdu)
{
    InfoLog("recv msg for kitout");
    if(!pPdu)                                                                                      
    {
        ErrLog("pPdu is empty!");
        return;

    }
    int cmdId = pPdu->GetCommandId();
    InfoLog("recv cmdId = %d", cmdId);
    switch (cmdId)
    {
        case SVR_LOGIN_RESULT://app kitout
        {
            im::LoginCMNotify msg;
            if(!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
            {
                ErrLog("pb phrase is error!"); 
                return;

            }
           // uuid_t out;
           // char buf[64];
           // uuid_generate_random(out);
           // uuid_unparse_lower(out, buf);
           // string msgId = buf;
            string msgId = getuuid();
            string userId = msg.suserid();//for get user link
            string pcUserId = string(PREFIX_PC_STR) + userId;//for get pcLink;
            InfoLog("ready to kickout %s", pcUserId.c_str());
            CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(userId); 
            if(pUserLink!=NULL) 
            {
                UserCache_t userCache;
                if(m_pCache->GetUserRecord(pcUserId.c_str(), userCache))
                {
                    string registerId = userCache.sDeviceToken.c_str();//registerId store as token;
                    char *kickoutRsp = NULL;
                    asprintf(&kickoutRsp, "{\"msgid\":\"%s\", \"registerid\":\"%s\", \"userid\":\"%s\"}",
                             msgId.c_str(), registerId.c_str(), userId.c_str());
                    sendAck(kickoutRsp, PC_KITOUT, pUserLink->GetSessionId());
                    InfoLog("pc_kitout  with sessionid = %s", pUserLink->GetSessionId().toString().c_str());
                    free(kickoutRsp);
                    kickoutRsp= NULL;
                    //del cache 
                    m_pCache->DelUserRec(pcUserId.c_str());
                    {
                        InfoLog("successfully kitout , and del user cache well");
                    }
                    //close link
                    pUserLink->CloseLink();
                }   
            }
            else 
            {
                InfoLog("the linker has lost");
            }
        }
        break;
        case LOGIN_CM_NOTIFY: //app logout
        {
            im::OnLoginResult msg;
            if(!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
            {
                ErrLog("pb phrase is error!"); 
                return;
            }
           // uuid_t out;
           // char buf[64];
           // uuid_generate_random(out);
           // uuid_unparse_lower(out, buf);
           // string msgId = buf;
            string msgId = getuuid();
            string userId = msg.suserid();

            string pcUserId = string(PREFIX_PC_STR) + userId;//for get pcLink;
            InfoLog("ready to kickout %s", pcUserId.c_str());
            CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(userId); 
            if(pUserLink!=NULL) 
            {
                UserCache_t userCache;
                if(m_pCache->GetUserRecord(pcUserId.c_str(), userCache))
                {
                    string registerId = userCache.sDeviceToken.c_str();//registerId store as token;
                    char *kickoutRsp = NULL;
                    asprintf(&kickoutRsp, "{\"msgid\":\"%s\", \"registerid\":\"%s\", \"userid\":\"%s\"}",
                             msgId.c_str(), registerId.c_str(), userId.c_str());
                    sendAck(kickoutRsp, PC_KITOUT, pUserLink->GetSessionId());
                    InfoLog("pc kitout  with sessionid = %s", pUserLink->GetSessionId().toString().c_str());
                    free(kickoutRsp);
                    kickoutRsp= NULL;
                    //del cache
                    if(m_pCache->DelUserRec(pcUserId.c_str()))
                    {
                        InfoLog("successfully kitout , and del user cache well");
                    }
                    //close link
                    pUserLink->CloseLink();
                }   
            }
            else 
            {
                InfoLog("the linker has lost");
            }
            //
        }
        break;
        default:
        {
            return;
        }   
    }
}

void DesktopMsgTran::LoginAck()
{

}

bool DesktopMsgTran::RegistPacketExecutor(void)
{
//	CmdRegist(PC_LOGIN,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnLogin));
//  for test
//    CmdRegist(TESTDESKTOP,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnTestDesktopMsgTran));


	CmdRegist(PC_LOGIN,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromPC));
	CmdRegist(PC_LOGOUT,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromPC));
	CmdRegist(PC_PCSYNMESSAGETOGW,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromPC));
	CmdRegist(PC_GWSYNMESSAGETOPCACK,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromPC));
	CmdRegist(PC_HEARTBEART,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromPC));

	CmdRegist(PC_APPSYNMESSAGETOGW,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromCM));
	CmdRegist(PC_CHECKAPPACTIVEACK,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromCM));
	CmdRegist(PC_GWSYNMESSAGETOAPPACK,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromCM));
	CmdRegist(PC_GWSYNMESSAGETOAPPACK,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromCM));
	CmdRegist(LOGIN_CM_NOTIFY,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromCM));
	CmdRegist(SVR_LOGIN_RESULT,	m_nNumberOfInst,  CommandProc(&DesktopMsgTran::OnMessageFromCM));
    return true;
}


void DesktopMsgTran::OnMessageFromPC(std::shared_ptr<CImPdu> pPdu)
{
    if (!pPdu->GetBodyData())
    {
        InfoLog("pdu is empty!");
        return;
    }
    uint16_t nCmdId =  pPdu->GetCommandId();
    switch(nCmdId)
    {
        case PC_LOGIN:
        {
            OnLogin(pPdu);
        }
        break;
        
        case PC_LOGOUT:
        {
            OnLogout(pPdu);
        }
        break;
        case PC_PCSYNMESSAGETOGW:
        {
            OnPCSynMessageToGw(pPdu);//message from pc trans to app, needed put into queue
        }
        break;
        case PC_GWSYNMESSAGETOPCACK:
        {
            OnGwSynMessageToPCAck(pPdu);//message from pc trans to app, needed put into queue
        }
        break;
        case PC_HEARTBEART:
        {
            OnHeartBeat(pPdu);
        }
        break;

        default:
        {

        }
    }

    return;
}

void DesktopMsgTran::OnMessageFromCM(std::shared_ptr<CImPdu> pPdu)
{

    if (!pPdu->GetBodyData())
    {
        InfoLog("pdu is empty!");
        return;
    }
    uint16_t nCmdId =  pPdu->GetCommandId();
    switch(nCmdId)
    {
        case PC_APPSYNMESSAGETOGW:
        {
            OnAppSynMessageToGw(pPdu);
        }
        break;
        
        case PC_CHECKAPPACTIVEACK:
        {
            OnCheckAppActiveAck(pPdu);
        }
        break;
        case PC_GWSYNMESSAGETOAPPACK:
        {
            OnGwSynMessageToAppAck(pPdu);
        }
        break;
	    case LOGIN_CM_NOTIFY:
        {
            OnKitout(pPdu);//app logout
        }
        break;
        case SVR_LOGIN_RESULT:
        {
            OnKitout(pPdu);//app kitout
        }
        break;
        default:
        {

        }
    }

    return;

}

//from pc 
void DesktopMsgTran::OnLogin(std::shared_ptr<CImPdu> pPdu)
{
    Json::Reader reader;
    Json::Value value;

    if (!pPdu->GetBodyData())
    {
        return;
    }

    string strInput((char*)pPdu->GetBodyData(), (size_t)pPdu->GetBodyLength());
    if (!reader.parse(strInput, value)) {
        log("json parse failed: %s", strInput.c_str());
   
        return;
    }

    string msgId = value["msgid"].asString();
    string sToken = value["token"].asString();
    string registerId = value["registerid"].asString();
    string userId= value["userid"].asString();

    log("msg content  msgId = %s , sToken = %s, registerid = %s, userId =%s", 
        msgId.c_str(), sToken.c_str(), registerId.c_str(), userId.c_str());
    //ask php
    PCLoginAuth_t auth;
    auth.regid = registerId;
    auth.token = sToken;
    auth.sUserId = userId.c_str();

    UserCache_t userCache;
    string pcUserId = string(PREFIX_PC_STR) + userId; 
    do 
    {
        if(m_pCache->GetUserRecord(pcUserId.c_str(), userCache)) 
        {

            if(!CheckAuth(auth))
            {
                ErrLog("CheckAuth Error! errCode = %d", auth.errCode);
                // return;
                break;
            }

            CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(userId); 
            if(pUserLink!=NULL) 
            {
                char *kickoutRsp = NULL;
               // asprintf(&kickoutRsp, "{\"msgid\":\"%s\", \"registerid\":\"%s\", \"userid\":\"%s\"}",
               //          msgId.c_str(), registerId.c_str(), userId.c_str());
                //kitout old acount ,and the registerId is pre registerId.
                asprintf(&kickoutRsp, "{\"msgid\":\"%s\", \"registerid\":\"%s\", \"userid\":\"%s\"}",
                         msgId.c_str(), userCache.sDeviceToken.c_str(), userId.c_str());
                
                sendAck(kickoutRsp, PC_KITOUT, pUserLink->GetSessionId());
                InfoLog("pc_kitout  with sessionid = %s", pUserLink->GetSessionId().toString().c_str());
                free(kickoutRsp);
                kickoutRsp= NULL;
                //Kitout(userId);
            }

            string ip = "";
            uint16_t port = 0;
            CClientLinkMgr::GetInstance()->GetLocalHost(ip, port);
            userCache.sLoginToken = sToken.c_str();       
            userCache.sDeviceToken = registerId.c_str();  
            //userCache.sSessionId = pPdu->GetSessionId();     
            userCache.sIPAddr = ip.c_str();                 
            userCache.nIPPort = port;
            userCache.sUserId = userId.c_str();           
            if (!m_pCache->UpdateUserRec(pcUserId.c_str(), userCache))
            {
                ErrLog("update cash error!");

            }

            UpdateUserLink(pPdu->GetSessionId(), userId);
        }
        else
        {
            if(!CheckAuth(auth))
            {
                ErrLog("CheckAuth Error! errCode = %d", auth.errCode);
                //printf("CheckAuth Error! errCode = %d", auth.errCode);
                break;
            }

            string ip = "";
            uint16_t port = 0;
            CClientLinkMgr::GetInstance()->GetLocalHost(ip, port);
            userCache.sLoginToken = sToken.c_str();       
            userCache.sDeviceToken = registerId.c_str();  
            //userCache.sSessionId = pPdu->GetSessionId();     
            userCache.sIPAddr = ip.c_str();                 
            userCache.nIPPort = port;
            userCache.sUserId = userId.c_str();           
            if (!m_pCache->InsertUserRec(pcUserId.c_str(), userCache))
            {
                ErrLog("Insert cash error!");

            }

            UpdateUserLink(pPdu->GetSessionId(), userId);
        }
    }while(0); 
    
    char *loginRsp = NULL;
    asprintf(&loginRsp, "{\"msgid\":\"%s\", \"registerid\":\"%s\", \"userid\":\"%s\", \"errcode\":%d}",
             msgId.c_str(), registerId.c_str(), userId.c_str(), auth.errCode);

    sendAck(loginRsp, PC_LOGINACK, pPdu->GetSessionId());
    InfoLog("sendAck with sessionid = %s", pPdu->GetSessionId().toString().c_str());
    free(loginRsp);
    loginRsp = NULL;
}

bool DesktopMsgTran::CheckAuth(PCLoginAuth_t &auth)
{
    string str = m_sAppSecretChecking + "app_id2register_id" + auth.regid + "user_token" + auth.token +  m_sAppSecretChecking;
//    InfoLog("ready for signe str = %s", str.c_str());
    std::string strMD5 = MD5Str(str);
    transform(strMD5.begin(), strMD5.end(), strMD5.begin(), ::toupper);
    string strPost = "app_id=2&sign=" + strMD5 + "&user_token=" + auth.token + "&register_id=" + auth.regid;

    DbgLog("login auth post data %s", strPost.c_str());
    string sChkResult;
    CHttpClient httpClient;
    InfoLog("url=%s", m_sCheckAuthUrl.c_str());
    if(httpClient.Post(m_sCheckAuthUrl, strPost, sChkResult) != CURLE_OK)
    {
		ErrLog("http request fail! url=%s post=%s userid=%s", m_sCheckAuthUrl.c_str(), strPost.c_str(), auth.sUserId.c_str());
		return false;
    }
    InfoLog("The auth check result is :%s", sChkResult.c_str());

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

void DesktopMsgTran::OnLogout(std::shared_ptr<CImPdu> pPdu)
{
    Json::Reader reader;
    Json::Value value;

    if (!pPdu->GetBodyData())
    {
        return;
    }

    string strInput((char*)pPdu->GetBodyData(), (size_t)pPdu->GetBodyLength());
    if (!reader.parse(strInput, value)) {
        log("json parse failed: %s", strInput.c_str());
   
        return;
    }

    
    string msgid = value["msgid"].asString();
    string sToken = value["token"].asString();
    string regid = value["registerid"].asString();
    string userId= value["userid"].asString();

    log("msg content  msgId = %s , sToken = %s, regid = %s, userId =%s", 
        msgid.c_str(), sToken.c_str(), regid.c_str(), userId.c_str());

    //check cash for user info to closeLink;
    string linkuser = userId;
    CClientLink* pLink =  CClientLinkMgr::GetInstance()->GetLinkByUserId(linkuser);
    if(pLink)
    {    
        InfoLog("linker %p will be closed , bind with user %s", pLink, linkuser.c_str());
        pLink->CloseLink();
        pLink->ReleaseRef();
    }

    //remove redis recoder
	std::string pcUserId = string(PREFIX_PC_STR) + userId;
    if(m_pCache)
    {
        if(m_pCache->DelUserRec(pcUserId.c_str()))
        {
            InfoLog("del pc cache successfully!");
        }
    }
}

void DesktopMsgTran::OnPCSynMessageToGw(std::shared_ptr<CImPdu> pPdu)
{
    //InfoLog("recv pPdu = %p", pPdu.get());
    Json::Reader reader;
    Json::Value value;

    if (!pPdu->GetBodyData())
    {
        return;
    }

    string strInput((char*)pPdu->GetBodyData(), (size_t)pPdu->GetBodyLength());
    if (!reader.parse(strInput, value)) {
        log("json parse failed: %s", strInput.c_str());
   
        return;
    }
    InfoLog("recv messages %s", strInput.c_str());
    //json phrase
    string fromid = value["fromid"].asString();
    string toid = value["toid"].asString();
    uint32_t synctype = 0;
    if(value["synctype"].isInt())
    {
        synctype = value["synctype"].asInt();
    }
    string msgid = value["msgid"].asString();
    string synmessage = value["synmessage"].asString();
    string extend = value["extend"].asString();
    
    string ip= "";
    int port = 0;
	std::string cmUserId = string(PREFIX_CM_STR) + fromid;
    std::shared_ptr<CLoginInfo> pLogin =  m_pCache->GetLoginInfo(cmUserId);
    if(pLogin) 
    {
        if(pLogin->IsLogin())
        {
            ip = pLogin->GetCmIp();
            port = atoi(pLogin->GetCmPort().c_str());
            //trans to pb               
            im::GwSynMessageToApp msg;  
            msg.set_sfromid(fromid);    
            msg.set_stoid(toid);        
            msg.set_synctype(synctype); 
            msg.set_msgid(msgid);       
            msg.set_synmsg(synmessage); 
            msg.set_extended(extend);     
            
            sendReq(&msg, PC_GWSYNMESSAGETOAPP, ip, port);
            InfoLog("send msg to %s , msg = %s", toid.c_str(), strInput.c_str());
        }
        else
        {
            //drop temporary and wakeup app and store later stage
            USER_INFO_ userInfo;
            string errCode;
            CGetDataInterfaceManager::GetInstance()->getUserInfo(m_sUserInfoUrl, m_sAppSecretChecking, fromid, userInfo, errCode);
            sendPush(fromid, fromid, msgid, im::APP_WAKEUP, "app wakeup", userInfo.pushType, userInfo.pushToken, userInfo.voipToken, 3);
            //InfoLog("send app_wakeup to ipush");
            InfoLog("app offline, drop the msg and wakeup app or store for resending");
        }
    }
    else
    {
        InfoLog("App offline, Cache clean ,do nothing");
    }
}

void DesktopMsgTran::OnGwSynMessageToPCAck(std::shared_ptr<CImPdu> pPdu)
{
    Json::Reader reader;
    Json::Value value;

    if (!pPdu->GetBodyData())
    {
        return;
    }

    string strInput((char*)pPdu->GetBodyData(), (size_t)pPdu->GetBodyLength());
    if (!reader.parse(strInput, value)) {
        log("json parse failed: %s", strInput.c_str());
   
        return;
    }

    InfoLog("revc ack message = %s from pc ", strInput.c_str());
    string fromid = value["fromid"].asString();
    string msgid =  value["msgid"].asString();

    //check the user which cm connected
    //CLogin loginInfo;
    string ip= "";
    int port = 0;
	std::string cmUserId = string(PREFIX_CM_STR) + fromid;
    std::shared_ptr<CLoginInfo> pLogin =  m_pCache->GetLoginInfo(cmUserId);
    if(pLogin) 
    {
        if(pLogin->IsLogin())
        {
            ip = pLogin->GetCmIp();
            port = atoi(pLogin->GetCmPort().c_str());
            
            im::AppSynMessageToGwAck msg;
            msg.set_sfromid(fromid);
            msg.set_msgid(msgid);
            sendReq(&msg, PC_APPSYNMESSAGETOGWACK, ip, port);
        }
        else
        {
            //drop temporary and wakeup app and store later stage
            USER_INFO_ userInfo;
            string errCode;
            CGetDataInterfaceManager::GetInstance()->getUserInfo(m_sUserInfoUrl, m_sAppSecretChecking, fromid, userInfo, errCode);
            sendPush(fromid, fromid, msgid, im::APP_WAKEUP, "app wakeup", userInfo.pushType, userInfo.pushToken, userInfo.voipToken, 3);
            //InfoLog("send app_wakeup to ipush");
            InfoLog("app offline, drop the msg and wakeup app or store for resending");
        }
    }
    else
    {

        InfoLog("App offline, Cache clean, do nothing");
    }
}


void DesktopMsgTran::OnHeartBeat(std::shared_ptr<CImPdu> pPdu)
{
    Json::Reader reader;
    Json::Value value;

    if (!pPdu->GetBodyData())
    {
        return;
    }

    string strInput((char*)pPdu->GetBodyData(), (size_t)pPdu->GetBodyLength());
    if (!reader.parse(strInput, value)) {
        log("json parse failed: %s", strInput.c_str());
   
        return;
    }
    InfoLog("recv heartbeat msg = %s", strInput.c_str());
    string msgid = value["msgid"].asString();
    string fromid = value["fromid"].asString();
    int64_t msgtime = 0;
    if(value["time"].isInt64())
    {
       msgtime =  value["time"].asInt64();
    }
    //HeartBeatAck();//send back to pc directly
    InfoLog("recv pPdu sessionid = %s", pPdu->GetSessionId().toString().c_str());
    sendAck(strInput, PC_HEARTBEARTACK, pPdu->GetSessionId());
    //trans to pb send to cm
    //check Login Info ,find fromid which cm connect;
    string ip = "";
    int port = 0;
	std::string cmUserId = string(PREFIX_CM_STR) + fromid;
    std::shared_ptr<CLoginInfo> pLogin =  m_pCache->GetLoginInfo(cmUserId);
    if(pLogin)
    {
        //online send msg, offline send push
        if (pLogin->IsLogin()) 
        {
            ip = pLogin->GetCmIp();
            port = atoi(pLogin->GetCmPort().c_str());

            im::CheckAppActive msg;
            msg.set_msgid(msgid);
            msg.set_sfromid(fromid);
            msg.set_time(msgtime);

            sendReq(&msg, PC_CHECKAPPACTIVE, ip, port);
            InfoLog("send PC_CHECKAPPACTIVE to cm");
        } 
        else
        {
            USER_INFO_ userInfo;
            string errCode;
            CGetDataInterfaceManager::GetInstance()->getUserInfo(m_sUserInfoUrl, m_sAppSecretChecking, fromid, userInfo, errCode);
            sendPush(fromid, fromid, msgid, im::APP_WAKEUP, "app wakeup", userInfo.pushType, userInfo.pushToken, userInfo.voipToken, 3);
            InfoLog("send app_wakeup to ipush");
        }
    }
    else
    {
        InfoLog("App offline, Cache clean, do nothing");
    }
}

//from cm
void DesktopMsgTran::OnAppSynMessageToGw(std::shared_ptr<CImPdu> pPdu)
{
    if(!pPdu)
    {
        ErrLog("pPdu is empty!");
        return;
    }

    im::AppSynMessageToGw msg;
    if(!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        ErrLog("pb phrase is error!");
        return;
    }

    //construct json based on msg
    string fromid = msg.sfromid();//value["fromid"].asString();
    string toid = msg.stoid(); //value["toid"].asString();
    uint32_t synctype = msg.synctype();//value["synctype"].asString();
    string msgid = msg.msgid();// value["msgid"].asString();
    string synmessage = msg.synmsg();//value["synmessage"].asString();
    string extend = msg.extended();//value["extend"].asString();
    //string jsonStr = synmessage.toJsonStr();
    InfoLog("msgid = %s, synmessage = %s", msgid.c_str(), msg.synmsg().c_str());
    char *longJson = NULL;
    asprintf(&longJson, "{\"fromid\":\"%s\", \"toid\":\"%s\", \"synctype\":%d, \"msgid\":\"%s\",\"synmessage\":\"%s\"}",fromid.c_str(), toid.c_str(),  synctype, msgid.c_str(), synmessage.c_str());
   // asprintf(&longJson, "{\"fromid\":\"%s\", \"toid\":\"%s\", \"synctype\":%d, \"msgid\":\"%s\",\"synmessage\":\"%s\", \"extend\":\"%s\"}",
   //          fromid.c_str(), toid.c_str(),  synctype, msgid.c_str(), synmessage.c_str(), extend.c_str());  
    InfoLog("recv dwappsynmessagetogw %d, content = %s,  tans to gwsynmessagetopc=%s to pc",pPdu->GetCommandId(), synmessage.c_str(), longJson);
    //whether we need to check user online or not?
    //get linke by userid , and find the linker to pc for the sessionid;
    //string linkuser = string(PREFIX_PC_STR) + fromid;
    string linkuser =  fromid;
    CClientLink* pLink =  CClientLinkMgr::GetInstance()->GetLinkByUserId(linkuser);
    if(pLink)
    {    
        sendAck(longJson, PC_GWSYNMESSAGETOPC, pLink->GetSessionId());
    }
    
    free(longJson);
    longJson = NULL;
}

void DesktopMsgTran::OnCheckAppActiveAck(std::shared_ptr<CImPdu> pPdu)
{
    if(!pPdu)
    {
        ErrLog("pPdu is empty!");
        return;
    }

    im::CheckAppActiveAck msg;
    if(!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        ErrLog("pb phrase is error!");
        return;
    }

    string msgid = msg.msgid();
    string fromid = msg.sfromid();
    int64_t  msgtime = msg.time();

    char* longJson = NULL;
    asprintf(&longJson, "{\"msgid\":\"%s\", \"fromid\":\"%s\", \"time\":%ld}", msgid.c_str(), fromid.c_str(), msgtime);
    InfoLog("recv checkappactiveack %d ,trans to Json ReportAppActive=%s to pc",pPdu->GetCommandId(), longJson);

    //whether we need to check user online or not?
    //get linke by userid , and find the linker to pc for the sessionid;
    //string linkuser = string(PREFIX_PC_STR) + fromid;
    string linkuser = fromid;
    CClientLink* pLink =  CClientLinkMgr::GetInstance()->GetLinkByUserId(linkuser);
    if(pLink)
    {    
        sendAck(longJson, PC_REPORTAPPACTIVE, pLink->GetSessionId());
    }

    free(longJson);
    longJson = NULL;
}

void DesktopMsgTran::OnGwSynMessageToAppAck(std::shared_ptr<CImPdu> pPdu)
{
    if(!pPdu)
    {
        ErrLog("pPdu is empty!");
        return;
    }

    im::GwSynMessageToAppAck msg;
    if(!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        ErrLog("pb phrase error!");
        return;
    }
    //trans to json
    string fromid = msg.sfromid();
    string msgid = msg.msgid();
    //fromid
    char *longJson = NULL;
    asprintf(&longJson, "{\"fromid\":\"%s\", \"msgid\":\"%s\"}", fromid.c_str(), msgid.c_str());
    InfoLog("recv gwsynmessagetoappack %d , trans to Json PcSynMessageToGwAck= %s to pc", pPdu->GetCommandId(), longJson);
    
    //whether we need to check user online or not?
    //get linke by userid , and find the link to pc for the sessionid;
    //string linkuser = string(PREFIX_PC_STR) + fromid;
    string linkuser = fromid;
    CClientLink* pLink =  CClientLinkMgr::GetInstance()->GetLinkByUserId(linkuser);
    if(pLink)
    {    
        sendAck(longJson, PC_PCSYNMESSAGETOGWACK, pLink->GetSessionId());
    }

    free(longJson);
    longJson = NULL;
}

