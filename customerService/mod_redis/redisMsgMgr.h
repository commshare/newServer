/******************************************************************************
Filename: redisFriendMgr.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/05
Description: 
******************************************************************************/
#ifndef REDISMSGMGR_H_
#define REDISMSGMGR_H_


#include <string>
#include <vector>
using std::string;


#define  MSG_FIELD_CMD					"command"
#define  MSG_FIELD_CONTENT				"content"
#define  MSG_FIELD_SFROMID				"sFromId"
#define  MSG_FIELD_STOID				"sToId"
#define  MSG_FIELD_SMSGID				"sMsgId"
#define  MSG_FIELD_MSGTYPE				"msgType"
#define  MSG_FIELD_MSGTIME				"msgTime"
#define  MSG_FIELD_ENCRYPT				"encrypt"
#define  MSG_FIELD_SCONTENT				"sContent"
#define  MSG_FIELD_NCANCELTYPE			"nCancelType"
#define  MSG_FIELD_SGROUPID				"sGroupId"
#define  MSG_FIELD_CANCELTIME			"cancelTime"
#define  MSG_FIELD_SSERVICEID           "sServiceId"
#define  MSG_FIELD_SQUESTIONID          "sQuestionId"

class CReidsProtoMsgMgr
{
public:
	CReidsProtoMsgMgr();
	~CReidsProtoMsgMgr();
	static bool InsertProtoMsg(const std::vector<std::string>& msgs);
	static string PopCustomerServiceMsg();
public:
	static const char* getMsgReadKey();
	static const char* getMsgWriteKey();
};

#endif