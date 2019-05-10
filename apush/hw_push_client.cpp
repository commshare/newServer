#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/resource.h>
#include <poll.h>
#include <memory.h>

#include <netinet/tcp.h>
#include <netdb.h>
#include <time.h>

#include "jsoncpp/json/json.h"
#include "hw_push_client.h"

#include "hw_handle_protobuf.h"
#include "hw_ssl_get_token.h"
#include "apush_server_manager.h"


void CHWPushClient::OnNotifyCallBack(void *usrData, shared_ptr<APushData> shared_APushData)
{
	NOTUSED_ARG(usrData);
	CApushLocalManager::GetInstance()->GetServer()->OnNotify(shared_APushData);
}

CHWPushClient::CHWPushClient(CConfigFileReader* pConfigReader)
{
	m_pConfigReader = pConfigReader;
}

CHWPushClient::~CHWPushClient()
{
}

bool CHWPushClient::Regist()
{
	m_pManage =  CApushLocalManager::GetInstance();
	if (!m_pManage)
	{
		ErrLog("CApushLocalSvr Initialize");
		return false;
	}

	if (!m_pManage->RegistClient(this, HW_PUSH))
	{
		ErrLog("CApushLocalSvr StartApp");
		return false;
	}

	return true;
}


bool CHWPushClient::Init()
{
	if (!m_pConfigReader)
	{
		ErrLog("m_pConfigReader is null");
		return false;
	}

	return Regist();
}


static int hwTask = 0;
//if the hw ssl_sockets are don`t have the ability to handle buf:
//1. the network is not available
//2. the hw connections total bandwidth(KB/s) is slow than the local bandwidth
//quickly send endian, slowly recv endian
//3. some error occured of the network
//
//the server will be blocking there 

void CHWPushClient::AddTask(std::shared_ptr<HTTP_REQDATA_> prtData) 
{
	DbgLog("CHWPushClient::AddTask %d", ++hwTask);
   addHttpData(prtData, [](void* p){cerr << (char*)p << endl;} );

}

void CHWPushClient::Start()
{
    httpStart();
}

