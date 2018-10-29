/**
 * file xmpp_ssl_google_fcm_impl.h
 *  create by liulang 2017-08-24
 */
#ifndef XMPP_SSL_GOOGLE_FCM_IMPL_H
#define XMPP_SSL_GOOGLE_FCM_IMPL_H

#include "ssl_socket.h"
#include "ssl_packet_queue.h"
#include "protobuf_phase.h"
#include "base_client.h"

const int XMPP_RECVBUF_SIZE = 2048;


/**
 * 
 * 
 * @author root (9/14/17)
 * 
 */
class CXmppImpl
{
public:

	//epoll中,socket事件回调
	static void SockEventCallBack(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);
	//读事件
	int OnRead(uint32_t);

	//写事件
	int OnWrite();

	//对端关闭连接
	int OnClose();

	//初始化

	/**
	* 功能:初始化ssl,初始化实例
	* param:
	* 日期:2017-8-28-15:36
	* 作者:jobs
	**/
	int Init(const char *pSendId, const char *pServerKey, OnNotifyCallBack_t func, void *userData);

	/**
	*功能:1. 建立socket 链接 2. 建立ssl链路3. 建立xmpp链路, 成功返回socket id 失败返回 0或-1
	*日期:2017-8-28-15:33
	*作者:jobs
	*/
	int XmppHandshake();

	void CloseXmpp();

	//链路检测
	int CheckSession();

	//发送数据, 单条链路时,当前环境支持,150kb+/s的带宽
	//int AddTask(PaPushData data, bool bImmediate = true);
	int AddTask(shared_ptr<APushData> shared_APushData);

	int OnFcmNotify(string &completeStr);

	/**
	 * 
	 * 
	 * @author lang (9/14/17)
	 * 
	 * @return 成功返回上次心跳时间,300s左右为正常
	 */
	int CheckHeatBeat() {return m_heartBeatTime;}
	int CheckSendMapDataTime();
private:

	int _HandleXmppOnceAck(char *buf);

	int _Write(const char *buf, int len);

	

	time_t 	m_heartBeatTime;

	bool	m_bHandhake;
	//int _Read();
	//int _Close();

	char m_recvBuf[XMPP_RECVBUF_SIZE +1];
	string 	m_packageCache;					//粘包缓存字符串

	CSslSocket m_sslSock;						//ssl链路

	static  string m_strSendId;				//注册本地服务器发送id
	static  string m_strSvrToken;			//注册本地服务器token

	CSendDataMapMgrSharedPtr m_sendDataMapMgr;		//发送数据包管理--管理链路respone和重发

	//CLock	m_bSendAvailableMutex;			//发送状态锁
	//bool m_bSendAvailable;					//发送状态

	//1. 经测试,openssl发起ssl链接时,使用非阻塞和ET模型,出现SSL_ERROR_WANT_READ后(errno:EAGAIN(11)), 
	//如果继续向m_sslSock中写数据,会导致openssl加密协议错误,该错误不可挽回,需注意.应当:停止写数据,
	//等待socket写事件到来后,在继续(尽量多的)写操作,如此往复
	//2. 采用非组赛-ET模型.不做半包处理,如果出现SSL_write写的返回值与buf的长度不一致,将重传整个数据包
	CPackStreamQueue m_PackStreamQueue;		//发送缓冲区 for ET,边缘触发时,使用此数据作为缓存

	OnNotifyCallBack_t m_onNotifyCallBack;
	void *m_userData;
};

#endif

