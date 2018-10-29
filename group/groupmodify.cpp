#include "groupmodify.h"

CGroupModify::CGroupModify(CConfigFileReader* pConfigReader,int nNumOfInst)
	: m_pConfigReader(pConfigReader),m_nNumberOfInst(nNumOfInst)
{
}

CGroupModify::~CGroupModify()
{

}
	
bool CGroupModify::Initialize(void)
{
	if(!m_pConfigReader)					// Handle of config instance is null
		return false;

	if(!CDbHelper::Initialize(m_pConfigReader))
	{
		ErrLog("Failed to initialize database...");
		return false;
	}


	RegistPacketExecutor();
	StartThread();
	
	return true;
}

bool CGroupModify::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(GROUP_MODIFY,m_nNumberOfInst,CommandProc(&CGroupModify::OnModify));

	return true;
}

void CGroupModify::SendNotify(AccessUserPara_t* pAccessUserPara)
{
	if(!pAccessUserPara || !pAccessUserPara->pUserData)
		return;
	
	void* pUserData = pAccessUserPara->pUserData;

	_InnerGrpNotify notify;
	string sToId = (((GroupModify*)pUserData)->nmodifytype()==NOTIFY_TYPE_GRPMASTER_CHANGED) ?
					((GroupModify*)pUserData)->sinvolvedid() : ((GroupModify*)pUserData)->sgroupid();
	
	notify.set_smsgid(((GroupModify*)pUserData)->smsgid());
	notify.set_sgrpid(((GroupModify*)pUserData)->sgroupid());
	notify.add_lstoid(sToId);
	notify.set_sopruserid(((GroupModify*)pUserData)->suserid());
	if(((GroupModify*)pUserData)->nmodifytype()==NOTIFY_TYPE_GRPMASTER_CHANGED)
	{
		notify.add_smnpleduserid(sToId);		
	}
	notify.set_scontent(((GroupModify*)pUserData)->scontent());
	notify.set_notifytype(((GroupModify*)pUserData)->nmodifytype());
	notify.set_errcode(NON_ERR);
	
	CGroupNotify::sendNotify(notify);

}

void  CGroupModify::OnModifyStartup(CPacket* pAccessPack,AccessUserPara_t* pAccessUserPara,ErrCode      errCode)
{
	if(!pAccessUserPara || !pAccessPack)
	{
		ErrLog("Access data is null when accessing group creation!");
		return;
	}

	
	GroupModify* pModifyInst = (GroupModify*)pAccessUserPara->pUserData;
	CGroupModify * pModifyPacket = (CGroupModify*)pAccessPack;
	
	pModifyPacket->ModifyRsp(pModifyInst,pAccessUserPara->sessionId,errCode);	

	if(errCode == NON_ERR)
	{
		InfoLog("User %s has modify group information %s in successful!",
				pModifyInst->suserid().c_str(), pModifyInst->sgroupid().c_str());
		pModifyPacket->SendNotify(pAccessUserPara);
	}
	else
	{
		ErrLog("Failed to execute the modification %d ,errCode 0x%x",
			pModifyInst->nmodifytype(),errCode);
	}


	delete (GroupModify*)pAccessUserPara->pUserData;
	delete pAccessUserPara;					// Free user instance data.
	
}

bool CGroupModify::ModifyStartup(GroupModify* pInstData,UidCode_t sessionId)
{
	if(!pInstData)
	{
		ErrLog("The Remove instance data should not be null!");
		return false;
	}

	SqlAccess_t sqlAccess;

	
	AccessUserPara_t* pAccessPara =  new AccessUserPara_t;	
	pAccessPara->pUserData = (void*)pInstData;
	pAccessPara->sessionId = sessionId;
		
	sqlAccess.accessCb = AccessCallback(&CGroupModify::OnModifyStartup);
	sqlAccess.pAccessPacket = this;
	sqlAccess.pAccessPara = pAccessPara;
	sqlAccess.accessTask = MODIFY_GROUP_TASK;
									
	return DatabaseHelper(sqlAccess);

}


bool CGroupModify::ModifyProc(GroupModify* pModifyInst,UidCode_t sessionId)
{	
	if(!ModifyStartup(pModifyInst,sessionId))
	{
		ErrLog("Failed to start group modify for the internal exception!");
		ModifyRsp(pModifyInst,sessionId,ERR_GROUP_INTEREXCEPTION);
		return false;
	}
	InfoLog("Startup user %s group modifying ...",pModifyInst->suserid().c_str());
	return true;	

}


bool CGroupModify::OnModify(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("encounter a null pdu when processing group modify!");
		return false;
	}
		
	GroupModify* pModifyInst = new GroupModify;
			
	UidCode_t sessionId = pPdu->GetSessionId();
	uchar_t* pContent = pPdu->GetBodyData();
		
	if(!pContent || !pModifyInst->ParseFromArray(pContent,pPdu->GetBodyLength())||
		pModifyInst->suserid().empty() || !IsGrpIdValid(pModifyInst->sgroupid()))
	{
		ErrLog("Group quit parameter error!");
		delete pModifyInst;
		
		return false;		
	}
	InfoLog("User %s group %s modify type %d with InstId = %d , cmd = 0x%x ,size = %d",pModifyInst->suserid().c_str(),
		pModifyInst->sgroupid().c_str(),pModifyInst->nmodifytype(),m_nNumberOfInst,pPdu->GetCommandId(),pPdu->GetBodyLength());	

	return ModifyProc(pModifyInst,sessionId);
}

void CGroupModify::ModifyRsp(GroupModify* pModifyInst, UidCode_t sessionId,ErrCode bCode)
{
	if(!pModifyInst)
	{
		ErrLog("Failed to response modification for the modifying instance data is null!");
		return;
		
	}
	GroupModifyAck modifyAck;
	CImPdu	   	modifyAckPdu;

	modifyAck.set_suserid(pModifyInst->suserid());
	modifyAck.set_sgroupid(pModifyInst->sgroupid());
	modifyAck.set_smsgid(pModifyInst->smsgid());
	modifyAck.set_errcode(bCode);

	modifyAckPdu.SetPBMsg(&modifyAck);
	modifyAckPdu.SetCommandId(GROUP_MODIFY_ACK);
	modifyAckPdu.SetSessionId(sessionId);

	if(SendPdu(&modifyAckPdu)<0)
	{
		ErrLog("Failed to response the user's %s modify,perhaps the linker is lost！",
			pModifyInst->suserid().c_str());
	}
}


