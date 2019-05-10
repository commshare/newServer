#ifndef _CUST_COMMON_H_
#define _CUST_COMMON_H_

namespace CUST{

enum CUST_COMMAND_TYPE{

	COMMAND_UNKOWN       =0,
	
	//消息接口
	MES_LOGIN            = 0x1001,
	MES_LOGIN_ACK        = 0X1002,
	MES_KICK_OUT         = 0X1003,
	MES_CHAT             = 0X1005,
	MES_CHAT_ACK         = 0X1006,
	MES_CHAT_PULL        = 0X1007,
	MES_CHAT_PULL_ACK    = 0X1008,

	//会话接口
	SESSION_CLOSE        =0X2001,
	SESSION_PULL         =0x2003,
	SESSION_PULL_ACK     =0x2004,
	SESSION_PUSH         =0x2005,
	SESSION_TRANSFER     =0x2007,
	
	//状态接口
	STATUS_UPDATE        =0x3001
};

#define CUST_JSON_FIELD_COMMAND    "command"
#define CUST_JSON_FIELD_USERID     "userId"
#define CUST_JSON_FIELD_SERVICEID  "serviceId"
#define CUST_JSON_FIELD_SESSIONID  "sessionId"
#define CUST_JSON_FIELD_ERRCODE    "errCode"

}
#endif //_CUST_COMMON_H_