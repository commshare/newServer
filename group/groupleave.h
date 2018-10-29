#ifndef __GROUPLEAVE_H__
#define __GROUPLEAVE_H__

#include "configfilereader.h"
//#include "packet.h"
#include "groupnotify.h"
#include "dbhelper.h"
#include "im.pub.pb.h"
#include "im.group.pb.h"
#include "im.mes.pb.h"
#include "im_grpmem.h"




using namespace std;
using namespace im;


class CGroupLeave : public CPacket,public CDbHelper
{
public:
	CGroupLeave(CConfigFileReader* pConfigReader,int nNumOfInst);
	~CGroupLeave();
	
	bool Initialize(void);	
	bool OnKickout(std::shared_ptr<CImPdu> pPdu); 	
	bool OnQuit(std::shared_ptr<CImPdu> pPdu); 
	void SendNotify(AccessUserPara_t* pAccessUserPara);
	static void OnRemoveStartup(CPacket* pAccessPack,AccessUserPara_t* pAccessUserPara,ErrCode errCode); 	

protected:
	virtual bool RegistPacketExecutor(void);
	bool RemoveStartup(void* pInstData,UidCode_t sessionId, uint8_t bMode);
	bool KickoutProc(GroupKickOut* pKickoutInst,UidCode_t sessionId);
	bool QuitProc(GroupQuit* pQuitInst,UidCode_t sessionId);	
	void KickoutRsp(GroupKickOut* pKickoutInst, UidCode_t sessionId,ErrCode bCode);
	void QuitRsp(GroupQuit* pQuitInst, UidCode_t sessionId,ErrCode bCode);
private:
	CConfigFileReader* m_pConfigReader;	
	int 			   m_nNumberOfInst;
};


#endif
