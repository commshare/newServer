/******************************************************************************
Filename: custData.h
Description: 保存客服消息以及会话相关内容
******************************************************************************/
#ifndef __CUSTDATA_H__
#define __CUSTDATA_H__

#include <string>
#include "mongoDbColl.h"
#include "bsoncxx/builder/basic/document.hpp"
#include "ostype.h"

using std::string;
using bsoncxx::document::view;

//custMsg collect fields
#define CUSTMSG_FIELD_CMD			"command"
#define CUSTMSG_FIELD_FROM_ID		"fromId"
#define CUSTMSG_FIELD_TO_ID		    "toId"
#define CUSTMSG_FIELD_SERVICEID	    "serviceId"
#define CUSTMSG_FIELD_MSG_ID	    "msgId"
//#define CUSTMSG_FIELD_MSG_TYPE   	"msgType"
#define CUSTMSG_FIELD_ENCRYPT	    "encrypt"
#define CUSTMSG_FIELD_CONTENT	    "content"
#define CUSTMSG_FIELD_MSG_TIME	    "msgTime"
#define CUSTMSG_FIELD_CREATE_TIME    "createTime"
#define CUSTMSG_FIELD_UPDATE_TIME    "updateTime"
#define CUSTMSG_FIELD_PROCESSED      "processed"

//custSession collect fields
#define CUSTSESSION_FIELD_CUSTID     "custId"
#define CUSTSESSION_FIELD_USERID     "userId"
#define CUSTSESSION_FIELD_SERVICEID  "serviceId"
#define CUSTSESSION_FIELD_SESSIONID  "sessionId"
#define CUSTSESSION_FIELD_BEGINTIME  "beginTime"
#define CUSTSESSION_FIELD_ENDTIME    "endTime"

//custWait collect fields
#define CUSTWAIT_FIELD_CUSTID        "custId"
#define CUSTWAIT_FIELD_USERID        "userId"
#define CUSTWAIT_FIELD_SERVICEID     "serviceId"
#define CUSTWAIT_FIELD_ENCRYPT	    "encrypt"
#define CUSTWAIT_FIELD_LASTMSG       "lastMsg"
#define CUSTWAIT_FIELD_LASTMSG_TIME  "lastMsgTime"


//custProcessed collect fields
#define CUSTPROCESSED_FIELD_CUSTID        "custId"
#define CUSTPROCESSED_FIELD_USERID        "userId"
#define CUSTPROCESSED_FIELD_SERVICEID     "serviceId"
#define CUSTPROCESSED_FIELD_ENCRYPT	      "encrypt"
#define CUSTPROCESSED_FIELD_LASTMSG       "lastMsg"
#define CUSTPROCESSED_FIELD_LASTMSG_TIME  "lastMsgTime"

//serviceWait collect fields
#define SERVICEWAIT_FIELD_SERVICEID     "serviceId"
#define SERVICEWAIT_FIELD_USERID        "userId"
#define SERVICEWAIT_FIELD_ENCRYPT	      "encrypt"
#define SERVICEWAIT_FIELD_LASTMSG       "lastMsg"
#define SERVICEWAIT_FIELD_LASTMSG_TIME  "lastMsgTime"

class CCustBase;
class CCustWait;
class CCustProcessed;
class CServiceWait;
	
typedef std::shared_ptr<CCustBase> AutoSessionRec;
class CCustMsg : public IMongoDataEntry
{
public:
	CCustMsg(){};
	virtual ~CCustMsg(){};

	virtual bsoncxx::builder::basic::document ToDoc() const override;

	virtual bool IsValid() const override;

	virtual std::shared_ptr<IMongoDataEntry> Clone()const override;

	virtual unsigned int hashVal() const override;

	virtual bsoncxx::builder::basic::document KeyDoc() const override;

public:
	uint32_t	m_command;  //命令嘛
	string	m_sFromId; //消息发送方ID
	string	m_sToId;   //消息接收方ID
	string  m_sServiceId;   //服务号ID
	string  m_sMsgId; //消息ID
	uint32_t  m_encrypt;  //加密方式
	string  m_sContent; //消息内容
	uint64_t m_msgTime;  //消息时间
	uint64_t m_createTime;  //消息创建时间
	uint64_t m_updateTime;  //消息更新时间
	uint16_t m_processed;   //是否已处理 1-已处理 0-未处理
	
};

