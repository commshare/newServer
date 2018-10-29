/**
 *  file base_client.cpp
 *  
 */

#include "base_client.h"
#include "apush_server_manager.h"


void CBaseClient::OnNotifyCallBack(void *usrData, shared_ptr<APushData> shared_APushData)
{
	NOTUSED_ARG(usrData);
	CApushLocalManager::GetInstance()->GetServer()->OnNotify(shared_APushData);
}
