#include "imappframe.h"
#include "configfilereader.h"
#include "packetmgr.h"
#include "desktopmsgtran.h"

CIMAppFrame::CIMAppFrame(CConfigFileReader * pReader)
	: m_pConfigReader(pReader),m_nActualServiceInst(0)
{
}
	
CIMAppFrame::~CIMAppFrame()
{
	StopApp();
}

bool CIMAppFrame::Initialize()
{
	char* pInstNumber = m_pConfigReader->GetConfigName("actual_instant_number");
	if(pInstNumber==NULL)
		return false;

	m_nActualServiceInst = atoi(pInstNumber);
	if(m_nActualServiceInst > MAX_INSTANCE_SERVICE)
	{
		ErrLog("The actual service inst can't over %d!",MAX_INSTANCE_SERVICE);
		return false;
	}
	
	CPacketMgr::GetInstance()->Initialize(m_nActualServiceInst);

	return true;
}

bool CIMAppFrame::StartApp()
{
	bool bRet = true;
	for(int i = 0;  i < m_nActualServiceInst; i++)
	{

		m_pDesktopMsgTran[i] = new DesktopMsgTran(m_pConfigReader,i);
		if(!m_pDesktopMsgTran[i] || (false==m_pDesktopMsgTran[i]->Initialize()))
		{
			bRet = false;
			break;
		}
	}

	return bRet;
	
}

void CIMAppFrame::StopApp()
{
	int i;
	
	for(i = 0;	i < m_nActualServiceInst; i++)
	{
		if(m_pDesktopMsgTran[i])
		{
			delete m_pDesktopMsgTran[i];
		}	
	}

}

