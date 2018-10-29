/**
 * apush_server_manager.cpp
 *  
 * app manager 
 * hold the apushclient and apushserver 
 *  
 * add by liulang 
 *  
 * 2017-07-01 
 */


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




