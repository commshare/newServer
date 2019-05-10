#include <poll.h>
#include "hw_ssl_get_token.h"
#include "configfilereader.h"
CSocket::CSocket()
{
	m_iFd = -1;
	m_strHost.clear();
	m_iPort = -1;
}	

CSocket::~CSocket()
{
	m_strHost.clear();
}


CSocket::CSocket(const CSocket& sock)
{
	if (&sock != this)
	{
		this->m_iFd = sock.m_iFd;
		this->m_strHost = sock.m_strHost;
		this->m_iPort = sock.m_iPort;
	}
}

CSocket &CSocket::operator=(const CSocket& sock)
{
	if (&sock != this)
	{
		this->m_iFd = sock.m_iFd;
		this->m_strHost = sock.m_strHost;
		this->m_iPort = sock.m_iPort;
	}

	return *this;
}


bool CSocket::SetHostPort(const char* sHost, const int16_t iPort)
{
	if (!sHost || iPort < 0)
	{
		return false;
	}

	m_strHost = sHost;
	m_iPort = iPort;
	
	return true;
}

int32_t CSocket::GetNewSocket()
{
	if((m_iFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		ErrLog("create socket error!");
		close(m_iFd);
		return -1;
	}

	return m_iFd;
}


bool CSocket::WaitReady( int sockFd, int32_t msec )
{
	struct pollfd   wfd[1];

	wfd[0].fd     = sockFd;
	wfd[0].events = POLLOUT;

	if (errno == EINPROGRESS)
	{
		int res;
		int err;
		socklen_t errlen = sizeof(err);

		if((res = poll(wfd, 1, msec)) == -1) 
		{
			ErrLog("CasAuthentication poll error!");
			return false;
		} 
		else if(res == 0)
		{
			ErrLog("CasAuthentication poll timeOut:%ld", msec);
			return false;
		}

		if(getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) 
		{
			ErrLog("CasAuthentication getsockopt error!");
			return false;
		}

		if(err) 
		{
			ErrLog("CasAuthentication getsockopt errno %d: %s !", errno, strerror(errno));
			return false;
		}
	}

	return true;
}


int CSocket::OnConnect(int32_t msec)
{

	if(!ResolveHostName())
	{
		ErrLog("ResolveHostName error!");
		return -1;
	}

	if ((m_iFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		ErrLog("create socket error!");
		close(m_iFd); 

		return -1;
	}
	
	if(-1 == SetBlockOrNot(m_iFd, 1))
	{
		ErrLog("setnonblocking socket :%d error!", m_iFd);
		return -1;
	}
	
	if (connect(m_iFd,(struct sockaddr *)&m_casIpv4, sizeof(m_casIpv4)) == -1) 
	{
		if (errno == EHOSTUNREACH) 
		{
			ErrLog("EHOSTUNREACH socket :%d error!", m_iFd);
			close(m_iFd);
			return -1;
		} 
		else if (errno == EINPROGRESS) 
		{
			//ok
		} 
		else 
		{
			if (WaitReady(m_iFd, msec) != true)
			{
				ErrLog("U16_CONNTIMEOUT socket :%d error!", m_iFd);
				close(m_iFd);
				return -1;
			}
		}				
	}

	if(-1 == SetBlockOrNot(m_iFd, 0))
	{
		ErrLog("set blocking socket :%d error!", m_iFd);
		close(m_iFd);
		return -1;
	}

	int nRet = 1;
	if (setsockopt(m_iFd, IPPROTO_TCP, TCP_NODELAY, &nRet, sizeof(nRet)) == -1) {
		ErrLog("%d setsockopt TCP_NODELAY  error!", m_iFd);
		close(m_iFd);
		return -1;
	}

	return m_iFd;
}

bool CSocket::ResolveHostName()
{
		memset(&m_casIpv4, 0, sizeof(sockaddr_in));

		char service[100];
		snprintf(service, sizeof(service), "%u", m_iPort);

		struct addrinfo hints;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		struct addrinfo *res = NULL;

		int rv = getaddrinfo(m_strHost.c_str(), service, &hints, &res);
		if (rv != 0)
		{
			ErrLog("getaddrinfo", gai_strerror(rv));
			return false;
		}

		if (res)
		{
			memcpy(&m_casIpv4, (struct sockaddr_in *) res->ai_addr, sizeof(sockaddr_in));
		}

		return true;
}

CSslClient::CSslClient()
{

	m_iSocket = -1;
	m_sslCtx = nullptr;
	m_ssl = nullptr;
	m_bInit = false;
}

CSslClient::~CSslClient()
{
	if (m_ssl)
	{
		int32_t nRet = -1;
		SSL_set_shutdown(m_ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);

		nRet = SSL_shutdown(m_ssl);
		if (nRet == 0)
		{
			int32_t nErrorCode = SSL_get_error(m_ssl, nRet);
			ErrLog("ssl shutdown not finished, errno: %d.\n", nErrorCode);
		}
		else if (nRet == 1)
		{
			InfoLog("ssl shutdown successed.");
		}
		else if (nRet < 0)
		{
			int32_t nErrorCode = SSL_get_error(m_ssl, nRet);
			ErrLog("ssl shutdown failed, errno: %d.", nErrorCode);
		}

		nRet = SSL_clear(m_ssl);
		if (nRet != 1)
		{
			int32_t nErrorCode = SSL_get_error(m_ssl, nRet);
			ErrLog("ssl shutdown not finished, errno: %d.\n", nErrorCode);
		}

		SSL_free(m_ssl);
		m_ssl = NULL;
	}

	if (m_sslCtx)
	{
		SSL_CTX_free(m_sslCtx);
		m_sslCtx = NULL;
	}

	if (m_iSocket > 0)
	{
		close(m_iSocket);
	}

}

bool CSslClient::Init()
{
	
	const SSL_METHOD *sslMethod = SSLv23_client_method();

     if(NULL == sslMethod)
     {
		 ErrLog("SSLv23_client_method");
		 return false;
     }

     m_sslCtx = SSL_CTX_new(sslMethod);
     if (NULL == m_sslCtx )
     {
		 ErrLog("SSL_CTX_new");
		 return false;
     }

     m_ssl = SSL_new(m_sslCtx);
     if (NULL == m_ssl)
     {
		 ErrLog("SSL_CTX_new");
		 return false;
     }
	 
	 m_bInit = true;
	return true;
}


int CSslClient::SslConnectTimeout(const char* sHost, const int16_t iPort, int32_t msec)
{

	if (!m_bInit)
	{
		Init();
	}
	
	CSocket sock;
	if (!sock.SetHostPort(sHost, iPort))
	{
		InfoLog("SslConnectTimeout");
	}

	m_iSocket = sock.OnConnect(msec);

	if (m_iSocket < 0)
	{
		 InfoLog("GetNewSocket");
	}

	SSL_set_fd(m_ssl, m_iSocket);

	int nRet = SSL_connect(m_ssl);
	if (1 != nRet)
	{
			ErrLog("connect error!");

			int iRet = SSL_get_error(m_ssl, nRet);
			ErrLog("ssl err:%d, errno %d: %s", iRet, errno, strerror(errno));
			return -1;
	}

	InfoLog("TLS connect success!");

	X509 * serverCertification = SSL_get_peer_certificate(m_ssl); //从SSL套接字中获取对方的证书信息
	if (NULL != serverCertification)
	{

			char *cstrSslSubject = X509_NAME_oneline(X509_get_subject_name(serverCertification), 0, 0);
			char *cstrSslIssuer = X509_NAME_oneline(X509_get_issuer_name(serverCertification), 0, 0);
			InfoLog("cstrSslSubject:%s\n cstrSslIssuer:%s",cstrSslSubject, cstrSslIssuer);
	}
	else
	{
			InfoLog("SSL_get_certificate error!");
	}
	X509_free(serverCertification);
	const char * cipher = SSL_get_cipher(m_ssl);
	InfoLog("cipher:%s\n", cipher);

	return m_iSocket;
}


int CSslClient::SslSendTimeout(uint16_t mesc, const char *buf, int32_t bufSize)
{
		struct timeval tv;
		
		tv.tv_sec = mesc/1000;
		tv.tv_usec = mesc&0x3ff;

		int n = -1;

		if (!buf || bufSize <=0)
		{
			ErrLog("SslSendTimeout param error");
			return false;
		}
		//超时属性
		if(setsockopt(m_iSocket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1)
		{
				ErrLog("setsockopt SO_SNDTIMEO failue, errno %d: %s", errno, strerror(errno));
				return false;
		}

		//报文
		n = SSL_write(m_ssl, (void*)buf, bufSize);
		if (n <= 0)
		{
			int iRet = SSL_get_error(m_ssl, n);
			ErrLog("ssl err:%d, errno %d: %s", iRet, errno, strerror(errno));
			return false;
		}

		if (n != bufSize)
		{
				ErrLog("sendto len error! n%d, packet len:%d", n, bufSize);
				return false;
		}
		
		return true;
}

int CSslClient::SslRecvTimeOut(uint16_t mesc, char *bufDesc, int32_t bufSize)
{
	struct timeval tv;

	tv.tv_sec = mesc/1000;
	tv.tv_usec = mesc&0x3ff;

	int n = -1;

	if (!bufDesc || bufSize <=0)
	{
		ErrLog("SslRecvTimeOut param error");
		return n;
	}
	 //超时属性
	if(setsockopt(m_iSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
	{
			ErrLog("setsockopt SO_RCVTIMEO failue, errno %d: %s", errno, strerror(errno));
			return n;
	}

	 //报文
	n = SSL_read(m_ssl, (void*)bufDesc, bufSize);
	if (n <= 0)
	{
		int iRet = SSL_get_error(m_ssl, n);
		ErrLog("SslRecvTimeOut ssl err:%d, errno %d: %s", iRet, errno, strerror(errno));

		//exit(-1);
		return n;
	}

	if (n > bufSize)
	{
			ErrLog("sendto len error! n%d, packet len:%d", n, bufSize);
			return n;
	}

	return n;
}


//uint32_t CHwGetPushTokenClient::uTimeTick = 0;
//uint32_t CHwGetPushTokenClient::m_uExpires = 0;

CHwGetPushTokenClient::CHwGetPushTokenClient()
{
	m_uExpires = 0;
	m_ssllink = new CSslClient;
}

CHwGetPushTokenClient::~CHwGetPushTokenClient()
{

	if (m_ssllink)
	{
		delete m_ssllink;
		m_ssllink = nullptr;
	}
}

bool CHwGetPushTokenClient::Init(CConfigFileReader* pConfigReader)
{

	if (!pConfigReader)
	{
		ErrLog("HwClientId pConfigReader is nullptr");
		return false;
	}
	m_strSecret = pConfigReader->GetConfigName("HwSecret");
	m_strClientId = pConfigReader->GetConfigName("HwClientId");

	if (m_strClientId.empty())
	{
		InfoLog("HwClientId is not set use default: 100042631");
		m_strClientId =  Default_HW_clientId;
	}

	if (m_strSecret.empty())
	{
		InfoLog("m_strSecret is not set use default: 9ace04cb63f631cfc24f5a7411560332");
		m_strSecret = Default_HW_secret;
	}

	if (!m_ssllink)
	{
		ErrLog("new CSslClient");
		return false;
	}

	return m_ssllink->Init();
}

string CHwGetPushTokenClient::Post_ToGetToken()
{
	if (!m_ssllink)
	{
		ErrLog("m_ssllink is null");
		return "";
	}
	int isocket = m_ssllink->SslConnectTimeout(STR_HW_HOST_ADDR.c_str(), HW_HOST_PORT, U16_CONNTIMEOUT);
	//int isocket = m_ssllink->SslConnectTimeout(STR_HW_HOST_ADDR_V2.c_str(), HW_HOST_PORT, U16_CONNTIMEOUT);
	if (isocket <= 0)
	{
		ErrLog("SslConnectTimeout");
		return "";
	}

	char sendContent[HW_POST_CONTENT_SIZE] = {0};
	if (STR_HW_POST_CONTENT.size() + m_strSecret.size() + m_strClientId.size() > HW_POST_CONTENT_SIZE + 3)
	{
		ErrLog("SslConnectTimeout HW_POST_CONTENT_SIZE");
		return "";
	}

	sprintf(sendContent, STR_HW_POST_CONTENT.c_str(),
			m_strSecret.c_str(), m_strClientId.c_str());

	string sendBuf = sendContent;
	memset(sendContent, 0, HW_POST_CONTENT_SIZE);

	sprintf(sendContent, STR_HW_POST_HEAD.c_str(), (int)sendBuf.size());
	//sprintf(sendContent, STR_HW_POST_HEAD_V2.c_str(), (int)sendBuf.size());

	sendBuf = string(sendContent) + sendBuf;
    InfoLog("post request add =%s,  data = %s", STR_HW_HOST_ADDR.c_str(), sendBuf.c_str());
    //InfoLog("post request add =%s,  data = %s", STR_HW_HOST_ADDR_V2.c_str(), sendBuf.c_str());

	//printf("%s\n", sendBuf.c_str());
	int iRet = m_ssllink->SslSendTimeout(U16_SENDTIMEOUT, sendBuf.c_str(), sendBuf.size());

	if (iRet != 1)
	{
		ErrLog("send:%d, SslSendTimeout:%s, %d", iRet, sendBuf.c_str(), sendBuf.size());
		return "";
	}

	char recv[HW_POST_RECV_SIZE] = {0};

	iRet = m_ssllink->SslRecvTimeOut(U16_RECVTIMEOUT,(char*)recv, HW_POST_RECV_SIZE);
	if (iRet <= 0 )
	{
		ErrLog("SslRecvTimeOut");
		return "";
	}
	
	m_ssllink->SslCloseSocket();	

    InfoLog("post get_hw_token respone = %s", recv);
	PhaseRecvData(recv);

	//printf("%s\n", recv);
	return m_strToken;
}

string CHwGetPushTokenClient::Post_ToGetToken(uint32_t& expirtime, uint32_t& ticktime)
{
	if (!m_ssllink)
	{
		ErrLog("m_ssllink is null");
		return "";
	}
	int isocket = m_ssllink->SslConnectTimeout(STR_HW_HOST_ADDR.c_str(), HW_HOST_PORT, U16_CONNTIMEOUT);
	//int isocket = m_ssllink->SslConnectTimeout(STR_HW_HOST_ADDR_V2.c_str(), HW_HOST_PORT, U16_CONNTIMEOUT);
	if (isocket <= 0)
	{
		ErrLog("SslConnectTimeout");
		return "";
	}

	char sendContent[HW_POST_CONTENT_SIZE] = {0};
	if (STR_HW_POST_CONTENT.size() + m_strSecret.size() + m_strClientId.size() > HW_POST_CONTENT_SIZE + 3)
	{
		ErrLog("SslConnectTimeout HW_POST_CONTENT_SIZE");
		return "";
	}

	sprintf(sendContent, STR_HW_POST_CONTENT.c_str(),
			m_strSecret.c_str(), m_strClientId.c_str());

	string sendBuf = sendContent;
	memset(sendContent, 0, HW_POST_CONTENT_SIZE);

	sprintf(sendContent, STR_HW_POST_HEAD.c_str(), (int)sendBuf.size());
	//sprintf(sendContent, STR_HW_POST_HEAD_V2.c_str(), (int)sendBuf.size());

	sendBuf = string(sendContent) + sendBuf;
    InfoLog("post request add =%s,  data = %s", STR_HW_HOST_ADDR.c_str(), sendBuf.c_str());
    //InfoLog("post request add =%s,  data = %s", STR_HW_HOST_ADDR_V2.c_str(), sendBuf.c_str());

	//printf("%s\n", sendBuf.c_str());
	int iRet = m_ssllink->SslSendTimeout(U16_SENDTIMEOUT, sendBuf.c_str(), sendBuf.size());

	if (iRet != 1)
	{
		ErrLog("send:%d, SslSendTimeout:%s, %d", iRet, sendBuf.c_str(), sendBuf.size());
		return "";
	}

	char recv[HW_POST_RECV_SIZE] = {0};

	iRet = m_ssllink->SslRecvTimeOut(U16_RECVTIMEOUT,(char*)recv, HW_POST_RECV_SIZE);
	if (iRet <= 0 )
	{
		ErrLog("SslRecvTimeOut");
		return "";
	}
	
	m_ssllink->SslCloseSocket();	

    InfoLog("post get_hw_token respone = %s", recv);
	PhaseRecvData(recv);
    expirtime = m_uExpires; 
    ticktime = uTimeTick;
	//printf("%s\n", recv);
	return m_strToken;
}

//{"access_token":"CFlusjjbIqNC97d69wd9fRsIC8WDy1mX7BI8v0K0j0BBBXFAiGFZ3ajXd+XCHdfCFb9ZGZM+oI\/m4BGmcwP7CQ==","expires_in":604800}
//const string token = "{\"access_token\":\"";
//const string expires = "\",\"expires_in\":";
//const string strEnd = "}";
void CHwGetPushTokenClient::PhaseRecvData(string recvBuf)
{
	if (recvBuf.empty())
	{
		InfoLog("PhaseRecvData recvBuf is empty");
	}
	
	int posToken = recvBuf.find(token);
	int posexpires = recvBuf.find(expires, posToken + token.size());
	int posStrEnd = recvBuf.find(strEnd, posexpires + expires.size());

	posToken += token.size();
	if (posexpires < posToken)
	{
		ErrLog("PhaseRecvData find error");
		return;
	}

	m_strToken = recvBuf.substr(posToken, posexpires - posToken);

	posexpires += expires.size();
	if (posStrEnd < posexpires)
	{
		ErrLog("PhaseRecvData find error");
		return;
	}
	string strTime = recvBuf.substr(posexpires, posStrEnd - posexpires);

	m_uExpires = atoi(strTime.c_str());
	if (m_uExpires <= 0)
	{
		ErrLog("PhaseRecvData m_uExpires error");
		return;
	}
	
	InfoLog("m_strToken:%s, m_uExpires:%d", m_strToken.c_str(), m_uExpires);

	uTimeTick = time(NULL);
	InfoLog("uTimeTick:%d\n", uTimeTick);
}


