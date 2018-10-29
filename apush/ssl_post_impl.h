/**
 * file ssl_post_impl.h 
 *  
 * lang 20170906 
 */

#ifndef SSL_POST_IMPL_H
#define SSL_POST_IMPL_H


#include "util.h"
#include "ssl_socket.h"
#include "ssl_packet_queue.h"
#include "lock.h"
#include "protobuf_phase.h"
#include "hw_push_client.h"

/**
 * 同步，发送post请求
 * 
 * @author lang (7/21/17)
 */
class CPost
{
public:
	
	CPost()
	{
		m_pSslSock = nullptr;
		m_bUsrable = false;
		m_uPostIndex = 0;
	}

	~CPost()
	{
		if (!m_pSslSock)
		{
			delete m_pSslSock;
			m_pSslSock = nullptr;
		}
	}

	static CLock m_callbackmutex;

	static void ResponeCallBack(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);

	//close connect reconnect 三个函数应当串行执行, 暂未加锁
	bool InitConnection(string host, uint16_t port, OnNotifyCallBack_t fun, void* callback_data, CSslEventDispatch *pevent);

	int ReConnectPost();

	int32_t Request(shared_ptr<APushData> ptrData);

	void OnRead();

	void OnWrite();

	void OnClose();

	uint32_t GetHeartbeat()
	{
		CAutoLock lock(&m_heartBeatMutex);
		return m_uPreBeatHeartTime;
	}

	bool GetUsrable()
	{
		CAutoLock lock(&m_lockUsrable);
		if (!m_bUsrable)
		{
			//InfoLog("fd:%d\n unUseAble!", m_pSslSock->GetSocket());
		}
		return m_bUsrable;
	}

	void SetUsrable(bool useable)
	{
		 CAutoLock lock(&m_lockUsrable);
		 m_bUsrable = useable;
	}

	CSslSocket *GetSslSock() 
	{
		return m_pSslSock;
	}

	void SetHeartbeat()
	{
		CAutoLock lock(&m_heartBeatMutex);
		m_uPreBeatHeartTime = time(NULL);
	}

	//test
	//CSslSocket	*m_pSslSock;

private:
	shared_ptr<APushData> m_ptrCacheData;

	CLock m_lockUsrable;
	bool m_bUsrable;

	int m_uPostIndex;
	CSslSocket	*m_pSslSock;

	OnNotifyCallBack_t		m_Callback;
	void*			m_Callback_data;

	//uint32_t	m_uBeatHeartRate;
	CLock		m_heartBeatMutex;
	uint32_t	m_uPreBeatHeartTime;
	
};



#endif //

