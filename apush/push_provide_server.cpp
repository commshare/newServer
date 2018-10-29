/*****************************************************************************************
Filename: push_provide_server.cpp
Author:  lang
Description: 	
 *****************************************************************************************/

#include "im.push.android.pb.h"
#include "hw_handle_protobuf.h"
#include "hw_push_client.h"
#include "hw_ssl_get_token.h"
#include "xmpp_fcm_client.h"
#include "push_provide_server.h"

std::string rfc1738_encode(const std::string& src, bool bigOrLittle)
{

	char hex[] = "0123456789ABCDEF";
	if (bigOrLittle == true)
	{
		strcpy(hex, "0123456789abcdef");
	}


	std::string dst;
	for (size_t i = 0; i < src.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(src[i]);
		if (isalnum(c))
		{
			dst += c;
		}
		else
			if (c == ' ')
			{
			dst += '+';
			}
			else
			{
				dst += '%';
				dst += hex[c / 16];
				dst += hex[c % 16];
			}
	}
	return dst;
}

CLock *CApushLocalSvr::m_pLockSendResp = NULL;

CApushLocalSvr::CApushLocalSvr(CConfigFileReader* pConfigReader,int nNumOfInst) :
	m_pConfigReader(pConfigReader),m_nNumberOfInst(nNumOfInst)
{
}

CApushLocalSvr::~CApushLocalSvr()
{
	
	
}

bool CApushLocalSvr::Initialize(void)  
{
	
	//the source code is not set the return value;
	RegistPacketExecutor();

	m_pManage =  CApushLocalManager::GetInstance();
	if (!m_pManage)
	{
		ErrLog("CApushLocalSvr Initialize");
		return false;
	}

	if (!m_pManage->RegistSvr(this))
	{
		ErrLog("CApushLocalSvr StartApp");
		return false;
	}


	if(!m_pConfigReader)
	{
		ErrLog("m_pConfigReader null");
		return false;
	}

	m_strClientId = m_pConfigReader->GetConfigName("HwClientId");
	m_strVer = m_pConfigReader->GetConfigName("HwPushVer");

	if (m_strClientId.empty())
	{
		InfoLog("HwClientId is not set use default: 100042631");
		m_strClientId =  Default_HW_clientId;
	}

	if (m_strVer.empty())
	{
		InfoLog("HwPushVer is not set use default: 1");
		m_strVer = Default_HW_Ver;
	}

	if(!CHwPostSendBuf::Init(m_strVer.c_str(), m_strClientId.c_str()))
	{
		ErrLog("CHwPostSendBuf::Init");
		return false;
	}

	StartThread();

	return true;
}

bool CApushLocalSvr::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{

	//ANDROID_PUSH = 53255,
	//ANDROID_PUSH_ACK = 53256,
	CmdRegist(ANDROID_PUSH, m_nNumberOfInst, CommandProc(&CApushLocalSvr::OnAndroidPush));
	CmdRegist(ANDROID_NOTIFY_ACK, m_nNumberOfInst, CommandProc(&CApushLocalSvr::OnNotifyAck));
	return true;
}

string CApushLocalSvr::GetToken(bool bNecessary /*= false*/)
{
	//by default if the (now - requested) time is less than a half of Expires
	//don`t to request the Push Token again
	if (!bNecessary)
	{
		if(CHwGetPushTokenClient::uTimeTick != 0 &&
		   CHwGetPushTokenClient::m_uExpires != 0	&&
		   !m_strToken.empty() &&
		   (time(NULL) - CHwGetPushTokenClient::uTimeTick < CHwGetPushTokenClient::m_uExpires/2))
		{
				return m_strToken;
		}
	}
	
	if (!m_pConfigReader)
	{
		ErrLog("CHWPushClient m_configFileReader is null");
		return m_strToken;
	}

	CHwGetPushTokenClient *pTokenClient = new CHwGetPushTokenClient;
	if (!pTokenClient)
	{
		ErrLog("CHWPushClient new pTokenClient");
		return m_strToken;
	}

	if (!pTokenClient->Init(m_pConfigReader))
	{
		ErrLog("CHWPushClient pTokenClient init");
		delete pTokenClient;
		return m_strToken;
	}

	string strToken = pTokenClient->Post_ToGetToken();
	if (strToken.empty())
	 {
		 ErrLog("CHWPushClient new pTokenClient");

		 delete pTokenClient;
		 return strToken;
	 }

	for (string::iterator it = strToken.begin(); it != strToken.end(); )
	{
		if (*it == '\\')
		{
			strToken.erase(it);
		}
		else
		{
			it++;
		}
	}

	m_strToken = rfc1738_encode(strToken);

	delete pTokenClient;

	return m_strToken;
}


//debuf
static int testcount = 0;

