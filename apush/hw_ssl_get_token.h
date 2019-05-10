#ifndef HW_SSL_GET_TOKEN_H
#define	HW_SSL_GET_TOKEN_H
#include "util.h"
#include "hw_push_client.h"

class CSocket
{
public:
	CSocket();
	~CSocket();
	CSocket(const CSocket& sock);
	CSocket &operator=(const CSocket& sock);

	bool SetHostPort(const char* sHost, const int16_t iPort);
	int32_t GetNewSocket();

	int OnConnect(int32_t msec);

private:
	bool ResolveHostName();

	bool WaitReady(int sockFd, int32_t msec);

	int32_t  	m_iFd;
	string 		m_strHost;
	int16_t 	m_iPort;
	struct sockaddr_in m_casIpv4;
};


class CSslClient
{
public:
	CSslClient();
	~CSslClient();

	bool Init();

	int SslConnectTimeout(const char* sHost, const int16_t iPort, int32_t msec);
	int SslSendTimeout(uint16_t mesc, const char *buf, int32_t bufSize);
	int SslRecvTimeOut(uint16_t mesc, char *bufDesc, int32_t bufSize);

	void SslCloseSocket()
	{
		if (m_iSocket > 0)
		{
			close(m_iSocket);
			m_iSocket = 0;
		}
	}

private:
	bool 		m_bInit;
	int32_t 	m_iSocket;
	SSL_CTX* 	m_sslCtx;
	SSL* 		m_ssl;

};

class CHwGetPushTokenClient
{

public:
	CHwGetPushTokenClient();
	~CHwGetPushTokenClient();
	bool Init(CConfigFileReader* pConfigReader);

	string Post_ToGetToken();
    string Post_ToGetToken(uint32_t& expirtime, uint32_t& ticktime);
	uint32_t uTimeTick;
	uint32_t m_uExpires;	
private:
	void PhaseRecvData(string recvBuf);

	CSslClient *m_ssllink;

	string	 m_strSecret;
	string	 m_strClientId;

	string	 m_strToken;

};
#endif

