/******************************************************************************
Filename: basehandle.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/15
Description: 
******************************************************************************/
#include "pdusender.h"
#include "im_time.h"

using namespace im;
using namespace std;

CPduSender::CPduSender()
{

}

CPduSender::~CPduSender()
{
	
}

int CPduSender::sendReq(const google::protobuf::MessageLite* pMsg, uint16_t command_id, int16_t nServiceId)
{
	CUsecElspsedTimer myTimer;
	myTimer.start();
	CImPdu pdu;
	pdu.SetPBMsg(pMsg);
	pdu.SetCommandId(command_id);
	int retCode = SendPdu(nServiceId, &pdu);
	if (retCode <= 0)
	{
		WarnLog("!!!send command 0x%x to svr %d failed, return %d", command_id, nServiceId, retCode);
	}
	return retCode;
}

