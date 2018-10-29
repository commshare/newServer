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

#include <linux/sockios.h> //ioctl

#include "ssl_socket.h"
#include "ssl_event.h"
#include "lock.h"

typedef hash_map<net_handle_t, CSslSocket*> SocketMap;
SocketMap	g_ssl_socket_map;
//extern CLock gSockLock;



CLock lockSocket_map;
void AddSocket(CSslSocket* pSocket)
{
	CAutoLock lock(&lockSocket_map);
	g_ssl_socket_map.insert(make_pair((net_handle_t)pSocket->GetSocket(), pSocket));
}

void RemoveSocket(CSslSocket* pSocket)
{
	CAutoLock lock(&lockSocket_map);
	g_ssl_socket_map.erase((net_handle_t)pSocket->GetSocket());
}

CSslSocket* FindSocket(net_handle_t fd)
{
	CAutoLock lock(&lockSocket_map);
	CSslSocket* pSocket = NULL;
	SocketMap::iterator iter = g_ssl_socket_map.find(fd);
	if (iter != g_ssl_socket_map.end())
	{
		pSocket = iter->second;
		pSocket->AddRef();
	}

	return pSocket;
}

CSslSocket::~CSslSocket()
{

	Close();

	if (m_ssl)
	{
		SSL_free(m_ssl);
		m_ssl = NULL;
	}

	if (m_sslCtx)
	{
		SSL_CTX_free(m_sslCtx);
		m_sslCtx = nullptr;
	}

}	

CSslSocket::CSslSocket()
{
	m_socket = -1;
	m_sslCtx = nullptr;
	m_ssl = nullptr;
	m_bInit = false;

	m_socket = INVALID_SOCKET;
	m_state = SOCKET_STATE_IDLE;

	m_pSslEventDispatch = nullptr;

	memset(&m_casIpv4, 0, sizeof(sockaddr_in));
}

bool CSslSocket::Init()
{
	if (m_bInit)
	{
		return true;
	}
	const SSL_METHOD *sslMethod = SSLv23_client_method();

     if(nullptr == sslMethod)
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

void CSslSocket::SetSendBufSize(uint32_t send_size)
{
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &send_size, 4);
	if (ret == SOCKET_ERROR) {
		log("set SO_SNDBUF failed for fd=%d", m_socket);
	}

	socklen_t len = 4;
	int size = 0;
	getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &size, &len);
	log("socket=%d send_buf_size=%d", m_socket, size);
}

void CSslSocket::SetRecvBufSize(uint32_t recv_size)
{
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &recv_size, 4);
	if (ret == SOCKET_ERROR) {
		log("set SO_RCVBUF failed for fd=%d", m_socket);
	}

	socklen_t len = 4;
	int size = 0;
	getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &size, &len);
	log("socket=%d recv_buf_size=%d", m_socket, size);
}


int CSslSocket::_GetErrorCode()
{
	return errno;
}

bool CSslSocket::_IsBlock(int error_code)
{
	return ( (error_code == EINPROGRESS) || (error_code == EWOULDBLOCK) );
}

int CSslSocket::_SslShakeHands()
{

	if (m_socket <= 0)
	{
		ErrLog("_SslShakeHands m_socket <= 0!");
		return -1;
	}

	if (!m_ssl)
	{
		ErrLog("_SslShakeHands m_ssl == 0!");
		return -1;
	}
	SSL_set_fd(m_ssl, m_socket);

	SSL_set_connect_state(m_ssl);


	/*
	if(1 != SSL_set_cipher_list(m_ssl, "TLSv1.2"))
	{
		ErrLog("SSL_set_cipher_list");
	}
	*/
	//ssl version 1.1.*
	//SSL_set_mode(m_ssl, SSL_MODE_ASYNC);

	int nRet = SSL_connect(m_ssl);
	if (1 != nRet)
	{
			int iRet = SSL_get_error(m_ssl, nRet);
			ErrLog("socket:%d ssl err:%d, errno %d: %s", m_socket, iRet, errno, strerror(errno));
			return -1;
	}

	InfoLog("TLS connect success!");

	//struct cert_st *ce = m_ssl->cert;
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
			return -1;
	}
	X509_free(serverCertification);
	const char * cipher = SSL_get_cipher(m_ssl);
	InfoLog("cipher:%s\n", cipher);
	
	//todo certify

	return m_socket;
}

