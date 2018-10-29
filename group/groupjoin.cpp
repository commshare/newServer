#include "groupjoin.h"

CGroupJoin::CGroupJoin(CConfigFileReader* pConfigReader,int nNumOfInst)
	: m_pConfigReader(pConfigReader),m_nNumberOfInst(nNumOfInst)
{
}

CGroupJoin::~CGroupJoin()
{

}
	
bool CGroupJoin::Initialize(void)
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

bool CGroupJoin::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(GROUP_APPLY,m_nNumberOfInst, CommandProc(&CGroupJoin::OnApply));
	CmdRegist(GROUP_INVITE,m_nNumberOfInst, CommandProc(&CGroupJoin::OnInvite));
	CmdRegist(GROUP_PERMIT,m_nNumberOfInst, CommandProc(&CGroupJoin::OnPermit));	
	CmdRegist(MES_JOINGRP_ACK,m_nNumberOfInst, CommandProc(&CGroupJoin::OnJoinAck));
	
	return true;
}

int CGroupJoin::JoinReq(string sFromId,string sToId,string sGroupId,string sMsgId,uint8_t bMode, const string& strExtend)
{
	MESJoinGrp joinGroup;
	CImPdu	   joinGroupPdu;

	joinGroup.set_sfromid(sFromId);
	joinGroup.set_stoid(sToId);
	joinGroup.set_sgrpid(sGroupId);
	joinGroup.set_smsgid(sMsgId);
	joinGroup.set_reqtype(bMode);
	joinGroup.set_msgtime(get_tick_count());
	joinGroup.set_extend(strExtend);

	joinGroupPdu.SetPBMsg(&joinGroup);
	joinGroupPdu.SetCommandId(MES_JOINGRP);

	return SendPdu(MSG,&joinGroupPdu);
}

/*void CGroupJoin::SendNotify(AccessUserPara_t* pAccessUserPara,ErrCode errCode)
{
	if(!pAccessUserPara || !pAccessUserPara->pUserData)
		return;
	
	void* pUserData = pAccessUserPara->pUserData;

	string sToId;
	string sGroupId;
	string sMsgId;
	string sInviterId;
	string sInviteeId;

	_InnerGrpNotify notify;
	NotifyType notifyType;
	
	
	if(pAccessUserPara->bMode==APPLY_JOIN_MODE)
	{
		sToId = ((GroupApply*)pUserData)->suserid();
		sGroupId = ((GroupApply*)pUserData)->sgroupid();
		sMsgId = ((GroupApply*)pUserData)->smsgid();

		notifyType = NOTIFY_TYPE_GRPAPPLY_RESULT; 		
	}
	else if(pAccessUserPara->bMode==PERMIT_JOIN_MODE)
	{
		sToId = ((GroupPermit*)pUserData)->stoid();
		sGroupId = ((GroupPermit*)pUserData)->sgrpid();
		sMsgId = ((GroupPermit*)pUserData)->smsgid();	
		sInviterId =  ((GroupPermit*)pUserData)->stoid();
		sInviteeId =  ((GroupPermit*)pUserData)->sfromid();
		notifyType = !((GroupPermit*)pUserData)->rsptype() ? 
						NOTIFY_TYPE_GRPAPPLY_RESULT :
					 	NOTIFY_TYPE_GRPINVITE_RESULT;
		notify.set_sopruserid(sInviterId);
 
	}
	
	notify.set_smsgid(sMsgId);
	notify.set_sgrpid(sGroupId);
	notify.add_lstoid(sToId);
	notify.set_notifytype(notifyType);
	notify.set_errcode(errCode);

	CGroupNotify::sendNotify(notify);

}
*/
//通知申请方申请结果
void CGroupJoin::SendApplyNotify(GroupApply* pInstData,ErrCode errCode)
{
	if(!pInstData)
		return;
	_InnerGrpNotify notify;

	notify.set_smsgid(pInstData->smsgid());
	notify.set_sgrpid(pInstData->sgroupid());
	notify.add_lstoid(pInstData->suserid());
	notify.set_notifytype(NOTIFY_TYPE_GRPAPPLY_RESULT);
	notify.set_errcode(errCode);
	notify.set_extend(pInstData->extend());
	
	CGroupNotify::sendNotify(notify);
}


