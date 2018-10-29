/**
 * hw_sslpushserver.cpp 
 *  
 * create by liulang 
 * desc: 
 * class :Get hw pushServer teoken
 * class : push the message to hw push Server 
 *  
 * 2017-07-11 
 * datuhao@foxmail.com 
 */

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
#include "ssl_event.h"


#include "hw_handle_protobuf.h"
#include "hw_ssl_get_token.h"
#include "ssl_socket.h"

#include "ssl_post_mgr.h"

void CHWPushClient::OnNotifyCallBack(void *usrData, shared_ptr<APushData> shared_APushData)
{
	NOTUSED_ARG(usrData);
	CApushLocalManager::GetInstance()->GetServer()->OnNotify(shared_APushData);
}

CHWPushClient::CHWPushClient(CConfigFileReader* pConfigReader)
{
	m_pConfigReader = pConfigReader;
	m_uConnectNum = 0;
	m_uMaxConnectNum = 0;
}

CHWPushClient::~CHWPushClient()
{
	if (m_pPostPoolMgr)
	{
		delete m_pPostPoolMgr;
	}
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

	m_pPostPoolMgr = new CPostPoolMgr;
	if (!m_pPostPoolMgr)
	{
		ErrLog("new CPostPoolMgr is null");
		return false;
	}

	char* numStr = m_pConfigReader->GetConfigName("hwSessionNum");
	if (NULL == numStr)
	{
		ErrLog("hwSessionNum config item is null use defalut 100 connects");
		m_uConnectNum = 10;
	}
	else
	{
		m_uConnectNum = atoi(numStr);
		if(m_uConnectNum< 0 || m_uConnectNum > 2000)
		{
			ErrLog("hwSessionNum %d not rational, use defalut 10 connects", m_uConnectNum);
			m_uConnectNum = 10;
			//return false;
		}
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
int CHWPushClient::AddTask(shared_ptr<APushData> ptrData)
{

	DbgLog("CHWPushClient::AddTask %d", ++hwTask);
	//int iRet = -1;

	
	return m_pPostPoolMgr->Post(ptrData);
}


void CHWPushClient::Start()
{
	//uMaxConnections - uMaxConnections % m_uPerThreadConns;

	if (m_uConnectNum < 0 || m_uConnectNum > 2000)
	{
		m_uConnectNum = 200;
	}
	m_pPostPoolMgr->Init(m_uConnectNum, this->OnNotifyCallBack, this, "api.push.hicloud.com", 443);
}


