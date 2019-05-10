#include "apush_server_manager.h"

CApushLocalManager *CApushLocalManager::GetInstance()
{
	static CApushLocalManager AppManager;
	return &AppManager;
}

CApushLocalSvr *CApushLocalManager::GetServer()
{
	return m_apushLocalSvr;
}