void CGroupJoin::SendGrpActiveNotify(AccessUserPara_t* pAccessUserPara)
{
	if (!pAccessUserPara || !pAccessUserPara->pUserData ||
		pAccessUserPara->bMode != APPLY_JOIN_MODE)
	{
		return;
	}
	void* pUserData = pAccessUserPara->pUserData;

	_InnerGrpNotify notify;


	notify.set_smsgid(((GroupApply*)pUserData)->smsgid());
	notify.set_sgrpid(((GroupApply*)pUserData)->sgroupid());
	notify.add_lstoid(pAccessUserPara->sMasterId);
	notify.set_notifytype(NOTIFY_TYPE_GRP_ACTIVE);
	notify.set_errcode(NON_ERR);
	notify.set_msgtime(getCurrentTime());
	notify.set_extend(((GroupApply*)pUserData)->extend());


	CGroupNotify::sendNotify(notify);

}

void CGroupJoin::SendPermitNotify(GroupPermit* pInstData,ErrCode errCode)
{
	if(!pInstData || pInstData->rsptype() == INVITE_JOIN_MODE)
	{
		WarnLog("Don't send this notify since the permission is anwsered by invitee ");
		return;
	}

	DbgLog("permission notity to %s from group %s,errcode=0x%x",pInstData->stoid().c_str(),
	pInstData->sgrpid().c_str(),errCode);
	_InnerGrpNotify notify;
	//NotifyType notifyType;

	notify.set_smsgid(pInstData->smsgid());
	notify.set_sgrpid(pInstData->sgrpid());
	notify.add_lstoid(pInstData->stoid());
	notify.set_notifytype(NOTIFY_TYPE_GRPAPPLY_RESULT);
	notify.set_errcode(errCode);
	notify.set_extend(pInstData->extend());
	
	CGroupNotify::sendNotify(notify);

}

void CGroupJoin::SendInviteNotifys(AccessUserPara_t* pAccessUserPara)
{
	if(!pAccessUserPara || !pAccessUserPara->pUserData || 
		pAccessUserPara->bMode != INVITE_JOIN_MODE || 
		!pAccessUserPara->InviteeUserMap.size())
	{
		return;
	}
	void* pUserData = pAccessUserPara->pUserData;
		
	_InnerGrpNotify notify;

	AccessUserMap_t::iterator it;
	string sUserId;
	uint8_t bPermit;

	
	notify.set_smsgid(((GroupInvite*)pUserData)->smsgid());
	notify.set_sgrpid(((GroupInvite*)pUserData)->sgroupid());
	notify.set_notifytype(NOTIFY_TYPE_GRPINVITE_RESULT);
	notify.set_sopruserid(((GroupInvite*)pUserData)->sinviterid());
	notify.set_errcode(NON_ERR);
	notify.set_extend(((GroupInvite*)pUserData)->extend());
	// only notify the invitee user list that no need permission. 
	for (it = pAccessUserPara->InviteeUserMap.begin(); it != pAccessUserPara->InviteeUserMap.end(); it++) 
	{
		sUserId = it->first;
		bPermit = (uint8_t)it->second;
		
		if(!bPermit)	// Means to ask the invitee user for permission
		{
			notify.add_lstoid(sUserId);
			notify.add_smnpleduserid(sUserId);
		}
	}
	
	CGroupNotify::sendNotify(notify);
}

void CGroupJoin::InviteeUserJoins(AccessUserPara_t* pAccessUserPara)
{
	if(!pAccessUserPara || !pAccessUserPara->pUserData || 
		pAccessUserPara->bMode != INVITE_JOIN_MODE || 
		!pAccessUserPara->InviteeUserMap.size())
	{
		DbgLog("Failed to invite user，Maybe the inviting data is null");
		return;
	}
		
	void* pUserData = pAccessUserPara->pUserData;
	AccessUserMap_t::iterator it;
	string sUserId;
	uint8_t bPermit;
	//string sToId;
	//string sFromId = ((GroupInvite*)pUserData)->suserid();
	//string sGroupId = ((GroupInvite*)pUserData)->suserid(); 
			
	for (it = pAccessUserPara->InviteeUserMap.begin(); it != pAccessUserPara->InviteeUserMap.end(); it++) 
	{
		sUserId = it->first;
		bPermit = (uint8_t)it->second;
		DbgLog("invite user %s with permission %d",sUserId.c_str(),bPermit);
		
		if(bPermit)	// Means to ask the invitee user for permission 
		{
			
			JoinReq(((GroupInvite*)pUserData)->sinviterid(),sUserId,  // F 
					 ((GroupInvite*)pUserData)->sgroupid(),
					((GroupInvite*)pUserData)->smsgid(),INVITE_JOIN_MODE, ((GroupInvite*)pUserData)->extend());		
		}
		
	}
}

