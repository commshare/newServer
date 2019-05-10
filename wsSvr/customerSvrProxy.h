#ifndef CUSTOMERSVRPROXY_H_
#define CUSTOMERSVRPROXY_H_

#include "basehandle.h"
#include <imappframe.h>
#include <string>
#include "custDataMgr.h"

class CImPdu;
class CConfigFileReader;

class CCustomerSvrProxy : public CBaseHandle
{
public:
	CCustomerSvrProxy(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CCustomerSvrProxy();

	//处理用户发过来的消息
	void OnMsgChatDeliver(std::shared_ptr<CImPdu> pPdu);						
	
	
private:
	void ignoreMsg(std::shared_ptr<CImPdu> pPdu);
	//bool   userIsProcessing(const string& userId,const string& serviceID);
	void   dispatchUserToCust(const string& userId,const string& serviceID,const string& questionId);
protected:	
	virtual bool RegistPacketExecutor(void);

	virtual void OnInitialize() override;

private:
	int 	m_nNumberOfInst;
	CCustMsgMgr m_custMsgMgr;
};

#endif