int CSslSocket::Send(void* buf, int len)
{
	int n = 0;
	int iSendLen = 0;
	m_LineWriteLock.lock();
	if (m_state != SOCKET_STATE_CONNECTED)
	{
		ErrLog("socekt state is: %d\n", m_state);
		goto end;
	}

	if (!m_bWriteable)
	{
		WarnLog("m_bWriteable is false");
		goto end;
	}

	/**
	 * 1.本来是不应该加这个控制条件 
	 * 2.不管是多线程(概率较高),还是单线程(高概率)调用ssl_write会出现ssl错误的情况 
	 * 3.这个条件判断,可有效的控制ssl这个错误出现的次数(低概率ssl错误) 
	 * 4.暂时这样处理
	 */
	//test
	if (1)
	{
		socklen_t optlen = 4;
		int value = 0;
		//int size = 0;
		static int size = 0;
		int iRet = 0;

		if (size == 0)
		{
			int iRet = getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (void *)&size, &optlen);
			if (iRet != 0)
			{
				DbgLog("getsockopt errno :%d, %s", errno, strerror(errno));
				goto end;
			}

		}

		iRet = ioctl(m_socket, SIOCOUTQ, &value);
		if (iRet != 0)
		{
			DbgLog("ioctl error\n");
			goto end;
		}
		
		DbgLog("totle:%d, value:%d\n", size, value);
		if (value + len > size)
		{
			goto end;
		}

	}

	//报文
	while (true)
	{
		InfoLog("send:%s", (char *)buf);
		n = SSL_write(m_ssl, (char*)buf + iSendLen, len - iSendLen);
		int iRet = SSL_get_error(m_ssl, n);
		DbgLog("%d ssl err:%d, errno %d: %s", n, iRet, errno, strerror(errno));

		if (iRet == SSL_ERROR_NONE)
		{

			if (n > 0)
			{
				iSendLen += n;
			}

			if (iSendLen >= len)
			{
				break;
			}

			InfoLog("socket %d send data %d",GetSocket(), n);
		}
		else if (iRet == SSL_ERROR_WANT_READ || iRet == SSL_ERROR_WANT_WRITE)
		{
			n = 0;
			InfoLog("socket send block fd=%d", m_socket);
			m_bWriteable = false;
			break;
		}
		else
		{
			m_bWriteable = false;
			m_LineWriteLock.unlock();
			ErrLog("socket:%d send failed, error code: %d, close!", m_socket, errno);

			Close();
			m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
			n =  -1;

			m_LineWriteLock.lock();

			break;
		}
	}
end:
	m_LineWriteLock.unlock();
	return n;
}

int CSslSocket::Recv(void* buf, int len)
{

	if (m_state != SOCKET_STATE_CONNECTED)
	{
		ErrLog("socekt not SOCKET_STATE_CONNECTED");
		return NETLIB_ERROR;
	}

	 //报文
	int n = SSL_read(m_ssl, (void*)buf, len);
	if (n <= 0)
	{
		int iRet = SSL_get_error(m_ssl, n);
		//DbgLog("%d ssl err:%d, errno %d: %s", n, iRet, errno, strerror(errno));

		if (iRet == SSL_ERROR_WANT_READ || iRet == SSL_ERROR_WANT_WRITE)
		{
			//n = 0;
			InfoLog("socket recv block fd=%d", m_socket);
		}
		else
		{
			ErrLog("socket:%d recv failed, error code: %d, ret:%d",m_socket, errno, n);

			/*
			if (errno == 11)
			{
				Close();
				m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
				n =  -1;
			}
			*/
		}
	}

	return n;
}

