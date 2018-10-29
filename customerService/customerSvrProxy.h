#ifndef CUSTOMERSVRPROXY_H_
#define CUSTOMERSVRPROXY_H_

#include "basehandle.h"
#include <imappframe.h>
#include <string>

class CImPdu;
class CConfigFileReader;

class CCustomerSvrProxy : public CBaseHandle
{
public:
	CCustomerSvrProxy(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CCustomerSvrProxy();

	//处理客户发过来的消息
	void OnMsgChatDeliver(std::shared_ptr<CImPdu> pPdu);						
	void OnMsgChatCancelDeliver(std::shared_ptr<CImPdu> pPdu);	
	void OnMsgChatReadDeliver(std::shared_ptr<CImPdu> pPdu);
	void OnMsgChatDeliverNotify(std::shared_ptr<CImPdu> pPdu);
	void OnGetOfflineMsgAck(std::shared_ptr<CImPdu> pPdu);
public:
	int postCustMsg(const std::string& jsonMsg);		//将客服发过来的消息发给msg服务器
private:
	int postClientMsg(const string& msg);				//将client消息发送给客服
	//int postClientOfflineMsg(const im::MESOfflineMsgAck& msg);

	void ignoreMsg(std::shared_ptr<CImPdu> pPdu);
	//获取并处理离线信息
	int sendGetOfflineMsgRequest();
	static void* GetOffLineMsgRoutine(void* arg);
protected:	
	virtual bool RegistPacketExecutor(void);

	virtual void OnInitialize() override;

private:
	int 	m_nNumberOfInst;
};

#endif




