/******************************************************************************
Filename: basehandle.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description:  
******************************************************************************/
#ifndef __BASEHANDLE_H__
#define __BASEHANDLE_H__

#include "packet.h"
#include "im.pub.pb.h"
#include "util.h"
#include "im_time.h"
#include "im_grpmem.h"

class COfflineMsg;
typedef void (CPacket::*MongoInsertCallBack)(const COfflineMsg& msg, bool result, void* paras);	// command processing function pointer, use to process command.

class CConfigFileReader;
class CLoginInfo;

namespace google
{
	namespace protobuf
	{
		class MessageLite;
	}
}

std::map<std::string, CGrpMem> GetGrpMems(const string& grpId);


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
protected:
	virtual bool RegistPacketExecutor(void) = 0;
protected:
	CConfigFileReader* m_pConfigReader;	
};
#endif // __BASEHANDLE_H__



