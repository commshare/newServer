/****************************************
author:bob
date:2018-12-12
description:customer websocket 代理类
*****************************************/
#ifndef _CUSTWEBPROXY_H_
#define _CUSTWEBPROXY_H_

#include <queue>
#include <string>
#include "packet.h"
#include "wsBase.h"


class CConfigFileReader;
class CLock;

#define MSG_PROCESS_UNIT		100
#define MSG_PROCESS_INTERVAL	5

typedef std::queue<std::string> MsgQueue_t;
class CCustWebProxy : public CPacket,public CWsBase,public Singleton<CCustWebProxy>
{
public:
	CCustWebProxy();//(CConfigFileReader* pConfigReader);
public:
	~CCustWebProxy();

	virtual void OnThreadRun(void) override;
	void StopThread();					//Stop packet processing thread.
	virtual bool init(uint32_t listen_port,uint32_t work_size);
public:
	//提交客户端的消息
	void PostClientMsg(const std::string& msg);
	//响应客服发过来的消息
	void OnCustomerSvrMsg(const std::string& jsonContent);

	void on_message(wsServer* s, websocketpp::connection_hdl hdl, message_ptr msg);
	virtual void on_close(websocketpp::connection_hdl hdl);
	virtual void close(websocketpp::connection_hdl hdl,const string& reason);
	

private:
	bool	m_bRunning;
	CLock	m_Lock;					//Thread Lock。
	MsgQueue_t m_queueMsg;			//消息队列
};


#endif  // _CUSTWEBPROXY_H_


