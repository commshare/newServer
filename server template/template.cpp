/*****************************************************************************************
Filename: usermanage.h
Author: jack			Version: im-1.0 		Date:2017/06/14
Description: 	用户登录/登出、权限检查、用户信息管理类实现
*****************************************************************************************/

#include "usermanage.h"

CTemplate::CTemplate(CConfigFileReader* pConfigReader)
	: m_pCache(0),m_sCheckAuthUrl(""),m_nCheckInterval(0),m_pConfigReader(pConfigReader)
{

}

CTemplate::~CTemplate()
{
	
}

bool CTemplate::Initialize(void)  
{
	//Add your statment to load config parameter about user manage.
	if(SetCheckAuthUrl()==false)				//Load config parameter from config file.
		return false;
	else if(SetCheckAuthInterval()== false)
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

bool CTemplate::SetCheckAuthUrl()
{
	if(!m_pConfigReader)
		return false;

	char* pAuth = 	m_pConfigReader->GetConfigName("check_url");
	if(pAuth==NULL)
		return false;
	m_sCheckAuthUrl = pAuth;

	return true;
}

bool CTemplate::SetCheckAuthInterval()         //Load checking user interval . 
{
	if(!m_pConfigReader)
		return false;

	char* pAuth =	m_pConfigReader->GetConfigName("check_interval"); 
	if(pAuth==NULL)
		return false;
	m_nCheckInterval = atoi(pAuth);

	return true;
}

bool CTemplate::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(CM_LOGIN, CommandProc(&CTemplate::OnLogin));
	CmdRegist(CM_LOGOUT, CommandProc(&CTemplate::OnLogout));
	CmdRegist(SYSTEM_TIMEOUT_NOTIFICATION, CommandProc(&CTemplate::OnTimer));
}



bool CTemplate::OnLogin(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing user login!");
		return false;
	}

	string sUserId = PREFIX_CM_STR;
	uint8_t bUserStatus = USER_OFFLINE;
	string sSessionId;
	uchar_t* pContent = NULL;
	bool bAuthCheck = false;
	CMLogin 	login;
	UserAuth_t  userAuth;
	UserCache_t userCache;
	
	
	DbgLog("Execute OnLogin ,0x%x....",pPdu->GetCommandId());

	pContent = pPdu->GetBodyData();
	if(!pContent || !login.ParseFromArray(pContent,pPdu->GetBodyLength()))
	{
		ErrLog("Login parameter error!");
		return false;
	}

	userAuth.sUserId = login.suserid();			//Pure userid ,
	userAuth.sLoginToken = login.susertoken();
	userAuth.bRole = 0;
	sUserId += userAuth.sUserId;				//UserId with 'CM' plugin to use in redis cache. 
	sSessionId = pPdu->GetSessionId();
	
	DbgLog("Now logining ... body cmd = 0x%x ,user=%s,session=%s,body size = %d",\
		pPdu->GetCommandId(),sUserId.c_str(),sSessionId.c_str(),pPdu->GetBodyLength());
	return true;
}

bool CTemplate::OnLogout(std::shared_ptr<CImPdu> pPdu)
{
}
bool CTemplate::OnTimer(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
		return false;
	
	string sSessionId = pPdu->GetSessionId();

	DbgLog("user ontimer,session id = %s",sSessionId.c_str());
	CClientLink* pCurLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sSessionId);
	if(!pCurLink)
	{
		ErrLog("Find a invalid link when processing link timeout event");
		return false;
	}
		
	string sUserId = pCurLink->GetUserId();
	pCurLink->Close();
	if(m_pCache->SetUserStatus(sUserId.c_str(),0)==false)
	{
		ErrLog("Failed to set user %s redis status",sUserId.c_str());
		return false;
	}

}


void CTemplate::LoginRsp(string sUserId,string sSessionId,ErrCode bCode)
{
	CMLoginAck 	loginAck;
	CImPdu 		loginAckPdu;

	loginAck.set_suserid(sUserId);
	loginAck.set_nerr(bCode);
	loginAckPdu.SetPBMsg(&loginAck);
	loginAckPdu.SetCommandId(CM_LOGIN_ACK);
	loginAckPdu.SetSessionId(sSessionId);
	
	SendPdu(&loginAckPdu);
}



