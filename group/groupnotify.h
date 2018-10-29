#ifndef __GROUPNOTIFY_H__
#define __GROUPNOTIFY_H__

#include <vector>
#include "basehandle.h"
namespace im
{
	class _InnerGrpNotify;
}

class CGroupNotify : public CBaseHandle
{
public:	
	CGroupNotify(CConfigFileReader* pConfigReader, int nNumOfInst);
	~CGroupNotify();
	
	bool Initialize(void);	
	bool OnNotifyAck(std::shared_ptr<CImPdu> pPdu); 	
public:
	//static int sendNotify(const string& grpId, const string& toId, im::NotifyType notifyType, const string& content, im::ErrCode errcode,
	//	const string& oprUserId = string(""), const std::vector<string>& mnpledUserId = std::vector<string>());
	static int sendNotify(const im::_InnerGrpNotify& notify);
protected:
	virtual bool RegistPacketExecutor(void);
private:
	CConfigFileReader* m_pConfigReader;	
	int 			   m_nNumberOfInst;
};


#endif
