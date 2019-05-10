#include "im_push_frame.h"
#include "configfilereader.h"
#include "pushHandler.h"
#include "voip_http2_client.h"
CIMPushFrame::CIMPushFrame(CConfigFileReader * pReader)
	: m_pConfigReader(pReader), m_nActualServiceInst(0)
{
}
	
CIMPushFrame::~CIMPushFrame()
{
	StopApp();
}

bool CIMPushFrame::Initialize()
{
	if (!m_pConfigReader)
	{
		ErrLog("CIMPushFrame Initialize");
		return false;
	}
	char* pInstNumber = m_pConfigReader->GetConfigName("actual_instant_number");
	if (pInstNumber == NULL)
	{
		ErrLog("The actual service configure item : actual_instant_number not spcified!");
		return false;
	}

	m_nActualServiceInst = atoi(pInstNumber);
	if (m_nActualServiceInst > MAX_INSTANCE_SERVICE)
	{
		ErrLog("The actual service inst configure item : actual_instant_number can't over %d!", MAX_INSTANCE_SERVICE);
		return false;
	}
	CPacketMgr::GetInstance()->Initialize(m_nActualServiceInst);

	return true;
}

bool CIMPushFrame::StartApp()
{
	m_pVoipHttp2DevClient = new CVoipHttp2Client(PUSH_CLIENT_TYPE_VOIP_DEV);

	if (!m_pVoipHttp2DevClient)
	{
		ErrLog("CVoipHttp2DevClient StartApp");
		return false;
	}

	if (! m_pVoipHttp2DevClient->init(m_pConfigReader))
	{
		ErrLog("CVoipHttp2DevClient StartApp");
		return false;
	}
    //

	m_pVoipHttp2ProClient = new CVoipHttp2Client(PUSH_CLIENT_TYPE_VOIP_PRODUCTION);
	if (!m_pVoipHttp2ProClient )
	{
		ErrLog("CVoipHttp2ProClient StartApp");
		return false;
	}

	if (! m_pVoipHttp2ProClient->init(m_pConfigReader))
	{
		ErrLog("CVoipHttp2ProClient StartApp");
		return false;
	}
	for (int i = 0; i < m_nActualServiceInst; i++)
	{

		m_pSvrToLogic[i] = new CPushHandler(m_pConfigReader,i);
		if (!m_pSvrToLogic[i])
		{
			ErrLog("Err of m_pSvrToLogic!");
			return false;
		}

		if(!m_pSvrToLogic[i]->Initialize())
		{
			ErrLog("Err of m_pSvrToLogic  initialization!");
			return false;
		}
	}

	// End 
	return true;
	
}

void CIMPushFrame::StopApp()
{
	if(m_pVoipHttp2DevClient)
	{
		delete m_pVoipHttp2DevClient;
		m_pVoipHttp2DevClient = nullptr;
	}

	if(m_pVoipHttp2ProClient)
		delete m_pVoipHttp2ProClient;
		m_pVoipHttp2ProClient = nullptr;

	for (int i = 0; i < m_nActualServiceInst; i++)
	{
		if (m_pSvrToLogic[i])
		{
			delete m_pSvrToLogic[i];
			m_pSvrToLogic[i] = nullptr;
		}
	}

}