void  CGroupJoin::OnJoinStartup(CPacket* pAccessPack,AccessUserPara_t* pAccessUserPara,ErrCode errCode)
{
	if(!pAccessUserPara || !pAccessPack)
	{
		ErrLog("Access data is null when accessing group creation!");
		return;
	}

	CGroupJoin * pJoinPacket = (CGroupJoin*)pAccessPack;
	void* pUserData = pAccessUserPara->pUserData;
	uint8_t bMode = pAccessUserPara->bMode;
	string sFromId;
	string sToId;

	switch(bMode)
	{
		case APPLY_JOIN_MODE:
			if(errCode==ERR_GROUP_INTERPERMISSION)
			{
				sFromId = ((GroupApply*)pUserData)->suserid();
				sToId = pAccessUserPara->sMasterId;
				
				pJoinPacket->JoinReq(sFromId,sToId, 
						((GroupApply*)pUserData)->sgroupid(),
						((GroupApply*)pUserData)->smsgid(),APPLY_JOIN_MODE, ((GroupApply*)pUserData)->extend());
				
				errCode = INFO_GROUP_PENDING;
				DbgLog("the user %s applied group %s is pending ,waiting for group master permission... ",
					sFromId.c_str(),((GroupApply*)pUserData)->sgroupid().c_str());
			}
			
			pJoinPacket->ApplyRsp((GroupApply*)pUserData,
						pAccessUserPara->sessionId,errCode);
			
			if(errCode==NON_ERR) // no error , means to apply successfully without applied confirmation.
			{
				DbgLog("the user %s applied group %s successfully! now sending notification ... ",
				sFromId.c_str(),((GroupApply*)pUserData)->sgroupid().c_str());
				pJoinPacket->SendApplyNotify((GroupApply*)pUserData,NON_ERR);

				if (NORMAL_GROUPSTATUS == pAccessUserPara->grpNewState)
				{
					pJoinPacket->SendGrpActiveNotify(pAccessUserPara);
				}
			}	
			
			delete (GroupApply*)pUserData;
			
			break;
		case INVITE_JOIN_MODE:
			if(errCode==ERR_GROUP_INVITESUCCESS)  // no error , means to invite successfully without confirmation required.
			{
				DbgLog("some invitee user has joined group %s successfully! now sending notification ... ",
				((GroupInvite*)pUserData)->sgroupid().c_str());				
				pJoinPacket->SendInviteNotifys(pAccessUserPara);
			}	
			
			pJoinPacket->InviteeUserJoins(pAccessUserPara);
			delete (GroupInvite*)pUserData;
			break;
		case PERMIT_JOIN_MODE:
			if(errCode==NON_ERR)
			{
				
				DbgLog("the invitee or applied user user %s,%s has accepted and joined group %s successfully!",
					((GroupPermit*)pUserData)->sfromid().c_str(),((GroupPermit*)pUserData)->stoid().c_str(),
					((GroupPermit*)pUserData)->sgrpid().c_str());

				pJoinPacket->SendPermitNotify((GroupPermit*)pUserData,NON_ERR);
			}
			log("send permitack user:%s, to: %s, group:%s, errcode:%d", ((GroupPermit*)pUserData)->sfromid().c_str(),((GroupPermit*)pUserData)->stoid().c_str(), ((GroupPermit*)pUserData)->sgrpid().c_str(), errCode);
			pJoinPacket->PermitRsp((GroupPermit*)pUserData,pAccessUserPara->sessionId,errCode); //response to remote user. 
			delete (GroupPermit*)pUserData;
			break;
		default:
			break;
	}

	delete pAccessUserPara;					// Free user instance data.
	
}

bool CGroupJoin::JoinStartup(void* pInstData,UidCode_t sessionId, uint8_t bMode)
{
	if(!pInstData)
	{
		ErrLog("The joining instance data should not be null!");
		return false;
	}
	
	AccessUserPara_t* pAccessPara =  new AccessUserPara_t;	
	pAccessPara->InviteeUserMap.clear();
	pAccessPara->pUserData = (void*)pInstData;
	pAccessPara->sessionId = sessionId;
	pAccessPara->bMode = bMode;
	pAccessPara->InviteeUserMap.clear();
	
	SqlAccess_t sqlAccess;
	sqlAccess.accessCb = AccessCallback(&CGroupJoin::OnJoinStartup);
	sqlAccess.pAccessPacket = this;
	sqlAccess.pAccessPara = pAccessPara;
	sqlAccess.accessTask = (bMode==PERMIT_JOIN_MODE) ? DIRECTLYJOIN_GROUP_TASK:JOIN_GROUP_TASK;
	
									
	return DatabaseHelper(sqlAccess);

}


