#ifndef __PDUSENDER_H__
#define __PDUSENDER_H__

#include "packet.h"
#include "singleton.h"
#include "im.mes.pb.h"
#include "im.cm.pb.h"
//#include "im.group.pb.h"
#include "im.inner.pb.h"
#include "im.pub.pb.h"

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




