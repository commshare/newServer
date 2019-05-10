#include "configfilereader.h"
#include "imappframe.h"
#include "packetmgr.h"
#include "notifySendAck.h"

CIMAppFrame::CIMAppFrame(CConfigFileReader * pReader)
    : m_pConfigReader(pReader), m_nActualServiceInst(1)
{
}
	
CIMAppFrame::~CIMAppFrame()
{
	StopApp();
}

bool CIMAppFrame::Initialize()
{
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

bool CIMAppFrame::StartApp()
{
	m_pNotifySendAck = new CNotifySendAck();
	if(nullptr == m_pNotifySendAck)
	{
		ErrLog("create NotifySendAck handle failed");
			return false;
	}
	if (false == m_pNotifySendAck->Initialize())
	{
		ErrLog("init NotifySendAck handle failed");
			return false;
	}

	return true;
	
}

void CIMAppFrame::StopApp()
{
	if(nullptr != m_pNotifySendAck)
	{
		delete m_pNotifySendAck;
	}
}

