#include "groupleave.h"

CGroupLeave::CGroupLeave(CConfigFileReader* pConfigReader,int nNumOfInst)
	: m_pConfigReader(pConfigReader),m_nNumberOfInst(nNumOfInst)
{
}

CGroupLeave::~CGroupLeave()
{

}
	
bool CGroupLeave::Initialize(void)
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

bool CGroupLeave::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(GROUP_KICKOUT,m_nNumberOfInst,CommandProc(&CGroupLeave::OnKickout));
	CmdRegist(GROUP_QUIT,m_nNumberOfInst, CommandProc(&CGroupLeave::OnQuit));

	return true;
}

void CGroupLeave::SendNotify(AccessUserPara_t* pAccessUserPara)
{
	if(!pAccessUserPara || !pAccessUserPara->pUserData)
		return;
	
	void* pUserData = pAccessUserPara->pUserData;

	_InnerGrpNotify notify;
	NotifyType notifyType = (pAccessUserPara->bMode==QUIT_REMOVE_MODE) ? 
							NOTIFY_TYPE_GRPMEM_QUIT : NOTIFY_TYPE_GRPMEM_REMOVE;
	string sGroupId = (pAccessUserPara->bMode==QUIT_REMOVE_MODE) ? 
					((GroupQuit*)pUserData)->sgroupid() :
					((GroupKickOut*)pUserData)->sgroupid();
	string sMsgId = (pAccessUserPara->bMode==QUIT_REMOVE_MODE) ? 
					((GroupQuit*)pUserData)->smsgid() :
					((GroupKickOut*)pUserData)->smsgid() ;
	string sManiUserId = (pAccessUserPara->bMode==QUIT_REMOVE_MODE) ? 
						((GroupKickOut*)pUserData)->suserid() : 
						((GroupKickOut*)pUserData)->skickid() ;
	string sUserId = (pAccessUserPara->bMode==QUIT_REMOVE_MODE) ? 
						((GroupQuit*)pUserData)->suserid() : 
						((GroupKickOut*)pUserData)->suserid() ;

	string sextend = (pAccessUserPara->bMode==QUIT_REMOVE_MODE) ? 
						((GroupQuit*)pUserData)->extend() : 
						((GroupKickOut*)pUserData)->extend() ;
						
	notify.set_smsgid(sMsgId);
	notify.set_sgrpid(sGroupId);
	notify.add_lstoid(sGroupId);
	notify.set_notifytype(notifyType);
	notify.set_sopruserid(sUserId);		
	notify.add_smnpleduserid(sManiUserId);	
	notify.set_extend(sextend);
	
	CGroupNotify::sendNotify(notify);
}

void  CGroupLeave::OnRemoveStartup(CPacket* pAccessPack,AccessUserPara_t* pAccessUserPara,ErrCode     errCode)
{
	if(!pAccessUserPara || !pAccessPack)
	{
		ErrLog("Access data is null when accessing group removing!");
		return;
	}

	
	void* pUserData = pAccessUserPara->pUserData;
	CGroupLeave * pLeavePacket = (CGroupLeave*)pAccessPack;

	
	(pAccessUserPara->bMode==QUIT_REMOVE_MODE) ? pLeavePacket->QuitRsp((GroupQuit*)pUserData,pAccessUserPara->sessionId,errCode) :
		pLeavePacket->KickoutRsp((GroupKickOut*)pUserData,pAccessUserPara->sessionId,errCode);
	
	if(errCode==NON_ERR)
	{
		pLeavePacket->SendNotify(pAccessUserPara);
	}
	
	if(pAccessUserPara->bMode==QUIT_REMOVE_MODE)
		delete (GroupQuit*) pAccessUserPara->pUserData ;
	else
		delete (GroupKickOut*)pAccessUserPara->pUserData;
	
	delete pAccessUserPara;					// Free user instance data.
}

bool CGroupLeave::RemoveStartup(void* pInstData,UidCode_t sessionId, uint8_t bMode)
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
	pAccessPara->bMode = bMode;
		

	sqlAccess.accessCb = AccessCallback(&CGroupLeave::OnRemoveStartup);
	sqlAccess.pAccessPacket = this;
	sqlAccess.pAccessPara = pAccessPara;
	sqlAccess.accessTask = REMOVE_GROUP_TASK;
									
	return DatabaseHelper(sqlAccess);

}


bool CGroupLeave::KickoutProc(GroupKickOut* pKickoutInst,UidCode_t sessionId)
{
	if(!RemoveStartup((void*)pKickoutInst,sessionId,KICKOUT_REMOVE_MODE))
	{
		ErrLog("Failed to start group kickout for the internal exception!");
		KickoutRsp(pKickoutInst,sessionId,ERR_GROUP_INTEREXCEPTION);
		return false;
	}
	InfoLog("Startup user %s group kickout ...",pKickoutInst->suserid().c_str());
	return true;	

}

