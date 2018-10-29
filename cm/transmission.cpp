#include "clientlinkmgr.h"
#include "transmission.h"
#include "usermanage.h"
#include "im.pub.pb.h"
#include "im.mes.pb.h"
#include "im.group.pb.h"
#include "im.sig.pb.h"
//extern string gSessionTestId;

CTransimission::CTransimission(int nNumOfInst)
	: m_nNumberOfInst(nNumOfInst)
{
}

CTransimission::~CTransimission()
{

}

bool CTransimission::Initialize(void)
{
	//Add your statment to load config parameter about user manage.

	//

	RegistPacketExecutor();
	StartThread();
	return true;
}


bool CTransimission::RegistPacketExecutor(void)
{
	//////////////////////////////////////////////////////////////////////////////////////
	// 						Regist chat cmd for forwarding to MSG.
	/////////////////////////////////////////////////////////////////////////////////////
	CmdRegist(MES_CHAT,m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_CHAT_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_CHAT_READ,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_OFFLINESUMMARY,  m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_OFFLINEMSG, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_OFFLINETOTAL,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_OFFLINEMSG_DELIVERED, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_CHAT_READ_DELIVER_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_OFFLINEMSG_DELIVERED_NOTIFICATION_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_CHAT_DELIVERED_NOTIFICATION_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_DECBLACKLIST,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_INCBLACKLIST,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_ADDFRIEND,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_ADDFRIEND_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_ADDFRIEND_ANS,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_ADDFRIEND_ANS_DELIVER_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_DELFRIEND,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	//CmdRegist(MES_JOINGRP_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));	
	//CmdRegist(MES_JOINGRP_ANS, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));	
	CmdRegist(MES_EXCHANGE_KEY,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));	
	CmdRegist(MES_EXCHANGE_KEY_DELIVER_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_EXCHANGE_KEY_DELIVERD_NOTIFY_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_GRPNOTIFY_DELIVER_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_GRPCHAT_DELIVER_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_CHATCANCEL,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	//CmdRegist(MES_CHATCANCEL_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_CHATCANCEL_DELIVER_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_JOINGRP_DELIVER_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	
	CmdRegist(SIG_SPONSORP2PCALL, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(SIG_SPONSORP2PCALL_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(SIG_SPONSORP2PCALL_ANS, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(SIG_SPONSORP2PCALL_ANS_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(SIG_P2PCALL_EXCHANGE_NATINFO, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	//CmdRegist(SIG_P2PCALL_EXCHANGE_NATINFO_DELIVERD_NOTIFY_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans)); È¡ÏûÁË¶ÔÓ¦µÄÃüÁî×Ö
	CmdRegist(SIG_P2PCALLHANGUP, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(SIG_P2PCALLHANGUPDElIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));

	CmdRegist(SIG_P2PCALLSTATENOTIFY, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(SIG_P2PCALLSTATENOTIFYDElIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));

	CmdRegist(im::MS_COMMONNOTIFY, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(im::MS_COMMONNOTIFY_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	//////////////////////////////////////////////////////////////////////////////////////
	// 						Regist chat cmd for forwarding to Group.
	/////////////////////////////////////////////////////////////////////////////////////
	CmdRegist(GROUP_CHAT, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));	
	CmdRegist(GROUP_CREATE,m_nNumberOfInst,  CommandProc(&CTransimission::OnGroupTrans));	
	CmdRegist(GROUP_APPLY, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));	
	CmdRegist(GROUP_INVITE,m_nNumberOfInst,  CommandProc(&CTransimission::OnGroupTrans));	
	CmdRegist(GROUP_KICKOUT,m_nNumberOfInst,  CommandProc(&CTransimission::OnGroupTrans));	
	CmdRegist(GROUP_QUIT, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));	
	CmdRegist(GROUP_MODIFY, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));	
	CmdRegist(GROUP_PERMIT, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));	
	CmdRegist(GROUP_CHATCANCEL, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));
	//////////////////////////////////////////////////////////////////////////////////////
	// 						Regist chat cmd for forwarding to client.
	/////////////////////////////////////////////////////////////////////////////////////	
	CmdRegist(MES_CHAT_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_CHAT_READ_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_OFFLINESUMMARY_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_OFFLINETOTAL_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_OFFLINEMSG_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_OFFLINEMSG_DELIVERED_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_OFFLINEMSG_DELIVERED_NOTIFICATION, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_DECBLACKLIST_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_CHAT_DELIVERED_NOTIFICATION,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_INCBLACKLIST_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_ADDFRIEND_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_ADDFRIEND_ANS_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_ADDFRIEND_ANS_DELIVER,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_DELFRIEND_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(CM_KICKOUT_NOTIFICATION,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_EXCHANGE_KEY_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));	
	CmdRegist(MES_CHATCANCEL_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));		
	CmdRegist(GROUP_CHAT_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(GROUP_CREATE_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(GROUP_APPLY_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(GROUP_INVITE_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(GROUP_KICKOUT_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(GROUP_QUIT_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(GROUP_PERMIT_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(GROUP_MODIFY_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(GROUP_CHATCANCEL_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));	
	CmdRegist(SIG_SPONSORP2PCALL_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(SIG_SPONSORP2PCALL_ANS_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(SIG_P2PCALL_EXCHANGE_NATINFO_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(SIG_P2PCALLHANGUP_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(SIG_P2PCALLSTATENOTIFY_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));

	CmdRegist(im::MS_COMMONNOTIFY_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	//////////////////////////////////////////////////////////////////////////////////////
	// 						Regist chat cmd for exanging to client.
	/////////////////////////////////////////////////////////////////////////////////////	
	CmdRegist(MES_CHAT_DELIVER,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientExange));
	CmdRegist(MES_CHAT_READ_DELIVER,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientExange));
	CmdRegist(MES_ADDFRIEND_DELIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	CmdRegist(MES_EXCHANGE_KEY_DELIVER,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientExange));
	CmdRegist(MES_EXCHANGE_KEY_DELIVERD_NOTIFY,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientExange));
	CmdRegist(MES_GRPNOTIFY_DELIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	CmdRegist(MES_GRPCHAT_DELIVER,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientExange));
	CmdRegist(MES_CHATCANCEL_DELIVER,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientExange));
	CmdRegist(MES_JOINGRP_DELIVER,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientExange));
	
	CmdRegist(SIG_SPONSORP2PCALL_DELIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	CmdRegist(SIG_SPONSORP2PCALL_ANS_DELIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	CmdRegist(SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	CmdRegist(SIG_P2PCALLHANGUPDElIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	CmdRegist(SIG_P2PCALLSTATENOTIFYDElIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	
	CmdRegist(im::MS_COMMONNOTIFY_DELIVER,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientExange));
	return true;
}

bool CTransimission::GetUserId(std::shared_ptr<CImPdu> pPdu,string &sUserId)
{
	if(!pPdu)
		return false;

	bool bRet = true;
	uint16_t nCmdId = pPdu->GetCommandId(); 
	uint16_t nBodySize = pPdu->GetBodyLength();
	char* pContent =  (char*)pPdu->GetBodyData();
	switch (nCmdId)
	{
		case MES_CHAT_ACK:
		{
			MESChatAck chatAck;
						
			if(false==chatAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += chatAck.suserid();
		}
		break;
		case MES_CHAT_DELIVER:
		{
			MESChat chat;
			//string msgContent="";
			
			if(false==chat.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += chat.stoid();
		}
		break;
		case MES_CHAT_DELIVERED_NOTIFICATION:
		{
			MESChatDeliveredAck chatDeliveredAck;
			//string msgContent="";
			
			if(false==chatDeliveredAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += chatDeliveredAck.stoid();
		}
		break;		
		case MES_CHAT_READ_ACK:
		{
			MESChatReadAck chatReadAck;
			//string msgContent="";
			if(false==chatReadAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += chatReadAck.suserid();
		}
		break;	
		case MES_CHAT_READ_DELIVER:
		{
			MESChatRead chatReadDeliver;
						
			if(false==chatReadDeliver.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += chatReadDeliver.stoid();
		}
		break;
		case MES_OFFLINESUMMARY_ACK:
		{
			MESOfflineSummaryAck offlineSummaryAck;
						
			if(false==offlineSummaryAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += offlineSummaryAck.suserid();
		}
		break;		
		case MES_OFFLINETOTAL_ACK:
		{
			MESOfflineSummaryAck offlineTotalAck;
						
			if(false==offlineTotalAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += offlineTotalAck.suserid();
		}
		break;				
		case MES_OFFLINEMSG_ACK:
		{
			MESOfflineMsgAck offlineMsgAck;
			if(false==offlineMsgAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += offlineMsgAck.stoid();
		}
		break;				
		case MES_OFFLINEMSG_DELIVERED_ACK:
		{
			MESOfflineMsgDelivereddAck offlineMsgDeliverdAck;
						
			if(false==offlineMsgDeliverdAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += offlineMsgDeliverdAck.suserid();
		}
		break;		
		case MES_OFFLINEMSG_DELIVERED_NOTIFICATION:
		{
			MESOfflineMsgDelivered offlineMsgDeliverdNotify;
						
			if(false==offlineMsgDeliverdNotify.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += offlineMsgDeliverdNotify.stoid();
		}
		break;		
		case MES_ADDFRIEND_ACK:
		{
			MESAddFriendAck friendAck;
						
			if(false==friendAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += friendAck.suserid();
		}
		break;			
		case MES_ADDFRIEND_DELIVER:
		{
			MESAddFriend AddFriendDeliver;
						
			if(false==AddFriendDeliver.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += AddFriendDeliver.stoid();
		}
		break;					
		case MES_ADDFRIEND_ANS_ACK:
		{
			MESAddFriendAnsAck friendAnsAck;
						
			if(false==friendAnsAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += friendAnsAck.suserid();
		}
		break;	
		case MES_ADDFRIEND_ANS_DELIVER:
		{
			MESAddFriendAns friendAnsDeliver;
						
			if(false==friendAnsDeliver.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += friendAnsDeliver.stoid();
		}
		break;			
		case MES_DELFRIEND_ACK:
		{
			MESDelFriendAck delFriendAck;
						
			if(false==delFriendAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += delFriendAck.suserid();
		}
		break;		
		case MES_INCBLACKLIST_ACK:
		{
			MESIncBlacklistAck incBlackFriendAck;
						
			if(false==incBlackFriendAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += incBlackFriendAck.suserid();
		}
		break;	
		case MES_DECBLACKLIST_ACK:
		{
			MESDecBlacklistAck decBlackFriendAck;
						
			if(false==decBlackFriendAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += decBlackFriendAck.suserid();
		}
		break;						
		case CM_KICKOUT_NOTIFICATION:
		{
			CMKickoutNotification kickoutNotify;
						
			if(false==kickoutNotify.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += kickoutNotify.suserid();
		}
		break;
		/*case MES_JOINGRP_ANS_ACK:
		{
			MESJoinGrpAnsAck JoinGrpAnsAck;
						
			if(false==JoinGrpAnsAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += JoinGrpAnsAck.suserid();
		}
		break;
		*/
		case MES_JOINGRP_DELIVER:
		{
			MESJoinGrp joinGrpDeliver;
						
			if(false==joinGrpDeliver.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += joinGrpDeliver.stoid();
		}
		break;
		case MES_EXCHANGE_KEY_ACK:
		{
			MESExchangeKeyAck exchangeKeyAck;
						
			if(false==exchangeKeyAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += exchangeKeyAck.suserid();
		}
		break;		
		case MES_EXCHANGE_KEY_DELIVER:
		{
			MESExchangeKeyDeliver exchangeKeyDeliver;
						
			if(false==exchangeKeyDeliver.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += exchangeKeyDeliver.stoid();
		}
		break;
		case MES_EXCHANGE_KEY_DELIVERD_NOTIFY:
		{
			MESExchangeKeyDeliverAck exchangeKeyDeliverNotify;
						
			if(false==exchangeKeyDeliverNotify.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += exchangeKeyDeliverNotify.stoid();
		}
		break;
		case MES_GRPNOTIFY_DELIVER:
		{
			MESGrpNotify grpNotifyDeliver;
						
			if(false==grpNotifyDeliver.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpNotifyDeliver.stoid();
		}
		break;
		case MES_GRPCHAT_DELIVER:
		{
			MESGrpChat grpChatDeliver;
						
			if(false==grpChatDeliver.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpChatDeliver.stoid();
		}
		break;
		case MES_CHATCANCEL_ACK:
		{
			MESChatCancelAck chatCancelAck;
						
			if(false==chatCancelAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += chatCancelAck.suserid();
		}
		break;
		case MES_CHATCANCEL_DELIVER:
		{
			MESChatCancel chatCancel;
						
			if(false==chatCancel.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += chatCancel.stoid();
		}
		break;
		case GROUP_CHAT_ACK:
		{
			GroupChatAck grpChatAck;
						
			if(false==grpChatAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpChatAck.sfromid();
		}
		break;
		case GROUP_CREATE_ACK:
		{
			GroupCreateAck grpCreateAck;
						
			if(false==grpCreateAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpCreateAck.suserid();
		}
		break;		
		case GROUP_APPLY_ACK:
		{
			GroupApplyAck grpApplyAck;
						
			if(false==grpApplyAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpApplyAck.suserid();
		}
		break;				
		case GROUP_INVITE_ACK:
		{
			GroupInviteAck grpInviteAck;
						
			if(false==grpInviteAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpInviteAck.suserid();
		}
		break;		
		case GROUP_KICKOUT_ACK:
		{
			GroupKickoutAck grpKickoutAck;
						
			if(false==grpKickoutAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpKickoutAck.suserid();
		}
		break;			
		case GROUP_QUIT_ACK:
		{
			GroupQuitAck grpQuitAck;
						
			if(false==grpQuitAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpQuitAck.suserid();
		}
		break;					
		case GROUP_MODIFY_ACK:
		{
			GroupModifyAck grpModifyAck;
						
			if(false==grpModifyAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpModifyAck.suserid();
		}
		break;
		case GROUP_PERMIT_ACK:
		{
			GroupPermitAck grpPermitAck;
						
			if(false==grpPermitAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpPermitAck.suserid();
		}
		break;	
		case GROUP_CHATCANCEL_ACK:
		{
			GroupChatCancelAck grpChatCancelAck;
						
			if(false==grpChatCancelAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += grpChatCancelAck.sfromid();
		}
		break;
		case SIG_SPONSORP2PCALL_ACK:
		{
			SIGSponsorCallAck ack;

			if (false == ack.ParseFromArray(pContent, nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += ack.stoid();
		}
			break;
		case SIG_SPONSORP2PCALL_DELIVER:
		{
			SIGSponsorCall deliver;

			if (false == deliver.ParseFromArray(pContent, nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += deliver.sinviteid();
		}
			break;
		case SIG_SPONSORP2PCALL_ANS_ACK:
		{
			SIGSponsorCallAnsAck ack;

			if (false == ack.ParseFromArray(pContent, nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += ack.stoid();
		}
			break;
		case SIG_SPONSORP2PCALL_ANS_DELIVER:
		{
			SIGSponsorCallAns deliver;

			if (false == deliver.ParseFromArray(pContent, nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += deliver.stoid();
		}
			break;
		case SIG_P2PCALL_EXCHANGE_NATINFO_ACK:
		{
			SIGP2PCallExchangeNatInfoAck ack;

			if (false == ack.ParseFromArray(pContent, nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += ack.stoid();
		}
			break;
		case SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER:
		{
			SIGP2PCallExchangeNatInfo deliver;

			if (false == deliver.ParseFromArray(pContent, nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += deliver.stoid();
		}
			break;
		case SIG_P2PCALLHANGUP_ACK:
		{
			SIGHangUpAck ack;

			if (false == ack.ParseFromArray(pContent, nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += ack.stoid();
		}
			break;
		case SIG_P2PCALLHANGUPDElIVER:
		{
			SIGHangUp deliver;

			if (false == deliver.ParseFromArray(pContent, nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += deliver.stoid();
		}
			break;	
		case SIG_P2PCALLSTATENOTIFY_ACK:
		{
			SIGP2PCallStateNotifyACK ack;

			if (false == ack.ParseFromArray(pContent, nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += ack.stoid();
		}
			break;
		case SIG_P2PCALLSTATENOTIFYDElIVER:
		{
			SIGP2PCallStateNotify deliver;

			if (false == deliver.ParseFromArray(pContent, nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += deliver.stoid();
		}
			break;
		// Abner 2018-08-20
		case im::MS_COMMONNOTIFY_ACK:
		{
			im::MSGCommonNotifyACK commNotifyAck;
													
			if(false==commNotifyAck.ParseFromArray(pContent,nBodySize))
		 	{
				bRet = false;
		 		break;
			}

			sUserId += commNotifyAck.suserid();
		}
			break;
		case im::MS_COMMONNOTIFY_DELIVER:
		{
			im::MSGCommonNotify commonNotifyDeliver;
						
			if(false==commonNotifyDeliver.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += commonNotifyDeliver.stoid();
		}
			break;

		default:
			bRet = false;
			ErrLog("There is no any of valid command packet ");
			break;
	}
	
	return bRet;
}


bool CTransimission::OnClientTrans(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
		return false;
	
	string sUserId=PREFIX_CM_STR;

	if(!GetUserId(pPdu, sUserId))
	{
		ErrLog("can't parse and extract from cmdId 0x%x the remote user id to be transmited.",pPdu->GetCommandId());
		return false;
	}

	DbgLog("transmitting the packet = 0x%x to user %s ...",pPdu->GetCommandId(),sUserId.c_str());
	if(SendPdu(sUserId,pPdu.get())==-1)
	{
		ErrLog("The target link is not exist ,Maybe the target user %s never login !",sUserId.c_str());
		return false;
	}

	if(CM_KICKOUT_NOTIFICATION == pPdu->GetCommandId())
	{
		CClientLink* pKickoutLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId.c_str());
		if(pKickoutLink)
		{
			DbgLog("Close user %s link after notified user",sUserId.c_str());
			//CAutoLock autolock(CClientLinkMgr::GetInstance()->GetCliLock());
			pKickoutLink->CloseLink(); //Close the old link since it has aleady kickout. 
			pKickoutLink->ReleaseRef();
		}
	}
		
	return true;
}
bool CTransimission::OnClientExange(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
		return false;
	
	string sUserId=PREFIX_CM_STR;


	if(!GetUserId(pPdu, sUserId))
	{
		ErrLog("can't parse and extract user id to be exange.");
		return false;
	}
	DbgLog("exchanging the packet = 0x%x to user %s ...",pPdu->GetCommandId(),sUserId.c_str());
	if(SendPdu(sUserId,pPdu.get())==-1)
	{
		ErrLog("The target link is not exist ,Maybe the target user %s never login !",sUserId.c_str());
		return false;
	}
		
	return true;	
}

bool CTransimission::OnMsgTrans(std::shared_ptr<CImPdu> pPdu)
{

	if(!pPdu)
		return false;
	
	
	UidCode_t sessionId = pPdu->GetSessionId();
	#if 0	
	const char* str1 = sSessionId.c_str();
	const char* str2 = gSessionTestId.c_str();
	char buf1[256] = {0};
	char buf2[256] = {0};
	sprintf(buf1,"0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x",str1[0],str1[1],str1[2],str1[3],str1[4],str1[5],str1[6],str1[7],str1[8],str1[9],str1[10],str1[11],str1[12],str1[13],str1[14]);
	sprintf(buf2,"0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x",str2[0],str2[1],str2[2],str2[3],str2[4],str2[5],str2[6],str2[7],str2[8],str2[9],str2[10],str2[11],str2[12],str2[13],str2[14]);
		
	DbgLog("msg str: %d,%d",sSessionId.size(),gSessionTestId.size());
	DbgLog("buf1:%s",buf1);
	DbgLog("buf2:%s",buf2);
	#endif	

	CClientLink* pCurLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
	
	if(!pCurLink || (pCurLink->GetUserId()==""))
	{
		if( pPdu->GetCommandId() == MES_CHAT )
		{
			MESChat msg;
			msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
		
			DbgLog("userID[%s] sessionId=[%x%x%x%x%x%x%x%x%x%x%x%x] link is invalid",msg.sfromid().c_str(),
				sessionId.Uid_Item.code[0],
				sessionId.Uid_Item.code[1],
				sessionId.Uid_Item.code[2],
				sessionId.Uid_Item.code[3],
				sessionId.Uid_Item.code[4],
				sessionId.Uid_Item.code[5],
				sessionId.Uid_Item.code[6],
				sessionId.Uid_Item.code[7],
				sessionId.Uid_Item.code[8],
				sessionId.Uid_Item.code[9],
				sessionId.Uid_Item.code[10],
				sessionId.Uid_Item.code[11]);
		}
		
		ErrLog("The source link is invalid or the user is not login ,so can't tranmit the pdu to msg server!");
		if(pCurLink){ pCurLink->ReleaseRef();}
		return false;
	}

	if(pCurLink){pCurLink->ReleaseRef();}
	InfoLog("transmitting the packet = 0x%x to msg svr...",pPdu->GetCommandId());
	if(SendPdu(MSG, pPdu.get()) <= 0)
	{
		ErrLog("Failed to send the packet 0x%x ,Maybe the linker between CM and MSG server is lost...",pPdu->GetCommandId());
		return false;
	}
	
	return true;	
}

bool CTransimission::OnGroupTrans(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
		return false;
	
	
	UidCode_t sessionId = pPdu->GetSessionId();

	CClientLink* pCurLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
	DbgLog("Group trans, cmd = 0x%x,link=%p",pPdu->GetCommandId(),pCurLink);
	if(!pCurLink || (pCurLink->GetUserId()==""))
	{	
		if( pPdu->GetCommandId() == GROUP_CHAT )
		{
			GroupChat msg;
			msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
		
			DbgLog("userID[%s] sessionId=[%x%x%x%x%x%x%x%x%x%x%x%x] link is invalid",msg.sfromid().c_str(),
				sessionId.Uid_Item.code[0],
				sessionId.Uid_Item.code[1],
				sessionId.Uid_Item.code[2],
				sessionId.Uid_Item.code[3],
				sessionId.Uid_Item.code[4],
				sessionId.Uid_Item.code[5],
				sessionId.Uid_Item.code[6],
				sessionId.Uid_Item.code[7],
				sessionId.Uid_Item.code[8],
				sessionId.Uid_Item.code[9],
				sessionId.Uid_Item.code[10],
				sessionId.Uid_Item.code[11]);
		}
		
		ErrLog("The source link is invalid or the user is not login ,so can't tranmit the pdu to group server!");
		if(pCurLink){ pCurLink->ReleaseRef();}
		return false;
	}

	if(pCurLink){pCurLink->ReleaseRef();}
	InfoLog("transmitting the packet = 0x%x to group svr...",pPdu->GetCommandId());
	if(SendPdu(GROUP, pPdu.get()) <= 0)
	{
		ErrLog("Failed to send the packet 0x%x ,Maybe the linker between CM and GROUP server is lost...",pPdu->GetCommandId());
		return false;
	}


	return true;
}




