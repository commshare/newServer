#include "imappframe.h"


CIMAppFrame::CIMAppFrame(CConfigFileReader * pReader)
	: m_pConfigReader(pReader)
{
}
	
CIMAppFrame::~CIMAppFrame()
{
	StopApp();
}

bool CIMAppFrame::Initialize()
{
	//Add your statment to load config parameter about user manage.
	//...	
	//

}

bool CIMAppFrame::StartApp()
{
	// Add your statement to create your application instance.

		
	
	m_pTemplate =  new CTemplate(m_pConfigReader);
	if(!m_pTemplate)
		return false;
	if(false==m_pTemplate->Initialize())
		return false;
	
	

	// End 
	return true;
	
}

void CIMAppFrame::StopApp()
{
	if(m_pTemplate)
	{
		delete m_pTemplate;
	}
	
	...
}

