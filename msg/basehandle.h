/******************************************************************************
Filename: basehandle.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description: 
******************************************************************************/
#ifndef __BASEHANDLE_H__
#define __BASEHANDLE_H__

#include "packet.h"
#include "im.pub.pb.h"
#include "offlineMsgMgr.h"

class COfflineMsg;
typedef void (CPacket::*MongoInsertCallBack)(const COfflineMsg& msg, bool result, void* paras);	// command processing function pointer, use to process command.

const char* getofflineTypeStr(unsigned int cmdId);

extern const string NewMsgStr;
extern const string NewRequestStr;

class CConfigFileReader;
class CLoginInfo;

namespace google
{
	namespace protobuf
	{
		class MessageLite;
	}
}

typedef struct _OfflineMsgInsertCallBackParas
{
	_OfflineMsgInsertCallBackParas(CPacket* handle, const UidCode_t& sessionId, bool bNeedSendPush = false)
		:m_bNeedSendPush(bNeedSendPush), m_sessionID(sessionId), m_handle(handle){}
	bool m_bNeedSendPush;
	const UidCode_t m_sessionID;
	CPacket* m_handle;
}OfflineMsgInsertCallBackParas_t;

class CBaseHandle : public CPacket
{
public:
	CBaseHandle(CConfigFileReader* pConfigReader);
	virtual ~CBaseHandle();
	bool Initialize(void);
protected:
	int		sendAck(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const UidCode_t& sessionId, int nDirection = 0);
	int		sendReq(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const string& ip, int port);
	int		sendReq(const google::protobuf::MessageLite* pMsg, uint16_t command_id, int16_t nServiceId);
	void sendPush(const string& fromId, const string& toId, const string& msgId, im::MsgType msgType, 
					const string& content, int pushType, const string& pushToken, const string& voipToken, int vesionCode);

	void sendAndroidPush(const string& fromId, const string& toId, const string& msgId, im::MsgType msgType, 
													const string& content, const string& pushToken, int pushType);
	void sendiOSPush(const string& fromId, const string& toId, const string& msgId, im::MsgType msgType, const string& content, 
													const string& pushToken, int pushType, int callType, const std::string& callId);
	string  getAppName()const;
	bool checkDeviceLastUser(const std::string& strUserId, const std::string& strDeviceToken);

protected:
	virtual bool RegistPacketExecutor(void) = 0;
	void getConfigData();
protected:
	CConfigFileReader* m_pConfigReader;	
	COfflineMsgMgr	m_offlineMsgMgr;

protected:
	std::string m_grpInfoUrl;
	std::string m_usrInfoUrl;
	std::string m_grpMemberInfoUrl;
	std::string m_friendInfoUrl;
	std::string m_sAppSecret;
};
#endif // __BASEHANDLE_H__



