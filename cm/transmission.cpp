#include "clientlinkmgr.h"
#include "transmission.h"
#include "im.pub.pb.h"
#include "im.mes.pb.h"
//#include "im.group.pb.h"
#include "im.sig.pb.h"
#include "im.cm.pb.h"
#include "im.inner.pb.h"
#include "im.desktop.pb.h"
#define PREFIX_CM_STR "CM_"					//user login cached information.
#define PREFIX_CMDT_STR "CMDT_"				//the logged in user device information. 
#define POSTFIX_CM_POS	3
#define PREFIX_DEVICELEN_REF 5

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

	CmdRegist(im::SYSTEM_TIMEOUT_NOTIFICATION, m_nNumberOfInst, CommandProc(&CTransimission::OnCloseLink));
	CmdRegist(im::CM_LOGIN_NOTIFY_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnCmLoginNotifyAck));
	//////////////////////////////////////////////////////////////////////////////////////
	// 						Regist chat cmd for forwarding to LOGIN.
	/////////////////////////////////////////////////////////////////////////////////////
	CmdRegist(im::CM_LOGIN,m_nNumberOfInst, CommandProc(&CTransimission::OnLoginTrans));
	CmdRegist(im::CM_LOGOUT,m_nNumberOfInst, CommandProc(&CTransimission::OnLoginTrans));
//	CmdRegist(im::CM_DEVICETOKENSYNC,m_nNumberOfInst, CommandProc(&CTransimission::OnLoginTrans));
	CmdRegist(im::LOGIN_CM_NOTIFY, m_nNumberOfInst,CommandProc(&CTransimission::OnLoginCMNotify));
	CmdRegist(im::SVR_LOGIN_RESULT,m_nNumberOfInst,  CommandProc(&CTransimission::OnLoginResult));

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
//	CmdRegist(MES_DECBLACKLIST,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
//	CmdRegist(MES_INCBLACKLIST,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
//	CmdRegist(MES_ADDFRIEND,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_ADDFRIEND_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
//	CmdRegist(MES_ADDFRIEND_ANS,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(MES_ADDFRIEND_ANS_DELIVER_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
//	CmdRegist(MES_DELFRIEND,m_nNumberOfInst,  CommandProc(&CTransimission::OnMsgTrans));
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

//	CmdRegist(im::MS_COMMONNOTIFY, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(im::MS_COMMONNOTIFY_DELIVER_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(im::GROUP_CHAT, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	CmdRegist(im::GROUP_CHATCANCEL, m_nNumberOfInst, CommandProc(&CTransimission::OnMsgTrans));
	//////////////////////////////////////////////////////////////////////////////////////
	// 						Regist chat cmd for forwarding to Group.
	/////////////////////////////////////////////////////////////////////////////////////