int CSslSocket::Close()
{

	m_state = SOCKET_STATE_CLOSING;

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

	}

	if (m_socket > 0)
	{

		InfoLog("socket %d close", GetSocket());
		CSslEventDispatch::Instance()->RemoveEvent(this, 0);

		RemoveSocket(this);

		shutdown(m_socket, SHUT_WR);
		close(m_socket);

		m_socket = -1;
	}
	else
	{
		InfoLog("m_socket <= 0");
	}
	//ReleaseRef();
	return 0;
}

void CSslSocket::OnRead()
{
	//gSockLock.lock();
	if (m_state == SOCKET_STATE_LISTENING)
	{
		//todo
		InfoLog("don`t to listen");
		//_AcceptNewSocket();
	}
	else if (m_state == SOCKET_STATE_CLOSING)
	{
		InfoLog("onread closed\n");
		return;
	}
	else
	{
		u_long avail = 0;
		if ( (ioctlsocket(m_socket, FIONREAD, &avail) == SOCKET_ERROR) || (avail == 0) )
		{
			m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
		}
		else
		{
			m_callback(m_callback_data, NETLIB_MSG_READ, (net_handle_t)m_socket, NULL);
		}
	}

}

//1. if the connect() is finished , the socket stat is being wirteable
//so the epoll will call this function
//2. some data send to sever and be ack,(io`s stat is changged) will call this function 
//getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF
//ioctl(m_socket, SIOCOUTQ, &value);
//3. error occured???
void CSslSocket::OnWrite()
{
	//gSockLock.lock();
	if (m_state == SOCKET_STATE_CLOSING)
	{
		InfoLog("onread closed\n");
		return;
	}

	//CSslEventDispatch::Instance()->DelEpollOut(this);

	DbgLog("OnWrite");
	m_LineWriteLock.lock();
	m_bWriteable = true;
	m_LineWriteLock.unlock();

	m_callback(m_callback_data, NETLIB_MSG_WRITE, (net_handle_t)m_socket, NULL);

	
	//gSockLock.unlock();
}

void CSslSocket::OnClose()
{
	//state = SOCKET_STATE_CLOSING;
	Close();
	m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
}


int CSslSocket::ReConnect()
{

	//printf("ReConnect\n");
	int iRet = -1;
	if (!m_callback || !m_callback_data || m_strHost.empty() || m_remote_port <= 0)
	{
		ErrLog("!m_callback || !m_callback_data || m_strHost.empty() || m_remote_port <= 0");
		return  -1;
	}
	
	if (m_state == SOCKET_STATE_CLOSING)
	{
		iRet =  SslConnectWebSite(m_callback, m_callback_data, m_strHost.c_str(), m_remote_port);

		if (iRet < 0)
		{
			Close();
		}
	}

	return iRet;
}

