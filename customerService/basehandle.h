/******************************************************************************
Filename: basehandle.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description: 
******************************************************************************/
#ifndef __BASEHANDLE_H__
#define __BASEHANDLE_H__

#include "packet.h"
#include "im.pub.pb.h"

class COfflineMsg;
typedef void (CPacket::*MongoInsertCallBack)(const COfflineMsg& msg, bool result, void* paras);	// command processing function pointer, use to process command.

const char* getCmdStr(unsigned int cmdId);

class CConfigFileReader;
class CLoginInfo;

namespace google
{
	namespace protobuf
	{
		class MessageLite;
	}
}

class CBaseHandle : public CPacket
{
public:
	CBaseHandle(CConfigFileReader* pConfigReader);
	virtual ~CBaseHandle();
	bool Initialize(void);
public:
	int		sendAck(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const UidCode_t& sessionId, int nDirection = 0);
	int		sendReq(const google::protobuf::MessageLite* pMsg, uint16_t command_id, int16_t nServiceId);
	//int		sendReq(const string& msg, uint16_t command_id, int16_t nServiceId);
protected:
	virtual bool RegistPacketExecutor(void) = 0;

	virtual void OnInitialize(){};

#ifdef DEBUG
	virtual void OnThreadRun(void) override;
#endif

protected:
	CConfigFileReader* m_pConfigReader;	
};
#endif // __BASEHANDLE_H__



