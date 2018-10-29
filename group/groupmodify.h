#ifndef __GROUPMODIFY_H__
#define __GROUPMODIFY_H__

#include "configfilereader.h"
#include "packet.h"
#include "dbhelper.h"
#include "groupnotify.h"
#include "im.pub.pb.h"
#include "im.group.pb.h"
#include "im.mes.pb.h"
#include "im_grpmem.h"



using namespace std;
using namespace im;


class CGroupModify : public CPacket,public CDbHelper
{
public:
	CGroupModify(CConfigFileReader* pConfigReader,int nNumOfInst);
	~CGroupModify();
	
	bool Initialize(void);	
	void SendNotify(AccessUserPara_t* pAccessUserPara);
	bool OnModify(std::shared_ptr<CImPdu> pPdu); 		
	static void  OnModifyStartup(CPacket* pAccessPack,AccessUserPara_t* pAccessUserPara,ErrCode     errCode);
protected:
	virtual bool RegistPacketExecutor(void);
	bool ModifyStartup(GroupModify* pModifyInst,UidCode_t sessionId);
	bool ModifyProc(GroupModify* pModifyInst,UidCode_t sessionId);
	void ModifyRsp(GroupModify* pModifyInst, UidCode_t sessionId,ErrCode bCode);	

private:
	CConfigFileReader* m_pConfigReader;	
	int 			   m_nNumberOfInst;
};


#endif
