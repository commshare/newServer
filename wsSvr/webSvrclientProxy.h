#ifndef WEBSVRCLIENTPROXY_H_
#define WEBSVRCLIENTPROXY_H_

#include <queue>
#include <string>
#include "thread.h"
//客服对应的客户对象
//1、读取客服发送的消息并处理
//2、将实际客户发送的消息发送给客服

class CConfigFileReader;
class CLock;

#define MSG_PROCESS_UNIT		100
#define MSG_PROCESS_INTERVAL	5

typedef std::queue<std::string> MsgQueue_t;
class CWebSvrClientProxy : public CThread
{
public:
	CWebSvrClientProxy(CConfigFileReader* pConfigReader);
	~CWebSvrClientProxy();

	virtual void OnThreadRun(void) override;
	void StopThread();					//Stop packet processing thread.
	bool Initialize(void);
public:
	//提交客户端的消息
	void PostClientMsg(const std::string& msg);
	//响应客服发过来的消息
	void OnCustomerSvrMsg(const std::string& jsonContent);

private:
	bool	m_bRunning;
	CLock	m_Lock;					//Thread Lock。
	MsgQueue_t m_queueMsg;			//消息队列
};


#endif  // WEBSVRCLIENTPROXY_H_

