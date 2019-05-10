#ifndef _CUST_COMMON_H_
#define _CUST_COMMON_H_

#include <string>

using namespace std;


#define CUST_JSON_FIELD_COMMAND     "command"
#define CUST_JSON_FIELD_USERID      "userId"
#define CUST_JSON_FIELD_FROM_CUSTID  "fromCustId"
#define CUST_JSON_FIELD_FROM_CUSTNAME  "fromCustName"
#define CUST_JSON_FIELD_TO_CUSTID    "toCustId"
#define CUST_JSON_FIELD_CUSTID     "custId"
#define CUST_JSON_FIELD_SERVICEID  "serviceId"
#define CUST_JSON_FIELD_SESSIONID  "sessionId"
#define CUST_JSON_FIELD_ERRCODE    "errCode"
#define CUST_JSON_FIELD_SESSION_TYPE  "sessionType"
#define CUST_JSON_FIELD_SESSION_OPT    "opt"
#define CUST_JSON_FIELD_SESSION_LIST   "sessionList"
#define CUST_JSON_FIELD_LASTPAGE_FLAG "lastPageFlag"
#define CUST_JSON_FIELD_NICK_NAME     "nickName"
#define CUST_JSON_FIELD_HEAD_PIXEL    "headPixel"
#define CUST_JSON_FIELD_MSG_CONTENT    "msgContent"
#define CUST_JSON_FIELD_MSG_TIME       "msgTime"
#define CUST_JSON_FIELD_CUST_STATUS    "status"
#define CUST_JSON_FIELD_PAGEINDEX      "pageIndex"
#define CUST_JSON_FIELD_PAGESIZE       "pageSize"
#define CUST_JSON_FIELD_TIMESTAMP      "timeStamp"

//#define  KEY_USER_PROCESSING_   "USER_PROCESSING_"      //正在处理当前用户的客服

//#define  KEY_SESSION_USER_INFO_          "SESSION_USER_INFO_"     //会话用户信息
#define  KEY_CUSTOMER_SESSION_CURRENT_   "CUST_SESSION_CURRENT_"  //客服当前会话列表
#define  KEY_CUSTOMER_SESSION_HISTORY_   "CUST_SESSION_HISTORY_"  //客服历史会话列表
#define  KEY_CUSTOMER_SESSION_QUEUE_     "CUST_SESSION_QUEUE_"    //客服排队会话列表

#define  KEY_USER_CURRENT_IN_       "USER_DISPATCH_IN_"           //用户所在客服当前会话队列或排队列表
#define  VALUE_CUST_ID            "custId"                        //用户所在客服
#define  VALUE_SESSION_TYPE       "sessionType"                   //当前会话队列或排队队列

#define  KEY_USER_INFO   "USER_INFO_"                   //用户信息
#define  VALUE_USER_USER_ID  "userId"                   //用户ID
#define  VALUE_USER_NICK_NAME "nickName"                //用户昵称
#define  VALUE_USER_HEADPIXEL "headPixel"               //用户头像url

#define  KEY_CUST_INFO_            "CUST_INFO_"                         //客服信息（包括状态以及连接信息）
#define  VALUE_CUST_STATUS         "custStatus"              //客服状态
#define  VALUE_CUST_IN_WSSERVER_IP_PORT    "wsServerIP"      //客服连接的WSServer IP&Port
//#define  VALUE_CUST_IN_WSSERVER_PORT  "wsServerPort"     //客服连接的WSServer Port


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
	MES_CHAT_HEARTBRAT   = 0X1009,

	//会话接口
	SESSION_CLOSE        =0X2001,
	SESSION_PULL         =0x2003,
	SESSION_PULL_ACK     =0x2004,
	SESSION_PUSH         =0x2005,
	SESSION_TRANSFER     =0x2007,
	SESSION_ACTIVE       =0x2009,
	SESSION_ACTIVE_ACK   =0x2010,
	
	//状态接口
	STATUS_UPDATE        =0x3001
};

enum USER_STATUS{
	STATUS_ONLINE	= 1,  //在线
	STATUS_BUSY 	= 2,  //忙碌
	STATUS_OFFLIE	= 3,  //离线
	STATUS_QUIT    =  4   //退出
};

enum SESSION_TYPE{
	TYPE_CURRENT   = 1,   //当前会话
	TYPE_HISTORY   = 2,   //历史会话
	TYPE_QUEUE	   = 3	  //排队会话
};

enum SESSION_OPT{
	OPT_ADD     = 1,   //增加
	OPT_REMOVE  = 2    //移除
};

typedef struct{
	string custId;	//当前客服ID
	SESSION_TYPE sessionType; //当前客服会话类型
	string wsServerIPPort;	 //客服所登录的WSService IP&Port
}USER_PROCESSING_INFO;


typedef struct{
	string custId;
	USER_STATUS custStatus;	//客服状态
	string wsServerIPPort;  //客服登录的WSServer IP&Port 
}CUST_INFO;

typedef struct
{
	string userId;   //用户ID
	string nickName;  //用户昵称
	string headPixel; //用户头像url
}USER_INFO;

typedef struct{
	string userId;   //用户ID
	string nickName;  //用户昵称
	string headPixel; //用户头像url
	uint32_t  encrypt;  //加密方式
	string lastMsgContent; //最新消息
	uint64_t lastMsgTime;  //最新消息时间
}SESSION_INFO;


inline string packCustSessionCurrent(const string& custId,const string& serviceId)
{
	char key[128];
	snprintf(key,sizeof(key),"%s%s_%s",KEY_CUSTOMER_SESSION_CURRENT_,custId.c_str(),serviceId.c_str());
	return key;
}

inline string packCustSessionHistory(const string& custId,const string& serviceId)
{
	char key[128];
	snprintf(key,sizeof(key),"%s%s_%s",KEY_CUSTOMER_SESSION_HISTORY_,custId.c_str(),serviceId.c_str());
	return key;
}

inline string packCustSessionQueue(const string& custId,const string& serviceId)
{
	char key[128];
	snprintf(key,sizeof(key),"%s%s_%s",KEY_CUSTOMER_SESSION_QUEUE_,custId.c_str(),serviceId.c_str());
	return key;
}

inline string packCustBind(const string& custId,const string& serviceId)
{
	char key[128];
	snprintf(key,sizeof(key),"%s_%s",custId.c_str(),serviceId.c_str());
	return key;
}

inline void unpackCustBind(const string&custBind,string& custId,string& serviceId)
{
	size_t pos = custBind.find('_');
	custId = custBind.substr(0,pos);
	serviceId = custBind.substr(pos+1);
}


}

#endif //_CUST_COMMON_H_
