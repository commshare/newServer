/*********************
author:bob
date:2018-12-12
description:定义ws连接的基础操作
*********************/
#ifndef _WSBASE_H
#define _WSBASE_H

#include "util.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "threadpool.h"

typedef websocketpp::server<websocketpp::config::asio> wsServer;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
//using websocketpp::frame::opcode;


// pull out the type of messages sent by our config
typedef wsServer::message_ptr message_ptr;

class CWsBase
{
public:
	CWsBase(){};
	virtual ~CWsBase(){};

	virtual bool init(uint32_t listen_port,uint32_t work_size);
	bool send(websocketpp::connection_hdl hdl,const string& msg);
	
protected:
	virtual void on_message(wsServer* s, websocketpp::connection_hdl hdl, message_ptr msg){};
	virtual void on_open(websocketpp::connection_hdl hdl);
	virtual void on_close(websocketpp::connection_hdl hdl);
	virtual void close(websocketpp::connection_hdl hdl,const string& reason);
	
	bool run();

protected:
	wsServer m_wsSocket;
	CImThreadPool m_taskThreadPool;
};

#endif //_WSBASE_H

