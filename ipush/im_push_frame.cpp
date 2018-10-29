#include "im_push_frame.h"
#include "configfilereader.h"
#include "pushHandler.h"
#include "apnsclient.h"
#include "voip_push_client.h"

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

	m_pApnsClient = new CApnsClient;
	if (!m_pApnsClient)
	{
		ErrLog("CIMPushFrame StartApp");
		return false;
	}

	if (!m_pApnsClient->init(m_pConfigReader))
	{
		ErrLog("CIMPushFrame StartApp");
		return false;
	}

	m_pVoipPushClient = new CVoipPushClient;
	if (!m_pVoipPushClient)
	{
		ErrLog("CVoipPushClient StartApp");
		return false;
	}

	if (!m_pVoipPushClient->init(m_pConfigReader))
	{
		ErrLog("CVoipPushClient StartApp");
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
	/*
	if(m_pSvrToLogic)
	{
		delete m_pSvrToLogic;
		m_pSvrToLogic = nullptr;
	}
	
	*/

	if(m_pApnsClient)
	{
		delete m_pApnsClient;
		m_pApnsClient = nullptr;
	}

	for (int i = 0; i < m_nActualServiceInst; i++)
	{
		if (m_pSvrToLogic[i])
		{
			delete m_pSvrToLogic[i];
			m_pSvrToLogic[i] = nullptr;
		}
	}


	//...
}