bool CApushLocalSvr::OnAndroidPush(std::shared_ptr<CImPdu> pPdu)
{
	testcount++;
	printf("onPush:%d\n", testcount);
	InfoLog("onPush:%d\n", testcount);
	//string sMsgId;
	UidCode_t  sessionId = pPdu->GetSessionId();

	int iRet = -1;

//	DbgLog("Success to regist sessionId=%x%x%x%x%x%x%x%x%x%x%x%x",
//    sessionId.Uid_Item.code[0],sessionId.Uid_Item.code[1],
//	sessionId.Uid_Item.code[2],sessionId.Uid_Item.code[3],sessionId.Uid_Item.code[4],
//	sessionId.Uid_Item.code[5],sessionId.Uid_Item.code[6],sessionId.Uid_Item.code[7],
//	sessionId.Uid_Item.code[8],sessionId.Uid_Item.code[9],sessionId.Uid_Item.code[10],
//	sessionId.Uid_Item.code[11]);

	//bool bRet = false;
	ErrCode eCode = EXCEPT_ERR;

	//CBaseClient *pClient = nullptr;

	m_strToken = GetToken();

	CProtoHandele proto;

	shared_ptr<APushData> shared_APushData = proto.GetaPushData((const char *)pPdu->GetBodyData(), pPdu->GetBodyLength(), m_strToken);

	APushData *data = shared_APushData.get();
	if (!data)
	{
		ErrLog("GetaPushData");

		return false;
		//goto goEnd;
	}

	data->sessionId = sessionId;
	
    CBaseClient *pclient= NULL;
    pclient = m_pManage->GetClient(data->diveceType);
    if (pclient) {
        iRet = pclient->AddTask(shared_APushData);
    } else {
        ErrLog("dones't support deiveceType %d", data->diveceType);
        return false;
    }

	if (iRet <= 0)
	{
		ErrLog("OnAndroidPush AddTask");

		OnNotify(shared_APushData);

		goto goEnd;
	}

	eCode =  NON_ERR;

goEnd:
	OnAndroidPushAck(data->msgId, sessionId, eCode);
	return true;

	
	//return bRet;
}

void CApushLocalSvr::OnAndroidPushAck(string sMsgId, UidCode_t sessionId, ErrCode bCode)
{

//	DbgLog("Success to regist sessionId=%x%x%x%x%x%x%x%x%x%x%x%x",
//    sessionId.Uid_Item.code[0],sessionId.Uid_Item.code[1],
//	sessionId.Uid_Item.code[2],sessionId.Uid_Item.code[3],sessionId.Uid_Item.code[4],
//	sessionId.Uid_Item.code[5],sessionId.Uid_Item.code[6],sessionId.Uid_Item.code[7],
//	sessionId.Uid_Item.code[8],sessionId.Uid_Item.code[9],sessionId.Uid_Item.code[10],
//	sessionId.Uid_Item.code[11]);


	if (!CApushLocalSvr::m_pLockSendResp)
	{
		CApushLocalSvr::m_pLockSendResp  = new CLock;
	}

	CAutoLock autolock(CApushLocalSvr::m_pLockSendResp);
	
    ANDPushMsgAck  androidPushAck;
    CImPdu      AckPdu;
                                                   
    androidPushAck.set_smsgid(sMsgId);
    androidPushAck.set_nerr(bCode);
                                                   
    AckPdu.SetPBMsg(&androidPushAck);         
                                                   
    AckPdu.SetCommandId(ANDROID_PUSH_ACK);    
    AckPdu.SetSessionId(sessionId);       
                                                   
    if(SendPdu(&AckPdu, 0) < 0)               
    {                                              
        //ErrLog("OnApnsPushAck:%d\n", testcountErr);
    }                                              

}

static int testNotify = 0;                                        
bool CApushLocalSvr::OnNotify(shared_ptr<APushData> shared_APushData)             
{
	if (!shared_APushData)
	{
		ErrLog("shared_APushData");
		return false;
	}
	
	APushData *pData = shared_APushData.get();
	if (!pData)
	{
		ErrLog("shared_APushData get");
		return false;
	}

    if (!CApushLocalSvr::m_pLockSendResp)
    {
        CApushLocalSvr::m_pLockSendResp  = new CLock;
    }

    ANDNotify     androidNotify;
    CImPdu              notifyPdu;
	androidNotify.set_smsgid(pData->msgId);
	androidNotify.set_stoid(pData->toId);
	androidNotify.set_nerr(pData->retStatus);

    notifyPdu.SetPBMsg(&androidNotify);

    notifyPdu.SetCommandId(ANDROID_NOTIFY);
	notifyPdu.SetSessionId(pData->sessionId);

	CAutoLock autolock(CApushLocalSvr::m_pLockSendResp);
    if(SendPdu(&notifyPdu, 0)< 0)
    {
        //ErrLog("OnApnsNotify:%d\n", testcountErr);
    }

    testNotify++;
	static int tmBeg = 0;
	if (testNotify == 1)
	{
		tmBeg = (int)time(NULL);
	}
	
	int tmNow = (int)time(NULL);

	int tmSub = tmNow - tmBeg;
	if (tmSub <= 0)
	{
		tmSub = 1;
	}

	printf("testNotify:%d, speed:%d/s\n", testNotify, testNotify/tmSub);
	InfoLog("testNotify:%d, speed:%d/s\n", testNotify, testNotify/tmSub);

//	DbgLog("Success to regist pData->sessionId=%x%x%x%x%x%x%x%x%x%x%x%x",
//pData->sessionId.Uid_Item.code[0],pData->sessionId.Uid_Item.code[1],
//pData->sessionId.Uid_Item.code[2],pData->sessionId.Uid_Item.code[3],pData->sessionId.Uid_Item.code[4],
//pData->sessionId.Uid_Item.code[5],pData->sessionId.Uid_Item.code[6],pData->sessionId.Uid_Item.code[7],
//pData->sessionId.Uid_Item.code[8],pData->sessionId.Uid_Item.code[9],pData->sessionId.Uid_Item.code[10],
//pData->sessionId.Uid_Item.code[11]);

    return true;                                                  
}                                                                 
                                                                  
                                                                  
void CApushLocalSvr::OnNotifyAck(std::shared_ptr<CImPdu> pPdu)
{      
	NOTUSED_ARG(pPdu);
    //do nothing
	//
	InfoLog("OnNotifyAckOnNotifyAckOnNotifyAckOnNotifyAck\n");                                                
    return;                                                       
}                                                                 


//
