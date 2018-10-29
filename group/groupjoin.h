#ifndef __GROUPJOIN_H__
#define __GROUPJOIN_H__

#include "configfilereader.h"
#include "packet.h"
#include "dbhelper.h"
#include "im.pub.pb.h"
#include "im.mes.pb.h"
#include "im.group.pb.h"
#include "groupnotify.h"
#include "im_grpmem.h"



using namespace std;
using namespace im;


class CGroupJoin : public CPacket,public CDbHelper
{
public:
	CGroupJoin(CConfigFileReader* pConfigReader,int nNumOfInst);
	~CGroupJoin();
	
	bool Initialize(void);	
	bool OnApply(std::shared_ptr<CImPdu> pPdu); 	
	bool OnInvite(std::shared_ptr<CImPdu> pPdu); 
	bool OnJoinAck(std::shared_ptr<CImPdu> pPdu); 	
	bool OnPermit(std::shared_ptr<CImPdu> pPdu); 	
	//void SendNotify(AccessUserPara_t* pAccessUserPara);
	void SendApplyNotify(GroupApply* pInstData,ErrCode errCode);
	void SendPermitNotify(GroupPermit* pInstData,ErrCode errCode);
	void SendInviteNotifys(AccessUserPara_t* pAccessUserPara);
	void InviteeUserJoins(AccessUserPara_t* pAccessUserPara);
	void SendGrpActiveNotify(AccessUserPara_t* pAccessUserPara);
	int JoinReq(string sFromId,string sToId,string sGroupId,string sMsgId,uint8_t bMode, const string& strExtend);
	static void OnJoinStartup(CPacket* pAccessPack,AccessUserPara_t* pAccessUserPara,ErrCode errCode); 	
protected:
	virtual bool RegistPacketExecutor(void);
	bool UpdateGroupJoin(void);
	bool JoinStartup(void* pInstData,UidCode_t sessionId, uint8_t bMode);
	bool ApplyProc(GroupApply* pAppyInst,UidCode_t sessionId);
	bool InviteProc(GroupInvite* pInviteInst,UidCode_t sessionId);	
	bool PermitProc(GroupPermit* pPermitInst,UidCode_t sessionId);

	void ApplyRsp(GroupApply* pInstData,UidCode_t sessionId,ErrCode errCode);
	void InviteRsp(GroupInvite* pInstData,UidCode_t sessionId,ErrCode bCode);
	void PermitRsp(GroupPermit* pInstData,UidCode_t sessionId, ErrCode errCode);
	
	
private:
	CConfigFileReader* m_pConfigReader;	
	int 			   m_nNumberOfInst;
};


#endif
