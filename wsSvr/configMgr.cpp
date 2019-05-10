#include "configMgr.h"

CConfigMgr::CConfigMgr()
{
	m_pConfigReader = NULL;
}

CConfigMgr::~CConfigMgr()
{
	if(NULL != m_pConfigReader)
	{
		delete m_pConfigReader;
		m_pConfigReader = NULL;
	}
}

void CConfigMgr::init(const char * pConfigFile)
{
	m_pConfigReader = new CConfigFileReader(pConfigFile);
}

