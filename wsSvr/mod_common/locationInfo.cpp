/******************************************************************************
Filename: locationInfo.cpp
Description: 
******************************************************************************/
#include "locationInfo.h"

CLocationInfo::CLocationInfo(uint32_t id, uint32_t pid,const string& path, uint32_t level, const string& name)
	:m_nId(id), m_nPid(pid),m_sPath(path),m_nLevel(level),m_sName(name)
{

}

CLocationInfo::CLocationInfo()
	:m_nId(0), m_nPid(0),m_sPath(""),m_nLevel(0),m_sName("")
{

}

CLocationInfo::~CLocationInfo()
{

}
