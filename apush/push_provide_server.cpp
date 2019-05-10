/*****************************************************************************************
Filename: push_provide_server.cpp
Author:  lang
Description: 	
 *****************************************************************************************/

#include "im.push.android.pb.h"
#include "hw_handle_protobuf.h"
#include "hw_push_client.h"
#include "hw_ssl_get_token.h"
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
	m_pConfigReader(pConfigReader),m_nNumberOfInst(nNumOfInst),m_uExpires(0), uTimeTick(0)
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
		if(uTimeTick != 0 &&
		   m_uExpires != 0	&&
		   !m_strToken.empty() &&
		   (time(NULL) - uTimeTick < m_uExpires/2))
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

//	string strToken = pTokenClient->Post_ToGetToken();
	string strToken = pTokenClient->Post_ToGetToken(m_uExpires, uTimeTick);
	if (strToken.empty())
	 {
		 ErrLog("CHWPushClient get new pTokenClient failed");

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
	InfoLog("onPush:%d\n", testcount);
	UidCode_t  sessionId = pPdu->GetSessionId();

	int iRet = -1;
	ErrCode eCode = EXCEPT_ERR;

	m_strToken = GetToken();
    InfoLog("Thread %lu Get hwToken = %s, currentTime = %u, token expirtime = %u",pthread_self(),  m_strToken.c_str(), uTimeTick, m_uExpires); 
	CProtoHandele proto;

	vector<shared_ptr<HTTP_REQDATA_>> shared_APushData = proto.GetAndroidPushData((const char *)pPdu->GetBodyData(), pPdu->GetBodyLength(), m_strToken);

    for(int i = 0; i < shared_APushData.size(); i++) 
    {
        P_HTTP_REQDATA_ pdata =  shared_APushData[i].get();
        if (pdata) {
            DbgLog("receive msg => devType=%d, msgId=%s, url=%s, postData=\n\"%s\"\n\n", pdata->diveceType, pdata->msgId.c_str(), pdata->strUrl_.c_str(), pdata->strPost_.c_str());
        }

        P_HTTP_REQDATA_ data = shared_APushData[i].get();
        if (!data)
        {
            ErrLog("GetaPushData");

            return false;
            //goto goEnd;
        }

        data->sessionId = sessionId;

        CHttpBase *pclient= NULL;
        pclient = m_pManage->GetClient(data->diveceType);
        if(pclient) {
            pclient->AddTask(shared_APushData[i]);
        } else {

            ErrLog("dones't support deiveceType %d", data->diveceType);
            return false;
        }
    }
    return true;
}

void CApushLocalSvr::OnAndroidPushAck(string sMsgId, UidCode_t sessionId, ErrCode bCode)
{
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

    return true;                                                  
}                                                                 
                                                                  
                                                                  
void CApushLocalSvr::OnNotifyAck(std::shared_ptr<CImPdu> pPdu)
{      
	NOTUSED_ARG(pPdu);
	InfoLog("OnNotifyAckOnNotifyAckOnNotifyAckOnNotifyAck\n");                                                
    return;                                                       
}                                                                 