bool CGroupJoin::ApplyProc(GroupApply* pApplyInst,UidCode_t sessionId)
{
	if(!JoinStartup((void*)pApplyInst,sessionId,APPLY_JOIN_MODE))
	{
		ErrLog("Failed to start apply group joining for the internal exception!");
		ApplyRsp(pApplyInst,sessionId,ERR_GROUP_INTEREXCEPTION);
		return false;
	}
	InfoLog("Startup user %s group applied join ...",pApplyInst->suserid().c_str());
	return true;	
}


bool CGroupJoin::InviteProc(GroupInvite* pInviteInst,UidCode_t sessionId)
{
	if(!JoinStartup((void*)pInviteInst,sessionId,INVITE_JOIN_MODE))
	{
		ErrLog("Failed to start invite group joining for the internal exception!");
		InviteRsp(pInviteInst,sessionId,ERR_GROUP_INTEREXCEPTION);
		return false;
	}
	InfoLog("Startup user %s invite group joining ...",pInviteInst->sinviterid().c_str());
	InviteRsp(pInviteInst,sessionId,INFO_GROUP_PENDING);
	
	return true;	

}

bool CGroupJoin::PermitProc(GroupPermit* pPermitInst,UidCode_t sessionId)
{
	//DbgLog("user %s permit status code = %d",pPermitInst->sfromid().c_str(),pPermitInst->errcode());
	if(pPermitInst->errcode() == NON_ERR) //Remote user has aleady accept the request
	{
		if(!JoinStartup((void*)pPermitInst,sessionId,PERMIT_JOIN_MODE))
		{
			ErrLog("Failed to start permiting group joining for the internal exception!");
			PermitRsp(pPermitInst,sessionId,ERR_GROUP_INTEREXCEPTION); // response to remote user. 
			
			return false;
		}
		InfoLog("Startup user %s group permiting join ...",pPermitInst->sfromid().c_str());
		return true;
	}
	else if(pPermitInst->errcode() == ERR_GROUP_JOIN_REJECT) // Notify the result to originator. 
	{
		SendPermitNotify(pPermitInst,ERR_GROUP_NOTIFY_REJECT); // only applying for a group.
		PermitRsp(pPermitInst,sessionId,NON_ERR); // response to remote user. 
	}
	return false;
}


bool CGroupJoin::OnApply(std::shared_ptr<CImPdu> pPdu) 	
{
	if(!pPdu)
	{
		ErrLog("encounter a null pdu when processing group apply!");
		return false;
	}
	
	GroupApply* pApplyInst = new GroupApply;
		
	UidCode_t sessionId = pPdu->GetSessionId();
	uchar_t* pContent = pPdu->GetBodyData();
	
	if(!pContent || !pApplyInst->ParseFromArray(pContent,pPdu->GetBodyLength()) ||
	    pApplyInst->suserid().empty() || !IsGrpIdValid(pApplyInst->sgroupid()))
	{
		ErrLog("Group apply parameter error!");
		delete pApplyInst;
		
		return false;		
	}
	InfoLog("User %s group applying %s with InstId = %d, cmd = 0x%x ,size = %d",pApplyInst->suserid().c_str(),
		pApplyInst->sgroupid().c_str(),m_nNumberOfInst,	pPdu->GetCommandId(),pPdu->GetBodyLength());	

	return ApplyProc(pApplyInst, sessionId);
	
	/*nResult= IsMember(pApplyInst->sgroupid(),pApplyInst->suserid())
	if(!nResult)
	{
		if(VerifyGroupJoinAuthoriry(pApplyInst->sgroupid(),pApplyInst->suserid())) //Need admin permission
		{
			if(JoinReq(string sFromId, string sToId, string sGroupId))
		}
		else
		{
			ApplyProc(pApplyInst,sessionId);
		}
		
				
	}
	else if(nResult)
	{
		ApplyRsp()
		return false;
	}
	else if(nResult<0)
	{
		ApplyRsp();
		return false;
	}
	*/	
	
}

bool CGroupJoin::OnInvite(std::shared_ptr<CImPdu> pPdu) 
{
	if(!pPdu)
	{
		ErrLog("encounter a null pdu when processing group invite!");
		return false;
	}
	
	GroupInvite* pInviteInst = new GroupInvite;
	UidCode_t sessionId = pPdu->GetSessionId();
	uchar_t* pContent = pPdu->GetBodyData();
	
	if(!pContent || !pInviteInst->ParseFromArray(pContent,pPdu->GetBodyLength())||
		pInviteInst->sinviterid().empty() || !IsGrpIdValid(pInviteInst->sgroupid()))
	{
		ErrLog("Group invite parameter error!");
		delete pInviteInst;
		
		return false;		
	}
	InfoLog("User %s group invite %s with InstId = %d, cmd = 0x%x ,size = %d",pInviteInst->sinviterid().c_str(),
		pInviteInst->sgroupid().c_str(),m_nNumberOfInst,pPdu->GetCommandId(),pPdu->GetBodyLength());	

	return InviteProc(pInviteInst, sessionId);
}