class CCustSession : public IMongoDataEntry
{
public:
	CCustSession(){};
	virtual ~CCustSession(){};

	virtual bsoncxx::builder::basic::document ToDoc() const override;

	virtual bool IsValid() const override;

	virtual std::shared_ptr<IMongoDataEntry> Clone()const override;

	virtual unsigned int hashVal() const override;

	virtual bsoncxx::builder::basic::document KeyDoc() const override;

public:
	string	m_sCustId; //客服ID
	string	m_sUserId;   //用户ID
	string  m_sServiceId;   //服务号ID
	string  m_sSessionId; //会话ID
	uint64_t m_beginTime;  //会话开始时间
	uint64_t m_endTime;  //会话结束时间
	
};

class CCustBase : public IMongoDataEntry
{
public:
	CCustBase(){};
	virtual ~CCustBase(){};

	//virtual bsoncxx::builder::basic::document ToDoc() const override;

	//virtual bool IsValid() const override;

	//virtual std::shared_ptr<IMongoDataEntry> Clone()const override;

	//virtual unsigned int hashVal() const override;

	//virtual bsoncxx::builder::basic::document KeyDoc() const override;

public:
	string	m_sCustId; //客服ID
	string	m_sUserId;   //用户ID
	string  m_sServiceId;  //服务号ID
	int32_t  m_encrypt;  //加密方式
	string  m_sLastMsg; //最新消息内容
	int64_t m_LastMsgTime;  //最新消息时间

};

class CCustWait : public CCustBase
{
public:
	CCustWait(){};
	CCustWait(CCustProcessed sessionRec);
	CCustWait(CServiceWait sessionRec);
	virtual ~CCustWait(){};

	virtual bsoncxx::builder::basic::document ToDoc() const override;

	virtual bool IsValid() const override;

	virtual std::shared_ptr<IMongoDataEntry> Clone()const override;

	virtual unsigned int hashVal() const override;

	virtual bsoncxx::builder::basic::document KeyDoc() const override;

public:
	//string	m_sCustId; //客服ID
	//string	m_sUserId;   //用户ID
	//string  m_sServiceId;  //服务号ID
	//uint32_t  m_encrypt;  //加密方式
	//string  m_sLastMsg; //最新消息内容
	//uint64_t m_LastMsgTime;  //最新消息时间

};

class CCustProcessed : public CCustBase
{
public:
	CCustProcessed(){};
	CCustProcessed(CCustWait sessionRec);
	CCustProcessed(CServiceWait sessionRec);
	virtual ~CCustProcessed(){};

	virtual bsoncxx::builder::basic::document ToDoc() const override;

	virtual bool IsValid() const override;

	virtual std::shared_ptr<IMongoDataEntry> Clone()const override;

	virtual unsigned int hashVal() const override;

	virtual bsoncxx::builder::basic::document KeyDoc() const override;

public:
	//string	m_sCustId; //客服ID
	//string	m_sUserId;   //用户ID
	//string  m_sServiceId;  //服务号ID
	//uint32_t  m_encrypt;  //加密方式
	//string  m_sLastMsg; //最新消息内容
	//uint64_t m_LastMsgTime;  //最新消息时间
	
};

class CServiceWait : public CCustBase
{
public:
	CServiceWait(){};
	CServiceWait(CCustWait sessionRec);
	CServiceWait(CCustProcessed sessionRec);
	virtual ~CServiceWait(){};

	virtual bsoncxx::builder::basic::document ToDoc() const override;

	virtual bool IsValid() const override;

	virtual std::shared_ptr<IMongoDataEntry> Clone()const override;

	virtual unsigned int hashVal() const override;

	virtual bsoncxx::builder::basic::document KeyDoc() const override;

public:
	//string	m_sCustId; //客服ID
	//string	m_sUserId;   //用户ID
	//string  m_sServiceId;  //服务号ID
	//uint32_t  m_encrypt;  //加密方式
	//string  m_sLastMsg; //最新消息内容
	//uint64_t m_LastMsgTime;  //最新消息时间
	
};

#endif // __CUSTDATA_H__