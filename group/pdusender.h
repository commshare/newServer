/******************************************************************************
Filename: pdusender.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/09/22
Description: 
******************************************************************************/
#ifndef __PDUSENDER_H__
#define __PDUSENDER_H__

#include "basehandle.h"
#include "packet.h"
#include "singleton.h"

namespace google
{
	namespace protobuf
	{
		class MessageLite;
	}
}

//class CPduSenderTask
//{
//public:
//	enum PDUTASKTYPE
//	{
//		PDU_TASK_TYPE_SESSION	= 1,
//		PDU_TASK_TYPE_IPPORT	= 2,
//		PDU_TASK_TYPE_SVRID		= 3
//	};
//public:
//	CPduSenderTask(const google::protobuf::MessageLite* pMsg, uint16_t command_id, int16_t nServiceId);
//	~CPduSenderTask();
//private:
//	CImPdu m_pdu;
//	int m_nSvrId;
//	PDUTASKTYPE m_pduType;
//};

//在新线程中发送
class CPduSender : public CPacket, public Singleton<CPduSender>
{
public:
	CPduSender();
	~CPduSender();
	//int		sendAck(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const UidCode_t& sessionId, int nDirection = 0);
	//int		sendReq(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const string& ip, int port);
	//同步发送数据,即调用线程中发送
	int		sendReq(const google::protobuf::MessageLite* pMsg, uint16_t command_id, int16_t nServiceId);
private:

};
#endif // __PDUSENDER_H__