bool CGroupLeave::QuitProc(GroupQuit* pQuitInst,UidCode_t sessionId)
{
	if(!RemoveStartup((void*)pQuitInst,sessionId,QUIT_REMOVE_MODE))
	{
		ErrLog("Failed to start group quit for the internal exception!");
		QuitRsp(pQuitInst,sessionId,ERR_GROUP_INTEREXCEPTION);
		return false;
	}
	InfoLog("Startup user %s group quiting ...",pQuitInst->suserid().c_str());
	return true;	

}


bool CGroupLeave::OnQuit(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("encounter a null pdu when processing group kickout!");
		return false;
	}
	
	GroupQuit* pQuitInst = new GroupQuit;
		
	UidCode_t sessionId = pPdu->GetSessionId();
	uchar_t* pContent = pPdu->GetBodyData();
	
	if(!pContent || !pQuitInst->ParseFromArray(pContent,pPdu->GetBodyLength())||
		pQuitInst->suserid().empty() || !IsGrpIdValid(pQuitInst->sgroupid()))
	{
		ErrLog("Group quit parameter error!");
		delete pQuitInst;
		
		return false;		
	}
	InfoLog("User %s group quit  %s with InstId = %d, cmd = 0x%x ,size = %d",pQuitInst->suserid().c_str(),
		pQuitInst->sgroupid().c_str(),m_nNumberOfInst,pPdu->GetCommandId(),pPdu->GetBodyLength());	

	return QuitProc(pQuitInst,sessionId);
	
}

bool CGroupLeave::OnKickout(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("encounter a null pdu when processing group kickout!");
		return false;
	}
	
	GroupKickOut* pKickoutInst = new GroupKickOut;
			
	UidCode_t sessionId = pPdu->GetSessionId();
	uchar_t* pContent = pPdu->GetBodyData();
	
	if(!pContent || !pKickoutInst->ParseFromArray(pContent,pPdu->GetBodyLength())||
		pKickoutInst->suserid().empty() || pKickoutInst->skickid().empty() ||
		!IsGrpIdValid(pKickoutInst->sgroupid()))
	{
		ErrLog("Group kickout parameter error!");
		delete pKickoutInst;
		
		return false;		

	}
	InfoLog("User %s group kickout %s from group %s with InstId = %d, cmd = 0x%x ,size = %d",pKickoutInst->suserid().c_str(),
		pKickoutInst->skickid().c_str(),pKickoutInst->sgroupid().c_str(),m_nNumberOfInst,pPdu->GetCommandId(),pPdu->GetBodyLength());	

	return KickoutProc(pKickoutInst,sessionId);

}

void CGroupLeave::KickoutRsp(GroupKickOut* pKickoutInst, UidCode_t sessionId,ErrCode bCode)
{
	if(!pKickoutInst)
	{
		ErrLog("Failed to response kickout for the kickout instance data is null!");
		return;
		
	}
	GroupKickoutAck kickoutAck;
	CImPdu	   	kickoutAckPdu;

	kickoutAck.set_suserid(pKickoutInst->suserid());
	kickoutAck.set_smsgid(pKickoutInst->smsgid());
	kickoutAck.set_errcode(bCode);

	kickoutAckPdu.SetPBMsg(&kickoutAck);
	kickoutAckPdu.SetCommandId(GROUP_KICKOUT_ACK);
	kickoutAckPdu.SetSessionId(sessionId);

	if(SendPdu(&kickoutAckPdu)<0)
	{
		ErrLog("Failed to response the user's %s kickout,perhaps the linker is lost！",
			pKickoutInst->suserid().c_str());
	}

}
void CGroupLeave::QuitRsp(GroupQuit* pQuitInst, UidCode_t sessionId,ErrCode bCode)
{
	if(!pQuitInst)
	{
		ErrLog("Failed to response quit for the quiting instance data is null!");
		return;
	}
	
	GroupQuitAck quitAck;
	CImPdu		quitAckPdu;

	quitAck.set_suserid(pQuitInst->suserid());
	quitAck.set_sgroupid(pQuitInst->sgroupid());
	quitAck.set_smsgid(pQuitInst->smsgid());
	quitAck.set_errcode(bCode);

	quitAckPdu.SetPBMsg(&quitAck);
	quitAckPdu.SetCommandId(GROUP_QUIT_ACK);
	quitAckPdu.SetSessionId(sessionId);

	if(SendPdu(&quitAckPdu)<0)
	{
		ErrLog("Failed to response the user's %s quit,perhaps the linker is lost！",
			pQuitInst->suserid().c_str());
	}

}