//	CmdRegist(GROUP_CHAT, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));	
//	CmdRegist(GROUP_CREATE,m_nNumberOfInst,  CommandProc(&CTransimission::OnGroupTrans));	
//	CmdRegist(GROUP_APPLY, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));	
//	CmdRegist(GROUP_INVITE,m_nNumberOfInst,  CommandProc(&CTransimission::OnGroupTrans));	
//	CmdRegist(GROUP_KICKOUT,m_nNumberOfInst,  CommandProc(&CTransimission::OnGroupTrans));	
//	CmdRegist(GROUP_QUIT, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));	
//	CmdRegist(GROUP_MODIFY, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));	
//	CmdRegist(GROUP_PERMIT, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));	
//	CmdRegist(GROUP_CHATCANCEL, m_nNumberOfInst, CommandProc(&CTransimission::OnGroupTrans));
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
//	CmdRegist(MES_ADDFRIEND_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(MES_ADDFRIEND_ANS_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_ADDFRIEND_ANS_DELIVER,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(MES_DELFRIEND_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(im::CM_KICKOUT_NOTIFICATION,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(MES_EXCHANGE_KEY_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));	
	CmdRegist(MES_CHATCANCEL_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));		
	CmdRegist(GROUP_CHAT_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(GROUP_CREATE_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(GROUP_APPLY_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(GROUP_INVITE_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(GROUP_KICKOUT_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(GROUP_QUIT_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(GROUP_PERMIT_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(GROUP_MODIFY_ACK,m_nNumberOfInst,  CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(GROUP_CHATCANCEL_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));	
	CmdRegist(SIG_SPONSORP2PCALL_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(SIG_SPONSORP2PCALL_ANS_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(SIG_P2PCALL_EXCHANGE_NATINFO_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(SIG_P2PCALLHANGUP_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(SIG_P2PCALLSTATENOTIFY_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));

//	CmdRegist(im::MS_COMMONNOTIFY_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(im::CM_LOGIN_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(im::CM_LOGOUT_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
//	CmdRegist(im::CM_DEVICETOKENSYNC_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
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

	// 转发channel服务器数据
	CmdRegist(im::RADIO_CHAT, m_nNumberOfInst, CommandProc(&CTransimission::OnChannelTrans));
	CmdRegist(im::RADIO_CHAT_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(im::RADIO_CHAT_DELIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
    CmdRegist(im::RADIO_ADMIN_CANCEL_CHAT, m_nNumberOfInst, CommandProc(&CTransimission::OnChannelTrans));
    CmdRegist(im::RADIO_ADMIN_CANCEL_CHAT_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
    CmdRegist(im::RADIO_ADMIN_CANCEL_CHAT_DELIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
    CmdRegist(im::RADIO_NOTIFY_DELIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	CmdRegist(im::RADIO_CANCEL_CHAT, m_nNumberOfInst, CommandProc(&CTransimission::OnChannelTrans));
	CmdRegist(im::RADIO_CANCEL_CHAT_ACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientTrans));
	CmdRegist(im::RADIO_CANCEL_CHAT_DELIVER, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));

    //transfer to desktopGW
	CmdRegist(im::PC_GWSYNMESSAGETOAPP, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	CmdRegist(im::PC_GWSYNMESSAGETOAPPACK, m_nNumberOfInst, CommandProc(&CTransimission::OnDesktopTrans));

	CmdRegist(im::PC_APPSYNMESSAGETOGWACK, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	CmdRegist(im::PC_APPSYNMESSAGETOGW , m_nNumberOfInst, CommandProc(&CTransimission::OnDesktopTrans));
	
	CmdRegist(im::PC_CHECKAPPACTIVE, m_nNumberOfInst, CommandProc(&CTransimission::OnClientExange));
	CmdRegist(im::PC_CHECKAPPACTIVEACK, m_nNumberOfInst, CommandProc(&CTransimission::OnDesktopTrans));
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
		case im::CM_KICKOUT_NOTIFICATION:
		{
			im::CMKickoutNotification kickoutNotify;
						
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
			DbgLog("group chat user id %s", grpChatDeliver.stoid().c_str());
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
#if 0
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
#endif
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
//		case im::MS_COMMONNOTIFY_ACK:
//		{
//			im::MSGCommonNotifyACK commNotifyAck;
//													
//			if(false==commNotifyAck.ParseFromArray(pContent,nBodySize))
//		 	{
//				bRet = false;
//		 		break;
//			}

//			sUserId += commNotifyAck.suserid();
//		}
//			break;
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
		case im::CM_LOGIN_ACK:
		{
			im::CMLoginAck loginAck;
			if(false==loginAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			
			sUserId += loginAck.suserid();
		}
			break;
		case im::CM_LOGOUT_ACK:
		{
			im::CMLogoutAck logoutAck;
						
			if(false==logoutAck.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}

			sUserId += logoutAck.suserid();
		}
			break;
//		case im::CM_DEVICETOKENSYNC_ACK:
//		{
//			im::CMDeviceTokenSyncAck syncAck;
						
//			if(false==syncAck.ParseFromArray(pContent,nBodySize))
//			{
//				bRet = false;
//				break;
//			}

//			sUserId += syncAck.suserid();
//		}
//			break;
		case im::RADIO_CHAT_ACK:
		case im::RADIO_ADMIN_CANCEL_CHAT_ACK:
		case im::RADIO_CANCEL_CHAT_ACK:
		{
			im::RadioChatAck msg;
			if(false==msg.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			sUserId += msg.sfromid();
		}
		break;
		case im::RADIO_CHAT_DELIVER:
		case im::RADIO_NOTIFY_DELIVER:
		{
			im::RadioChat msg;
			if(false==msg.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			sUserId += msg.stoid();
		}
		break;
		case im::RADIO_ADMIN_CANCEL_CHAT_DELIVER:
		case im::RADIO_CANCEL_CHAT_DELIVER:
		{
			im::RadioCancelChat msg;
			if(false==msg.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			sUserId += msg.stoid();
		}
		break;
        //for desktop
		case im::PC_GWSYNMESSAGETOAPP :
		{
			im::GwSynMessageToApp msg;
			if(false==msg.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			sUserId += msg.stoid();
        }
        break;
		case im::PC_APPSYNMESSAGETOGWACK:
		{
			im::AppSynMessageToGwAck msg;
			if(false==msg.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			sUserId += msg.sfromid();
		}
		break;
		case im::PC_CHECKAPPACTIVE:
		{
			im::CheckAppActive msg;
			if(false==msg.ParseFromArray(pContent,nBodySize))
			{
				bRet = false;
				break;
			}
			sUserId += msg.sfromid();
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
	bool bRest = false;
	string sUserId=PREFIX_CM_STR;
	do{
		if(nullptr == pPdu)
			break;
		if(!GetUserId(pPdu, sUserId))
		{
			ErrLog("can't parse and extract from cmdId 0x%x the remote user id to be transmited.",pPdu->GetCommandId());
			break;
		}
		
		DbgLog("transmitting the packet = 0x%x to user %s ...",pPdu->GetCommandId(),sUserId.c_str());
		if(SendPdu(sUserId,pPdu.get())==-1)
		{
			ErrLog("The target link is not exist ,Maybe the target user %s never login !",sUserId.c_str());
			break;
		}

		if(im::CM_LOGOUT_ACK == pPdu->GetCommandId())
		{
			
			CClientLink* pKickoutLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId);
			if(pKickoutLink)
			{
				DbgLog("Close user %s link after notified user",sUserId.c_str());
				pKickoutLink->CloseLink(); //Close the old link since it has aleady kickout. 
				pKickoutLink->ReleaseRef();
			}
		}
		
	}while(0);

	return bRest;
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
	if(SendPdu(sUserId,pPdu.get())== -1)
	{
		ErrLog("The target link is not exist ,Maybe the target user %s never login !",sUserId.c_str());
		return false;
	}
		
	return true;	
}

bool CTransimission::OnLoginTrans(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
		return false;
	UidCode_t sessionId = pPdu->GetSessionId();
	if(pPdu->GetCommandId() == im::CM_LOGIN)
	{
		// 获取userid, 添加到userid链路管理
		im::CMLoginTrans loginTrans;
		bool bRest = loginTrans.ParseFromArray(pPdu->GetBodyData(),pPdu->GetBodyLength());
		DbgLog("parse login_protocol code: %d, userid: %s", bRest, loginTrans.suserid().c_str());
		//string sUserId = PREFIX_CM_STR + loginTrans.suserid();
		//UpdateUserLink(sessionId,sUserId);
		//DbgLog("add link to useLinkmap %s", sUserId.c_str());
		string sIP = "";
		uint16_t nPort = 0;
		CClientLinkMgr::GetInstance()->GetLocalHost(sIP,nPort);
		loginTrans.set_sloginip(sIP);
		loginTrans.set_nloginport(nPort);

		CClientLink* pCurLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
		if(pCurLink)
		{
			string strIP = pCurLink->GetPeerIp();
			uint16_t port = pCurLink->GetPeerPort();
			string strHost = strIP + ":" + std::to_string(port);
			loginTrans.set_shost(strHost);
			pCurLink->ReleaseRef();
			DbgLog("device token=%s userId=%s host=%s session=%x%x%x%x%x%x%x%x%x%x%x%x", loginTrans.sdevicetoken().c_str(), loginTrans.suserid().c_str(), strHost.c_str(), 
																							sessionId.Uid_Item.code[0], sessionId.Uid_Item.code[1], sessionId.Uid_Item.code[2], 
																							sessionId.Uid_Item.code[3], sessionId.Uid_Item.code[4], sessionId.Uid_Item.code[5], 
																							sessionId.Uid_Item.code[6], sessionId.Uid_Item.code[7], sessionId.Uid_Item.code[8], 
																							sessionId.Uid_Item.code[9], sessionId.Uid_Item.code[10],sessionId.Uid_Item.code[11]);
		}

		// 转换cm与login之间的通讯命令
		pPdu->SetCommandId(im::CM_LOGIN_TRANS);
		pPdu->SetPBMsg(&loginTrans);
		//im::CMLogin login;
		//bool bRest = login.ParseFromArray(pPdu->GetBodyData(),pPdu->GetBodyLength());
		//DbgLog("parse login_protocol code: %d, userid: %s", bRest, login.suserid().c_str());
		//string sUserId = PREFIX_CM_STR + login.suserid();
		//UpdateUserLink(sessionId,sUserId);
		//DbgLog("add link to useLinkmap %s", sUserId.c_str());
		// 转换cm与login之间的通讯命令
		//pPdu->SetCommandId(im::CM_LOGIN_TRANS);
	}
	else
	{
		DbgLog("login server trans, cmd = 0x%x",pPdu->GetCommandId());
		CClientLink* pCurLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
		if(!pCurLink || (pCurLink->GetUserId()==""))
		{
			ErrLog("The source link is invalid or the user is not login ,so can't tranmit the pdu to login server!");
			if(pCurLink){ pCurLink->ReleaseRef();}
			return false;
		}
		if(pCurLink){pCurLink->ReleaseRef();}
	}
	
	InfoLog("transmitting the packet = 0x%x to login svr...",pPdu->GetCommandId());
	if(SendPdu(LOGIN, pPdu.get()) <= 0)
	{
		ErrLog("Failed to send the packet 0x%x ,Maybe the linker between CM and login server is lost...",pPdu->GetCommandId());
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
		if( pPdu->GetCommandId() == MES_CHAT)
		{
			MESChat msg;
			msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
		
			DbgLog("userID[%s] msgid=%s sessionId=[%x%x%x%x%x%x%x%x%x%x%x%x] link is invalid",msg.sfromid().c_str(), msg.smsgid().c_str(),
														sessionId.Uid_Item.code[0], sessionId.Uid_Item.code[1], sessionId.Uid_Item.code[2],
														sessionId.Uid_Item.code[3], sessionId.Uid_Item.code[4], sessionId.Uid_Item.code[5],
														sessionId.Uid_Item.code[6], sessionId.Uid_Item.code[7], sessionId.Uid_Item.code[8],
														sessionId.Uid_Item.code[9], sessionId.Uid_Item.code[10], sessionId.Uid_Item.code[11]);
		}
		else if( pPdu->GetCommandId() == GROUP_CHAT )
		{
			MESGrpChat msg;
			msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
								
		DbgLog("userID=%s groupid=%s msgId=%s sessionId=[%x%x%x%x%x%x%x%x%x%x%x%x] link is invalid",msg.sfromid().c_str(), msg.sgrpid().c_str(), msg.smsgid().c_str(),
																sessionId.Uid_Item.code[0],sessionId.Uid_Item.code[1],sessionId.Uid_Item.code[2],
																sessionId.Uid_Item.code[3], sessionId.Uid_Item.code[4], sessionId.Uid_Item.code[5],
																sessionId.Uid_Item.code[6], sessionId.Uid_Item.code[7], sessionId.Uid_Item.code[8],
																sessionId.Uid_Item.code[9],	sessionId.Uid_Item.code[10], sessionId.Uid_Item.code[11]);
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
			MESGrpChat msg;
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
#if 1
bool CTransimission::OnDesktopTrans(std::shared_ptr<CImPdu> pPdu)
{

	if(!pPdu)
		return false;
	
	UidCode_t sessionId = pPdu->GetSessionId();

	CClientLink* pCurLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
	DbgLog("DesktopGw trans, cmd = 0x%x,link=%p",pPdu->GetCommandId(),pCurLink);
	if(!pCurLink || (pCurLink->GetUserId()==""))
	{	
#if 0
        if( pPdu->GetCommandId() == GROUP_CHAT )
		{
			MESGrpChat msg;
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
#endif	
		ErrLog("The source link is invalid or the user is not login ,so can't tranmit the pdu to desktop server!");
		if(pCurLink){ pCurLink->ReleaseRef();}
		return false;
	}

	if(pCurLink){pCurLink->ReleaseRef();}
	InfoLog("transmitting the packet = 0x%x to DESKTOP svr...",pPdu->GetCommandId());
	if(BroadcastPdu(DESKTOP, pPdu.get()) <= 0)
	{
		ErrLog("Failed to send the packet 0x%x ,Maybe the linker between CM and DESTKTOP server is lost...",pPdu->GetCommandId());
		return false;
	}

	return true;
}
#endif

bool CTransimission::OnChannelTrans(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
		return false;
	
	
	UidCode_t sessionId = pPdu->GetSessionId();

	CClientLink* pCurLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
	
	if(!pCurLink || (pCurLink->GetUserId()==""))
	{
		if( pPdu->GetCommandId() == RADIO_CHAT)
		{
			RadioChat msg;
			msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
		
			DbgLog("userID[%s] msgid=%s sessionId=[%x%x%x%x%x%x%x%x%x%x%x%x] link is invalid",msg.sfromid().c_str(), msg.smsgid().c_str(),
														sessionId.Uid_Item.code[0], sessionId.Uid_Item.code[1], sessionId.Uid_Item.code[2],
														sessionId.Uid_Item.code[3], sessionId.Uid_Item.code[4], sessionId.Uid_Item.code[5],
														sessionId.Uid_Item.code[6], sessionId.Uid_Item.code[7], sessionId.Uid_Item.code[8],
														sessionId.Uid_Item.code[9], sessionId.Uid_Item.code[10], sessionId.Uid_Item.code[11]);
		}
		ErrLog("The source link is invalid or the user is not login ,so can't tranmit the pdu to msg server!");
		if(pCurLink){ pCurLink->ReleaseRef();}
		return false;
	}

	if(pCurLink){pCurLink->ReleaseRef();}
	InfoLog("transmitting the packet = 0x%x to channel svr...",pPdu->GetCommandId());
	if(SendPdu(CHANNEL, pPdu.get()) <= 0)
	{
		ErrLog("Failed to send the packet 0x%x ,Maybe the linker between CM and CHANNEL server is lost...",pPdu->GetCommandId());
		return false;
	}
	
	return true;	
}


void CTransimission::UpdateUserLink(UidCode_t currentSessionId,string sUserId)
{
	CClientLink* pLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(currentSessionId);
	if(!pLink)
	{
		ErrLog("get link by session id fail session=%x%x%x%x%x%x%x%x%x%x%x%x user=%s", currentSessionId.Uid_Item.code[0], currentSessionId.Uid_Item.code[1],
			currentSessionId.Uid_Item.code[2], currentSessionId.Uid_Item.code[3], currentSessionId.Uid_Item.code[4],
			currentSessionId.Uid_Item.code[5], currentSessionId.Uid_Item.code[6], currentSessionId.Uid_Item.code[7],
			currentSessionId.Uid_Item.code[8], currentSessionId.Uid_Item.code[9], currentSessionId.Uid_Item.code[10], 
			currentSessionId.Uid_Item.code[11], sUserId.c_str());
		return;
	}

	// 关闭之前登陆的链接 
	CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId);
	if(pUserLink && pUserLink != pLink)
	{
		DbgLog("close old link! user=%s ip=%s port=%d", sUserId.c_str(), pUserLink->GetPeerIp().c_str(), pUserLink->GetPeerPort());
		pUserLink->CloseLink(true);
		pUserLink->ReleaseRef();
	}
	DbgLog("new link info! user=%s ip=%s port=%d", sUserId.c_str(), pLink->GetPeerIp().c_str(), pLink->GetPeerPort());
	pLink->UserArrived(sUserId);
	pLink->ReleaseRef();
}

bool CTransimission::OnCloseLink(std::shared_ptr<CImPdu> pPdu)
{
	if(!pPdu)
	{
		ErrLog("Recv a null pdu when processing user overtime event!");
		return false;
	}
	
	UidCode_t sessionId = pPdu->GetSessionId();
	CClientLink* pSessionLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
	if(!pSessionLink)
	{
		ErrLog("Find a invalid link when processing link close event");
		return false;
	}

	string strIP = pSessionLink->GetPeerIp();
	uint16_t port = pSessionLink->GetPeerPort();
	string strHost = strIP + ":" + std::to_string(port);
		
	string sUserId = pSessionLink->GetUserId();
	CClientLink* pUserLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId);
	bool bUserLink = (pSessionLink==pUserLink); 
	DbgLog("Close link, bUserLink=%d,sessionLink=%p,userLink=%p, user %s ip=%s port=%d",bUserLink,pSessionLink,pUserLink, sUserId.c_str(), pSessionLink->GetPeerIp().c_str(), pSessionLink->GetPeerPort());
	if(pSessionLink)
	{	
		pSessionLink->CloseLink(bUserLink);
		pSessionLink->ReleaseRef();
	}
	
	if(pUserLink)
		pUserLink->ReleaseRef();

	if(!bUserLink)		// 当前存在user链路（另一条），不需要设置redis数据状态
		return true;

	string sIP = "";
	uint16_t nPort = 0;
	CClientLinkMgr::GetInstance()->GetLocalHost(sIP,nPort);
	im::CMLoginNotify cmloginNotify;
	cmloginNotify.set_suserid(sUserId.substr(POSTFIX_CM_POS));
	cmloginNotify.set_sip(sIP);
	cmloginNotify.set_nport(nPort);

	cmloginNotify.set_shost(strHost);
	cmloginNotify.set_notifytype(im::CMLOGIN_CLOSELINK);

	CImPdu cmloginNotifyPDU;
	cmloginNotifyPDU.SetPBMsg(&cmloginNotify);
	cmloginNotifyPDU.SetCommandId(im::CM_LOGIN_NOTIFY);
	cmloginNotifyPDU.SetSessionId(sessionId);

	if(SendPdu(LOGIN, &cmloginNotifyPDU) <= 0)
	{
		ErrLog("Failed to send the packet 0x%x ,Maybe the linker between CM and login server is lost...",cmloginNotifyPDU.GetCommandId());
		return false;
	}
			
	return true; 
}

bool CTransimission::OnCmLoginNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	return true;
}

bool CTransimission::OnLoginCMNotify(std::shared_ptr<CImPdu> pPdu)
{
	im::LoginCMNotify cmNotify;
	uchar_t* pContent = pPdu->GetBodyData();
	if (!pContent || !cmNotify.ParseFromArray(pContent, pPdu->GetBodyLength()))
	{
		ErrLog("LoginCMNotify parameter error!");
		return false;
	}
	std::string sUserId = PREFIX_CM_STR + cmNotify.suserid();
	CClientLink* pKickoutLink = CClientLinkMgr::GetInstance()->GetLinkByUserId(sUserId);			
	if(pKickoutLink)
	{
		DbgLog("Close user %s link after notified user",sUserId.c_str());
		pKickoutLink->CloseLink(); //Close the old link since it has aleady kickout. 
		pKickoutLink->ReleaseRef();
	}
	
	im::LoginCMNotifyAck notifyAck;
	notifyAck.set_suserid(cmNotify.suserid());
	notifyAck.set_nerr(im::NON_ERR);
	CImPdu notifyAckPdu;
	notifyAckPdu.SetPBMsg(&notifyAck);
	notifyAckPdu.SetCommandId(im::CM_LOGIN_NOTIFY);
	notifyAckPdu.SetSessionId(pPdu->GetSessionId());
	if(SendPdu(LOGIN, &notifyAckPdu) <= 0)
	{
		ErrLog("OnLoginCMNotify Failed to send the packet 0x%x ,Maybe the linker between CM and login server is lost...", im::CM_LOGIN_NOTIFY);
		return false;
	}
    InfoLog("ready to send NotifyPCToLineOff");
    if(NotifyPCToLineOff(pPdu))
    {
        return false;
    }
    //BroadcastPdu(DESKTOP, pPdu.get());
	return true;
}

bool CTransimission::OnLoginResult(std::shared_ptr<CImPdu> pPdu)
{
	im::OnLoginResult LoginRes;
	uchar_t* pContent = pPdu->GetBodyData();
	if (!pContent || !LoginRes.ParseFromArray(pContent, pPdu->GetBodyLength()))
	{
		ErrLog("LoginCMNotify parameter error!");
		return false;
	}
	UidCode_t sessionId = CClientLinkMgr::GetInstance()->getSessionByHost(LoginRes.shost());
	DbgLog("login result user id %s host=%s  session=%x%x%x%x%x%x%x%x%x%x%x%x", LoginRes.suserid().c_str(), LoginRes.shost().c_str(), 
								sessionId.Uid_Item.code[0], sessionId.Uid_Item.code[1], sessionId.Uid_Item.code[2], 
								sessionId.Uid_Item.code[3], sessionId.Uid_Item.code[4], sessionId.Uid_Item.code[5], 
								sessionId.Uid_Item.code[6], sessionId.Uid_Item.code[7], sessionId.Uid_Item.code[8], 
								sessionId.Uid_Item.code[9], sessionId.Uid_Item.code[10], sessionId.Uid_Item.code[11]);
	bool bCloseLink = true;
	if(im::LOGIN_ACK == LoginRes.type())
	{
		DbgLog("login ack user id %s code %d", LoginRes.suserid().c_str(), LoginRes.nerr());
		if(im::NON_ERR == LoginRes.nerr())
		{
			bCloseLink = false;
			string sUserId = PREFIX_CM_STR + LoginRes.suserid();
			UpdateUserLink(sessionId,sUserId);
			DbgLog("add link to useLinkmap %s", sUserId.c_str());
		}

		im::CMLoginAck 	loginAck;
		loginAck.set_suserid(LoginRes.suserid());
		loginAck.set_nerr(LoginRes.nerr());
		loginAck.set_nlastlogintime(getCurrentTime());
		sendAck(&loginAck, im::CM_LOGIN_ACK, sessionId);
	}
	else if(im::LOGIN_KICKOUT == LoginRes.type())
	{
		DbgLog("kickout userid %s at cm server ...",LoginRes.suserid().c_str());
		uint16_t nPort = 0;
		string strIp = "";
		CClientLinkMgr::GetInstance()->GetLocalHost(strIp, nPort);
		im::CMKickoutNotification kickoutUser;
		kickoutUser.set_suserid(LoginRes.suserid());
		kickoutUser.set_ip(strIp);
		kickoutUser.set_port(nPort);
		sendAck(&kickoutUser, im::CM_KICKOUT_NOTIFICATION, sessionId);
        NotifyPCToLineOff(pPdu);//
        //BroadcastPdu(DESKTOP, pPdu.get());
	}
	
	if(bCloseLink)
	{
		CClientLink* pKickoutLink = CClientLinkMgr::GetInstance()->GetLinkBySessionId(sessionId);
		if(pKickoutLink)
		{
			DbgLog("Close user %s link after notified user", LoginRes.suserid().c_str());
			pKickoutLink->CloseLink(); //Close the old link since it has aleady kickout. 
			pKickoutLink->ReleaseRef();
		}
	}
}

bool CTransimission::NotifyPCToLineOff(std::shared_ptr<CImPdu> pPdu)
{ 
    InfoLog("send notify pc to offline");
	if(BroadcastPdu(DESKTOP, pPdu.get()) <= 0)
	{
		ErrLog("Failed to send the packet 0x%x ,Maybe the linker between CM and DESTKTOP server is lost...",pPdu->GetCommandId());
		return false;
	}
} 
  
bool CTransimission::sendAck(const google::protobuf::MessageLite* pMsg, uint16_t command_id, const UidCode_t& sessionId)
{
	bool bRest = true;
	CImPdu pdu;
	pdu.SetPBMsg(pMsg);
	pdu.SetSessionId(sessionId);
	pdu.SetCommandId(command_id);
	int nLen = SendPdu(&pdu);
	if (nLen <= 0)
	{
		WarnLog("!!!send command 0x%x failed, return %d, session=%x%x%x%x%x%x%x%x%x%x%x%x", command_id, nLen,
			sessionId.Uid_Item.code[0], sessionId.Uid_Item.code[1],
			sessionId.Uid_Item.code[2], sessionId.Uid_Item.code[3], sessionId.Uid_Item.code[4],
			sessionId.Uid_Item.code[5], sessionId.Uid_Item.code[6], sessionId.Uid_Item.code[7],
			sessionId.Uid_Item.code[8], sessionId.Uid_Item.code[9], sessionId.Uid_Item.code[10],
			sessionId.Uid_Item.code[11]);
		bRest = false;
	}
	return bRest;
}