bool CGroupJoin::OnPermit(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("encounter a null pdu when processing group join permission!");
		return false;
	}

	GroupPermit* pPermitInst = new GroupPermit;
	UidCode_t sessionId = pPdu->GetSessionId();
	uchar_t* pContent = pPdu->GetBodyData();
	
	if(!pContent || !pPermitInst->ParseFromArray(pContent,pPdu->GetBodyLength())||
		pPermitInst->sfromid().empty() || !IsGrpIdValid(pPermitInst->sgrpid()))
	{
		ErrLog("Group permit parameter error!");
		delete pPermitInst;
		
		return false;		
	}
	InfoLog("User %s group permission %s with InstId = %d, cmd = 0x%x ,size = %d",pPermitInst->sfromid().c_str(),
		pPermitInst->sgrpid().c_str(),m_nNumberOfInst,pPdu->GetCommandId(),pPdu->GetBodyLength());	

	return PermitProc(pPermitInst, sessionId);	

}


bool CGroupJoin::OnJoinAck(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("encounter a null pdu when processing group Join ack!");
		return false;
	}
	
	return true;
}

void CGroupJoin::ApplyRsp(GroupApply* pInstData,UidCode_t sessionId,ErrCode errCode)
{
	if(!pInstData)
	{
		ErrLog("Failed to response apply for the applying instance data is null!");
		return;
	}

	GroupApplyAck applyAck;
	CImPdu	   	applyAckPdu;

	applyAck.set_suserid(pInstData->suserid());
	applyAck.set_sgroupid(pInstData->sgroupid());
	applyAck.set_smsgid(pInstData->smsgid());
	applyAck.set_errcode(errCode);

	applyAckPdu.SetPBMsg(&applyAck);
	applyAckPdu.SetCommandId(GROUP_APPLY_ACK);
	applyAckPdu.SetSessionId(sessionId);

	if(SendPdu(&applyAckPdu)<0)
	{
		ErrLog("Failed to response the user's %s apply,perhaps the linker is lost！",
			pInstData->suserid().c_str());	
	}
}
void CGroupJoin::InviteRsp(GroupInvite* pInstData,UidCode_t sessionId,ErrCode errCode)
{
	if(!pInstData)
	{
		ErrLog("Failed to response invite for the inviting instance data is null!");
		return;
	}

	GroupInviteAck inviteAck;
	CImPdu	   	inviteAckPdu;

	inviteAck.set_suserid(pInstData->sinviterid());
	inviteAck.set_sgroupid(pInstData->sgroupid());
	inviteAck.set_smsgid(pInstData->smsgid());
	inviteAck.set_errcode(errCode);

	inviteAckPdu.SetPBMsg(&inviteAck);
	inviteAckPdu.SetCommandId(GROUP_INVITE_ACK);
	inviteAckPdu.SetSessionId(sessionId);

	if(SendPdu(&inviteAckPdu)<0)
	{
		ErrLog("Failed to response the user's %s invite,perhaps the linker is lost！",
			pInstData->sinviterid().c_str());
	}
}

void CGroupJoin::PermitRsp(GroupPermit* pInstData,UidCode_t sessionId, ErrCode errCode)
{
	if(!pInstData)
	{
		ErrLog("Failed to response permit for the permiting instance data is null!");
		return;
			
	}

	DbgLog("permitack %s,%s, errcode:%d",pInstData->sfromid().c_str(),pInstData->sgrpid().c_str(), errCode);

	GroupPermitAck permitAck;
	CImPdu	   	permitAckPdu;

	permitAck.set_suserid(pInstData->sfromid());
	permitAck.set_sgrpid(pInstData->sgrpid());
	permitAck.set_smsgid(pInstData->smsgid());
	permitAck.set_errcode(errCode);

	permitAckPdu.SetPBMsg(&permitAck);
	permitAckPdu.SetCommandId(GROUP_PERMIT_ACK);
	permitAckPdu.SetSessionId(sessionId);

	if(SendPdu(&permitAckPdu)<0)
	{
		ErrLog("Failed to response the permission,perhaps the linker is lost！")	;
	}
		
}

	
