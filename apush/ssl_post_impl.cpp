/**
 * file ssl_post_impl.h 
 *  
 * lang 20170906 
 */

#include "ssl_post_impl.h"

//epoll callback
void CPost::ResponeCallBack(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	NOTUSED_ARG(pParam);
	if (!callback_data)
	{
		ErrLog("callback_data is null");
		return;
	}
	
	CPost *post = (CPost *) callback_data;
	
	if (msg == NETLIB_MSG_READ)
	{
		post->SetHeartbeat();
		post->OnRead();
	}
	else if (msg == NETLIB_MSG_CLOSE)
	{
		post->OnClose();
	}
	else
	{
		InfoLog("%d, other msg type:%d\n", handle, msg);
	}
}

//static time_t tt = 0;
bool CPost::InitConnection(string host, uint16_t port, OnNotifyCallBack_t fun, void* callback_data, CSslEventDispatch *pevent)
{
	NOTUSED_ARG(pevent);
	//debug
	m_Callback = fun;
	m_Callback_data = callback_data;

	m_pSslSock  = new CSslSocket;
	if (!m_pSslSock)
	{
		ErrLog("new CSslSocket");
		return false;
	}
	
	if(!m_pSslSock->Init())
	{
		ErrLog("sslsocket->Init");
		return false;
	}

	if (m_pSslSock->SslConnectWebSite((callback_t)&CPost::ResponeCallBack, this, host.c_str(), port) <= 0)
	{
		ErrLog("SslConnectWebSite");
		m_pSslSock->Close();
		return false;
	}

	if (-1 == SetBlockOrNot(m_pSslSock->GetSocket(), 1))
	{
		ErrLog("set blocking socket  error!");
		return -1;
	}

	CSslEventDispatch::Instance()->AddEvent(m_pSslSock, 5*60*1000);

	m_uPostIndex = 0;
	SetHeartbeat();
	SetUsrable(true);

	return true;
}

int CPost::ReConnectPost()
{
	SetUsrable(false);

	m_pSslSock->Close();
	int iRet = m_pSslSock->ReConnect();
	if(0 > iRet)
	{
		ErrLog("ReConnectPost");
		return -1;
	}

	if (-1 == SetBlockOrNot(m_pSslSock->GetSocket(), 1))
	{
		ErrLog("set blocking socket  error!");
		return -1;
	}

	CSslEventDispatch::Instance()->AddEvent(m_pSslSock, 5*60*1000);

	m_uPostIndex = 0;
	SetHeartbeat();

	if (m_ptrCacheData)
	{
		if (Request(m_ptrCacheData) <= 0)
		{
			ErrLog("%s, reSend error, not send!", m_ptrCacheData.get()->msgId.c_str());
		}
	}
	else
	{
		SetUsrable(true);
	}

	return iRet;
}

int32_t CPost::Request(shared_ptr<APushData> ptrData)
{

	if (!ptrData)
	{
		ErrLog("Request(shared_ptr<APushData> ptrData)");
		return -1;
	}

	m_ptrCacheData = ptrData;
	
	int iRet = m_pSslSock->Send((void *)ptrData->sendBuf.c_str(), ptrData->sendBuf.size());
	if (iRet <= 0)
	{
		ErrLog("m_pSslSock->Send");

		//m_ptrCacheData.reset();
		return -1;
	}

	SetUsrable(false);
	SetHeartbeat();

	return iRet;
}


static int hwPostNotify = 0;
void CPost::OnRead()
{

	char buf[1024];
	buf[0] = '\0';

	if (m_pSslSock->Recv(buf, 1024) <= 0)
	{
		InfoLog("OnRead recv return 0");
		return;
	}
	
	if (!m_ptrCacheData)
	{
		ErrLog("m_ptrCacheData null");
		InfoLog("PostIndex:%d, SslCallback Recv:%s!",m_uPostIndex, buf);

		return;
	}

	APushData *data = m_ptrCacheData.get();
	if (!data)
	{
		ErrLog("m_ptrCacheData.get() null");
		InfoLog("PostIndex:%d, SslCallback Recv:%s!",m_uPostIndex, buf);

		m_ptrCacheData.reset();
		return;
	}


	//InfoLog("PostIndex:%d, SslCallback Recv:%s!",m_uPostIndex, buf);
	string strAck = buf;
	if ((int)strAck.find(success_respone) > 0)
	{
		data->retStatus = NON_ERR;
		InfoLog("msgid:%s send success!", data->msgId.c_str());
	}
	else
	{
		data->retStatus = EXCEPT_ERR;
		InfoLog("PostIndex:%d, SslCallback Recv:%s!",m_uPostIndex, buf);
	}
	
	DbgLog("CPost hwPostNotify:%d", ++hwPostNotify);
	m_Callback(m_Callback_data, m_ptrCacheData);

	//hw one connect most send post is 100, so
	//begin an new connection
	if (m_uPostIndex == 97)
	{
		InfoLog("Close");

		m_uPostIndex = 0;
		m_pSslSock->Close();
	}
	else
	{
		SetUsrable(true);
	}

	m_ptrCacheData.reset();
}

void CPost::OnWrite()
{
	//do nothing
}

void CPost::OnClose()
{
	InfoLog("close by peer");

	//close by peer
	SetUsrable(false);
	//m_ptrCacheData.reset();
}


