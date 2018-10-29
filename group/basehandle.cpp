/******************************************************************************
Filename: basehandle.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description: 
******************************************************************************/
#include "basehandle.h"
#include "configfilereader.h"
#include "im.pushSvrAPNsMsg.pb.h"
#include "im.push.android.pb.h"
#include "im.mes.pb.h"
#include "util.h"
#include "im_time.h"
#include "redisGrpMgr.h"
#include "mysqlGrpmemMgr.h"

using namespace im;
using namespace std;

CBaseHandle::CBaseHandle(CConfigFileReader* pConfigReader)
	: m_pConfigReader(pConfigReader)
{

}

CBaseHandle::~CBaseHandle()
{
	
}


int CBaseHandle::sendAck(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const UidCode_t& sessionId, int nDirection)
{
	CImPdu pdu;
	pdu.SetPBMsg(pMsg);
	pdu.SetSessionId(sessionId);
	pdu.SetCommandId(command_id);
	int retCode = SendPdu(&pdu,nDirection);
	static int msgChatAckCount = 0;
	if (MES_CHAT_ACK == command_id)
	{
		++msgChatAckCount;
		DbgLog("sendMsgChatAck %d times", msgChatAckCount);
	}

	if (retCode <= 0)
	{
		WarnLog("!!!send command 0x%x failed, return %d", command_id, retCode);
	}
	return retCode;
}

int CBaseHandle::sendReq(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const string& ip, int port)
{
	CImPdu pdu;
	pdu.SetPBMsg(pMsg);
	pdu.SetCommandId(command_id);
	int retCode = SendPdu(ip, port, &pdu);
	if (retCode <= 0)
	{
		WarnLog("!!!send command 0x%x failed, return %d", command_id, retCode);
	}
	return retCode;
}

int CBaseHandle::sendReq(const google::protobuf::MessageLite* pMsg, uint16_t command_id, int16_t nServiceId)
{
	//log("enter sendReq at %llu", getCurrentTime_usec());
	CUsecElspsedTimer myTimer;
	myTimer.start();
	CImPdu pdu;
	pdu.SetPBMsg(pMsg);
	pdu.SetCommandId(command_id);
	//log("create pdu use %llu useconds, currentTime = %llu", myTimer.elapsed(), getCurrentTime_usec());
	int retCode = SendPdu(nServiceId, &pdu);
	//log("send pdu to server %d use %llu useconds, currentTime = %llu", nServiceId, myTimer.elapsed(), getCurrentTime_usec());
	if (retCode <= 0)
	{
		WarnLog("!!!send command 0x%x to svr %d failed, return %d", command_id, nServiceId, retCode);
	}
	//log("leave sendReq at %llu", getCurrentTime_usec());
	return retCode;
}

bool CBaseHandle::Initialize(void)
{
	bool retcode = RegistPacketExecutor();
	if (retcode)
		StartThread();

	return retcode;
}

std::map<std::string, CGrpMem> GetGrpMems(const string& grpId)
{
	std::map<std::string, CGrpMem> grpmems;
	try
	{
		grpmems = CReidsGrpMgr::GetGrpMems(grpId);
		if (grpmems.empty())
		{
			grpmems = CMysqlGrpmemMgr::GetGrpmems(grpId);
			if (!grpmems.empty())
			{
				CReidsGrpMgr::InsertGrpMem(grpId, grpmems);
			}
		}
	}
	catch (...)
	{
		
	}	
	return grpmems;
}
