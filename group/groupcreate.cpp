#include "groupcreate.h"

CGroupCreate::CGroupCreate(CConfigFileReader* pConfigReader,int nNumOfInst)
	: m_pConfigReader(pConfigReader),m_nNumberOfInst(nNumOfInst)
{
}

CGroupCreate::~CGroupCreate()
{

}
	
bool CGroupCreate::Initialize(CConfigFileReader* pConfigReader)
{
	if(!CDbHelper::Initialize(pConfigReader))
	{
		ErrLog("Failed to initialize database...");
		return false;
	}

	RegistPacketExecutor();
	StartThread();
	
	return true;
}

bool CGroupCreate::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(GROUP_CREATE,m_nNumberOfInst,CommandProc(&CGroupCreate::OnCreate));
	
	return true;
}
//
void CGroupCreate::SendNotify(AccessUserPara_t* pAccessUserPara)
{
	if(!pAccessUserPara || !pAccessUserPara->pUserData || 
		!pAccessUserPara->InviteeUserMap.size())
	{
		return;
	}
	GroupCreate* pUserData = (GroupCreate*)pAccessUserPara->pUserData;
	_InnerGrpNotify notify;
	AccessUserMap_t::iterator it;
	string sUserId;
	uint8_t bPermit;


	notify.set_smsgid(pUserData->smsgid());
	notify.set_sgrpid(pAccessUserPara->sGroupId);
	notify.set_notifytype(NOTIFY_TYPE_GRPINVITE_RESULT);
	notify.set_sopruserid(pUserData->suserid());
	notify.set_errcode(NON_ERR);
	notify.set_extend(pUserData->extend());
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
	log("------------------send notify to group member--------------------------");
}
void CGroupCreate::InviteeUserJoins(AccessUserPara_t* pAccessUserPara)
{
	if(!pAccessUserPara || !pAccessUserPara->pUserData || 
		!pAccessUserPara->InviteeUserMap.size())
	{
		WarnLog("No any invitee user when successfully creating a new group %s,Number of Invitee ：%d",
			pAccessUserPara->sGroupId.c_str(),pAccessUserPara->InviteeUserMap.size());

		return;
	}
		
	void* pUserData = pAccessUserPara->pUserData;
	AccessUserMap_t::iterator it;
	string sUserId;
	uint8_t bPermit;

	//DbgLog("Invitee join user total size = %d",pAccessUserPara->InviteeUserMap.size());
			
	for (it = pAccessUserPara->InviteeUserMap.begin(); it != pAccessUserPara->InviteeUserMap.end(); it++) 
	{
		sUserId = it->first;
		bPermit = (uint8_t)it->second;

		if(bPermit)	// Means to ask the invitee user for permission 
		{
			log("group create add memeber permit group:%s  user:%s  status:%d", pAccessUserPara->sGroupId.c_str(), sUserId.c_str(), bPermit);
			JoinReq(((GroupCreate*)pUserData)->suserid(),sUserId,  // F 
					pAccessUserPara->sGroupId,	
					((GroupCreate*)pUserData)->smsgid(),INVITE_JOIN_MODE, ((GroupCreate*)pUserData)->extend());		
		}
		
	}
}

// 59dc81c58fcb7c28c31171c
// 59dc9632c3ea96ce39d7a958
int CGroupCreate::JoinReq(string sFromId,string sToId,string sGroupId,string sMsgId,uint8_t bMode, const string& strExtend)
{
	MESJoinGrp joinGroup;
	CImPdu	   joinGroupPdu;

	joinGroup.set_sfromid(sFromId);
	joinGroup.set_stoid(sToId);
	joinGroup.set_sgrpid(sGroupId);
	joinGroup.set_smsgid(sMsgId);
	joinGroup.set_reqtype(bMode);
    joinGroup.set_msgtime(get_tick_count());//set server time
	joinGroup.set_extend(strExtend);

	joinGroupPdu.SetPBMsg(&joinGroup);
	joinGroupPdu.SetCommandId(MES_JOINGRP);

	log("-----------send group member join request--------------------");
	return SendPdu(MSG,&joinGroupPdu);
}