int CSslSocket::SslConnectWebSite(callback_t callback, void* callback_data, const char* sHost, const int16_t iPort)
{

	if (!callback)
	{
		ErrLog("callback is nullptr!");

		return -1;
	}
	
	m_remote_port = iPort;
	m_callback = callback;
	m_callback_data = callback_data;
	m_strHost = sHost;

	if(!m_bInit)
	{
		ErrLog("SslConnectWebSite not init");
		Init();
		//return -1;
	}

	//string sIp;
	if (!ResolveHostName(sHost, iPort))
	{
		ErrLog("SslConnectWebSite ");

		return -1;
	}

	if (m_remote_ip.empty())
	{
		ErrLog("SslConnectWebSite m_remote_ip");
		return -1;
	}
	
	InfoLog("ip:%s, port:%u", m_remote_ip.c_str(), iPort);


	if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		ErrLog("create socket error!");
		Close();
		return -1;
	}
	
	if(-1 == SetBlockOrNot(m_socket, 1))
	{
		ErrLog("setnonblocking socket :%d error!", m_socket);
		Close();
		return -1;
	}
	
	if (connect(m_socket,(struct sockaddr *)&m_casIpv4, sizeof(m_casIpv4)) == -1) 
	{
		if (errno == EHOSTUNREACH) 
		{
			ErrLog("EHOSTUNREACH socket :%d error!", m_socket);
			Close();
			return -1;
		} 
		else if (errno == EINPROGRESS) 
		{
			//ok
		} 
		else 
		{
			if (WaitReady(m_socket, 2000) != true)
			{
				ErrLog("WaitReady socket :%d error!", m_socket);
				Close();
				return -1;
			}
		}				
	}

	struct sockaddr_in locateSock;
	int nRet = sizeof(locateSock);
	if(getsockname(m_socket, (struct sockaddr *)&locateSock, (socklen_t *)&nRet) == -1)
	{
		ErrLog("%d setsockopt TCP_NODELAY errno:%s!", m_socket, strerror(errno));
		Close();
		return -1;
	}
	else
	{
		m_local_port = ntohs(locateSock.sin_port);
		m_local_addr = inet_ntoa(locateSock.sin_addr);

		InfoLog("tcp connect success, local port:%d, addr:%s", 
				m_local_port, m_local_addr.c_str());
	}	

	if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &nRet, sizeof(nRet)) == -1) {
		ErrLog("%d setsockopt TCP_NODELAY  error!", m_socket);
		Close();
		return -1;
	}

	if(-1 == SetBlockOrNot(m_socket, 0))
	{
		ErrLog("set blocking socket :%d error!", m_socket);
		Close();
		return -1;
	}

	if(_SslShakeHands() < 0)
	{
		ErrLog("%d  _SslShakeHands  error!", m_socket);

		m_state = SOCKET_STATE_SSL_ERR;

		//attion: need clear the ssl resource!!!
		//close(m_socket);
		Close();

		return -1;
	}

	/*
	if(-1 == SetBlockOrNot(m_socket, 1))
	{
		ErrLog("set blocking socket :%d error!", m_socket);
		Close();
		return -1;
	}
	*/

	m_remote_port = iPort;
	m_callback = callback;
	m_callback_data = callback_data;

	m_state = SOCKET_STATE_CONNECTED;

	AddSocket(this);

	m_bWriteable = true;

	return m_socket;
}

bool CSslSocket::ResolveHostName(const string s_hostname, const int16_t iPort)
{
	if (s_hostname.empty() || iPort <= 0)
	{
		ErrLog("ResolveHostName host or port not set");
		return false;
	}

	//域名和ip都没变
	if (m_strHost == s_hostname && iPort == m_remote_port)
	{
		//网络地址已经获取过
		if (!m_remote_ip.empty())
		{
			return true;
		}
	}

	m_strHost = s_hostname;
	m_remote_port = iPort;

	////////////////////////////////////////////////
	//重新通过域名和端口获取ip和网络地址
	m_remote_ip.clear();
	memset(&m_casIpv4, 0, sizeof(sockaddr_in));

	char service[NI_MAXSERV];
	snprintf(service, sizeof(service), "%u", iPort);

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *res = NULL;

	int rv = getaddrinfo(s_hostname.c_str(), service, &hints, &res);
	if (rv != 0)
	{
		ErrLog("getaddrinfo", gai_strerror(rv));
		return false;
	}

	if (res)
	{
		memcpy(&m_casIpv4, (struct sockaddr_in *) res->ai_addr, sizeof(sockaddr_in));
	}
	m_remote_ip = inet_ntoa(m_casIpv4.sin_addr);

	return true;
}

bool CSslSocket::WaitReady(int sockFd, int32_t msec)
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
			ErrLog("poll error!");
			return false;
		} 
		else if(res == 0)
		{
			ErrLog("poll timeOut:%ld", msec);
			return false;
		}

		if(getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) 
		{
			ErrLog("getsockopt error!");
			return false;
		}

		if(err) 
		{
			ErrLog("getsockopt errno %d: %s !", errno, strerror(errno));
			return false;
		}
	}

	return true;
}


