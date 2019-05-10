#include "voip_push_post.h"
#include "ssl_http2.h"
#include "CApnsPostData.h"
CSocketBase* CVoipPushPost::createSocket()
{
	return new CHttp2Socket();
}

int32_t CVoipPushPost::Request(shared_ptr<CApnsPostData> pData)
{
	InfoLog("Request\n");
	//m_Sendbuf = data;
	int iRet = GetSslSock()->Send(pData);
	if (iRet <= 0)
	{
		ErrLog("m_pSocket->Send");
		return -1;
	}
	
    return iRet;
}