void  CGroupCreate::OnCreateStartup(CPacket* pAccessPack,AccessUserPara_t* pAccessUserPara,ErrCode errCode)
{
	if(!pAccessUserPara || !pAccessPack)
	{
		ErrLog("Access data is null when accessing group creation!");
		return;
	}

	
	GroupCreate* pCreateInstData = (GroupCreate*)pAccessUserPara->pUserData;
	CGroupCreate * pCreatePacket = (CGroupCreate*)pAccessPack;

	uint64_t timeStamp = getCurrentTime();

			
	if((pCreateInstData->ncreatetype()==NORMAL_CREATION || pCreateInstData->ncreatetype()==NORMAL_CREATIONWITHHIDE ) 
		&& (errCode == im::ERR_GROUP_INVITEEXCEPTION ||	errCode == im::ERR_GROUP_INVITESUCCESS))
	{
		DbgLog("create group success,the result is : 0x%x",errCode);
		pCreatePacket->CreateRsp(pCreateInstData,pAccessUserPara->sGroupId,pAccessUserPara->sessionId,NON_ERR,timeStamp); 
		if(errCode==ERR_GROUP_INVITESUCCESS)
		{
			pCreatePacket->SendNotify(pAccessUserPara);
		}	
		pCreatePacket->InviteeUserJoins(pAccessUserPara);
		errCode = NON_ERR;    // Means group creation is success whether or not inviting exception 
	}
	else
	{
		pCreatePacket->CreateRsp(pCreateInstData,pAccessUserPara->sGroupId,pAccessUserPara->sessionId,errCode,timeStamp); 
	}

		
	if(errCode!=NON_ERR)
	{
		ErrLog("Encounter exception error 0x%x when user is creating a new group!",
			errCode,pCreateInstData->suserid().c_str());
	}
	else
	{
		InfoLog("User %s has created a group %s in successful!",
			pCreateInstData->suserid().c_str(), pAccessUserPara->sGroupId.c_str());
	}
			
	
	delete (GroupCreate*)pAccessUserPara->pUserData;
	delete pAccessUserPara;					// Free user instance data.
	
}

bool CGroupCreate::CreateStartup(GroupCreate *pCreateInstData,UidCode_t sessionId)
{
	SqlAccess_t sqlAccess;

	AccessUserPara_t* pAccessPara =  new AccessUserPara_t;
	pAccessPara->InviteeUserMap.clear();
	pAccessPara->pUserData = (void*)pCreateInstData;
	pAccessPara->sGroupId = "";
	pAccessPara->sessionId = sessionId;
	
	sqlAccess.accessCb = AccessCallback(&CGroupCreate::OnCreateStartup);
	sqlAccess.pAccessPacket = this;
	sqlAccess.pAccessPara = pAccessPara;
	sqlAccess.accessTask = CREATE_GROUP_TASK;
	
	return DatabaseHelper(sqlAccess);
}


bool CGroupCreate::CreateProc(GroupCreate* pCreateInstData,UidCode_t sessionId) 
{

	if(!CreateStartup(pCreateInstData,sessionId))
	{
		ErrLog("Failed to start group create for the internal exception!");
		CreateRsp(pCreateInstData,"",sessionId,ERR_GROUP_INTEREXCEPTION);
		return false;
	}
	//InfoLog("Startup user %s group creation...",pCreateInstData->suserid().c_str());
	return true;
}

bool CGroupCreate::OnCreate(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("encounter a null pdu when processing group creation!");
		return false;
	}

	GroupCreate* pCreateInst = new GroupCreate;
	UidCode_t sessionId = pPdu->GetSessionId();
	uchar_t* pContent = pPdu->GetBodyData();

	if(!pContent || !pCreateInst->ParseFromArray(pContent,pPdu->GetBodyLength()))
	{
		ErrLog("Group create parameter error!");
		delete pCreateInst;
		
		return false;		
	}
	InfoLog("User %s group creation requiring with InstId = %d, cmd = 0x%x ,size = %d ",pCreateInst->suserid().c_str(),
		m_nNumberOfInst,pPdu->GetCommandId(),pPdu->GetBodyLength());	
	
	return CreateProc(pCreateInst, sessionId);

}


void CGroupCreate::CreateRsp(GroupCreate* pCreateInst, string sGroupId,UidCode_t sessionId,ErrCode bCode,uint64_t ackTime)
{
	if(!pCreateInst)
	{
		ErrLog("Failed to response creation for the creating instance data is null!");
		return;
	}

	GroupCreateAck 	createAck;
	CImPdu 		createAckPdu;

	createAck.set_suserid(pCreateInst->suserid());
	createAck.set_smsgid(pCreateInst->smsgid());
	createAck.set_msgtime(ackTime);
	
	if(bCode==NON_ERR)
		createAck.set_sgroupid(sGroupId);
	
	createAck.set_errcode(bCode);
	
	createAckPdu.SetPBMsg(&createAck);
	createAckPdu.SetCommandId(GROUP_CREATE_ACK);
	createAckPdu.SetSessionId(sessionId);

	//DbgLog("createack, session=%x%x%x%x%x%x%x%x%x%x%x%x",
	//sessionId.Uid_Item.code[0],sessionId.Uid_Item.code[1],
	//sessionId.Uid_Item.code[2],sessionId.Uid_Item.code[3],sessionId.Uid_Item.code[4],
	//sessionId.Uid_Item.code[5],sessionId.Uid_Item.code[6],sessionId.Uid_Item.code[7],
	//sessionId.Uid_Item.code[8],sessionId.Uid_Item.code[9],sessionId.Uid_Item.code[10],
	//sessionId.Uid_Item.code[11]);

		
	if(SendPdu(&createAckPdu)<0)
	{
		ErrLog("Failed to response the user's %s create,perhaps the linker is lost！",
			pCreateInst->suserid().c_str());
	}
	log("----------------------------create group send ack-------------------------------------");
}



