/******************************************************************************
Filename: basehandle.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description:  
******************************************************************************/
#ifndef __BASEHANDLE_H__
#define __BASEHANDLE_H__

#include "packet.h"
#include "im.pub.pb.h"
#include "im.pushSvrAPNsMsg.pb.h"
//#include "im.and"
#include "util.h"
#include "im_time.h"
#include "loginInfo.h"

class CConfigFileReader;

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
	
    int		sendAck(const string& pMsg, uint16_t command_id, const UidCode_t& sessionId, int nDirection = 0);
	int		sendReq(const string& pMsg, uint16_t command_id, const string& ip, int port);
	int		sendReq(const string& pMsg, uint16_t command_id, int16_t nServiceId);
    
	void sendPush(const string& fromId, const string& toId, const string& msgId, im::MsgType msgType, 
					const string& content, int pushType, const string& pushToken, const string& voipToken, int vesionCode);
    void    sendPush(std::shared_ptr<CLoginInfo>& pLogin, const string& fromId, const string& toId, const string& msgId, MsgType msgType, const string& content);
protected:
	virtual bool RegistPacketExecutor(void) = 0;
protected:
	CConfigFileReader* m_pConfigReader;	
};
#endif // __BASEHANDLE_H__



