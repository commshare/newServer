/*
 * LoginServConn.cpp
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#include "impdu.h"
#include "public_define.h"
#include "ServConn.h"
//#include "IM.Other.pb.h"
//#include "IM.Message.pb.h"
#include "im.cm.pb.h"
#include "im.mes.pb.h"
#include "im.group.pb.h"
#include "im.sig.pb.h"


using namespace im;

static ConnMap_t g_server_conn_map;

static serv_info_t* g_server_list;
static uint32_t	g_server_count;

//static string g_server_ip_addr1;
//static string g_server_ip_addr2;
static uint16_t g_server_port;
static uint32_t g_max_conn_cnt;
extern string gSessionId;

extern void AddBuddyAns();

void server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CServConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();

	for (ConnMap_t::iterator it = g_server_conn_map.begin(); it != g_server_conn_map.end(); ) {
		it_old = it;
		it++;

		pConn = (CServConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}

	// reconnect Server
	serv_check_reconnect<CServConn>(g_server_list, g_server_count);
}

void init_serv_conn(serv_info_t* server_list, uint32_t server_count, const char* msg_server_ip_addr1,
		const char* msg_server_ip_addr2, uint16_t msg_server_port, uint32_t max_conn_cnt)
{
	g_server_list = server_list;
	g_server_count = server_count;

	serv_init<CServConn>(g_server_list, g_server_count);

//	g_server_ip_addr1 = msg_server_ip_addr1;
//	g_server_ip_addr2 = msg_server_ip_addr2;
	g_server_port = msg_server_port;
	g_max_conn_cnt = max_conn_cnt;

	netlib_register_timer(server_conn_timer_callback, NULL, 1000);
}

// if there is one Server available, return true
bool is_server_available()
{
	CServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_server_count; i++) {
		pConn = (CServConn*)g_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			return true;
		}
	}

	return false;
}

void send_to_all_server(CImPdu* pPdu)
{
	CServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_server_count; i++) {
		pConn = (CServConn*)g_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			printf("Sending\n");
			pConn->SendPdu(pPdu);
		}
	}
}

CServConn::CServConn()
{
	m_bOpen = false;
}

CServConn::~CServConn()
{

}

void CServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx)
{
	log("Connecting to Server %s:%d ", server_ip, server_port);
	m_serv_idx = serv_idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_server_conn_map);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CServConn::Close()
{
	serv_reset<CServConn>(g_server_list, g_server_count, m_serv_idx);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_server_conn_map.erase(m_handle);
	}

	ReleaseRef();
}

void CServConn::OnConfirm()
{
	printf("connect to server success \n");
	m_bOpen = true;
	g_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;
}

void CServConn::OnClose()
{
	log("server conn onclose, from handle=%d ", m_handle);
	Close();
}

void CServConn::OnTimer(uint64_t curr_tick)
{
#if 0
    if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
        IM::Other::IMHeartBeat msg;
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        //pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_HEARTBEAT);
		SendPdu(&pdu);
	}

	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
		log("conn to login server timeout ");
		Close();
	}
    #endif
}

void CServConn::HandlePdu(std::shared_ptr<CImPdu> pPdu)
{
	printf("recv client , CommandId=%x \r\n ", /*pPdu->GetSessionId().c_str(),*/pPdu->GetCommandId());

	switch (pPdu->GetCommandId()) {
	//case CID_OTHER_HEARTBEAT:
	//	break;
	case CM_LOGIN_ACK:
		_HandleLoginAck(pPdu);
		break;
	case MES_CHAT_ACK:
		_HandleShowChatMessage(pPdu);
		break;
	case MES_CHAT_DELIVER:
		_HandleChatDeliver(pPdu);
		break;
	case MES_ADDFRIEND_ACK:
		_HandleAddBuddyAck(pPdu);
	case SYSTEM_ASSOCSVR_REGIST_ACK:
		_HandleMessageRegAck(pPdu);
		break;
	case MES_ADDFRIEND_ANS_ACK:
		_HandleAddBuddyAnsAck(pPdu);
		break;	

	case MES_ADDFRIEND_DELIVER:
		_HandleAddBuddyDeliver(pPdu);
		break;
	//case CID_DBA_SET_SYSMSG_READ_ACK:
	//	_HandleDBASetSystemReadAck(pPdu);
	//	break;
	case MES_OFFLINESUMMARY_ACK:
		_HandleOfflineSummaryAck(pPdu);
		break;
	case MES_OFFLINETOTAL_ACK:
		_HandleOfflineTotalAck(pPdu);
		break;
	case MES_OFFLINEMSG_ACK:
		_HandleOfflineMsgAck(pPdu);
		break;
	case MES_OFFLINEMSG_DELIVERED_NOTIFICATION:
		_HandleOfflineMsgDeliveredNotify(pPdu);
		break;
	case GROUP_CHAT_ACK:
		_HandleGrpChatAck(pPdu);
		break;
	case MES_GRPNOTIFY_DELIVER:		//接收端处理通知消息
		_HandleGrpNotifyDeliver(pPdu);
		break;
	case MES_EXCHANGE_KEY_DELIVER:		
		_HandleExchangeKeyDeliver(pPdu);
		break;	
	case MES_EXCHANGE_KEY_DELIVERD_NOTIFY:		
		_HandleExchangeKeyDeliverNotify(pPdu);
		break;	
	case MES_JOINGRP_DELIVER:		//接收端收到请求
		_HandleJoinGrpDeliver(pPdu);
		break;
	case SIG_SPONSORP2PCALL_DELIVER:
		_HandleSponsorP2PCallDeliver(pPdu);
		break;
	case SIG_SPONSORP2PCALL_ANS_DELIVER:		
		_HandleSponsorP2PCallAnsDeliver(pPdu);
		break;
	case SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER:
		_HandleP2PCallExchangeNatInfoDeliver(pPdu);
		break;
	//case SIG_P2PCALL_EXCHANGE_NATINFO_DELIVERD_NOTIFY:
	//	_HandleP2PCallExchangeNatInfoDeliverNotify(pPdu);
	//	break;
	case SIG_P2PCALLHANGUPDElIVER:
		break;
		_HandleP2PCallHangupDeliver(pPdu);		
	default:
		break;
	}
}

void CServConn::_HandleMessageRegAck(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu){
		return ;
	}
	
	gSessionId = pPdu->GetSessionId();
	printf("**  recv reg ack  **\r\n"/*, gSessionId.c_str()*/);
}

void CServConn::_HandleLoginAck(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu){
		return ;
	}

	CMLoginAck loginAck;

	loginAck.ParseFromArray(pPdu->GetBodyData(),pPdu->GetBodyLength());
	gSessionId = pPdu->GetSessionId();
	printf("userid = %s, session len = %ld  status = 0x%x,\r\n",
		loginAck.suserid().c_str(), /*gSessionId.c_str(),*/ gSessionId.size(), loginAck.nerr());
}

void CServConn::_HandleShowChatMessage(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu){
		return ;
	}
	
	static int globalChatCount = 0;
	++globalChatCount;
	MESChatAck chatAck;
	chatAck.ParseFromArray(pPdu->GetBodyData(),pPdu->GetBodyLength());

	printf("recv msgChatAck %d times, %s:%s\r\n",globalChatCount,chatAck.suserid().c_str(),chatAck.smsgid().c_str());
}

void CServConn::_HandleChatDeliver(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu){
		return ;
	}
	MESChat chat;
	MESChatDeliveredAck chatDeliveredAck;
	CImPdu ackPdu;
	chat.ParseFromArray(pPdu->GetBodyData(),pPdu->GetBodyLength());
	printf("cmdid= 0x%x,fromid = %s,toid = %s,msgid=%s\r\n",pPdu->GetCommandId(),
		chat.sfromid().c_str(),chat.stoid().c_str(),chat.smsgid().c_str());	
	chatDeliveredAck.set_sfromid(chat.sfromid());
	chatDeliveredAck.set_stoid(chat.stoid());
	chatDeliveredAck.set_smsgid(chat.smsgid());

     ackPdu.SetPBMsg(&chatDeliveredAck);
     ackPdu.SetSessionId(gSessionId);
     ackPdu.SetCommandId(MES_CHAT_DELIVER_ACK);
     send_to_all_server(&ackPdu);
}


void CServConn::_HandleShowRecentMsgAck(std::shared_ptr<CImPdu> pPdu)
{

	if(NULL == pPdu){
		return ;
	}
#if 0	
	IM::Message::RecentMsgListAck msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(),pPdu->GetBodyLength()));

	//Show recent message ack
	if(0 == msg.ret()){
		int size = msg.recentmsglist_size();
		for(int i = 0;i < size;i++){
			printf("index=%d,type=%d,unreads=%d,curMsgId=%lld,curMsgContent=%s,curMsgTime=%lld\r\n",
				i,msg.recentmsglist(i).type(),msg.recentmsglist(i).unreads(),msg.recentmsglist(i).curmsgid(),
				msg.recentmsglist(i).curmsgcontent().c_str(),msg.recentmsglist(i).curmsgtime());
		}
	}else{
		printf("Recent Message List Ack Error=%d\r\n",msg.ret());
	}
#endif
}

void CServConn::_HandleAddBuddyAck(std::shared_ptr<CImPdu> pPdu)
{

	MESAddFriendAck friendAck;
	friendAck.ParseFromArray(pPdu->GetBodyData(),pPdu->GetBodyLength());

	printf("cmdid= 0x%x,fromid = %s,msgid=%s\r\n",pPdu->GetCommandId(),
		friendAck.suserid().c_str(),friendAck.smsgid().c_str());		
}

void CServConn::_HandleAddBuddyDeliver(std::shared_ptr<CImPdu> pPdu)
{
	MESAddFriend addFriendDeliver;
	addFriendDeliver.ParseFromArray(pPdu->GetBodyData(),pPdu->GetBodyLength());

	printf("cmdid= 0x%x,add from userid = %s,msgid=%s\r\n",pPdu->GetCommandId(),
		addFriendDeliver.sfromid().c_str(),addFriendDeliver.smsgid().c_str());	

	AddBuddyAns();
}

void CServConn::_HandleAddBuddyAnsAck(std::shared_ptr<CImPdu> pPdu)
{
	MESAddFriendAnsAck friendAnsAck;
	friendAnsAck.ParseFromArray(pPdu->GetBodyData(),pPdu->GetBodyLength());

	printf("cmdid= 0x%x,userid = %s,msgid=%s\r\n",pPdu->GetCommandId(),
		friendAnsAck.suserid().c_str(),friendAnsAck.smsgid().c_str());		

}

void CServConn::_HandleOfflineSummaryAck(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)
	{ 
		return ;
	}	
	MESOfflineSummaryAck msg;
	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());

	printf("**  recv MESOfflineSummaryAck ,to %s, ls size = %d  **\r\n", 
		msg.suserid().c_str(), msg.offlinetotals_size());

	int index = 0;
	for (index = 0; index < msg.offlinetotals_size(); ++index)
	{
		 const OfflineTotal& offLineTotal = msg.offlinetotals(index);		 
		const string& content = offLineTotal.srecentcontent();
		
		 if(MES_CHAT_DELIVER == offLineTotal.cmdid())
		 {
		 	MESChat chat;
			chat.ParseFromString(content);

			printf("%03d: offlineTotal 0x%x from %s - %s, uread %d,  and latest chat from %s - %s\r\n", index,
				MES_CHAT_DELIVER, offLineTotal.sfromid().c_str(), offLineTotal.stoid().c_str(), offLineTotal.unreadtotal(),
				chat.sfromid().c_str(), offLineTotal.stoid().c_str());

//			if(offLineTotal.unreadtotal() > 1)
//			{
//				MESOfflineMsg offlineMsg;
//				offlineMsg.set_sfromid(offLineTotal.sfromid());
//				offlineMsg.set_stoid(offLineTotal.stoid());
//				offlineMsg.set_count(40);
//
//				CImPdu pdu;
//				pdu.SetPBMsg(&offlineMsg);
//				pdu.SetSessionId(gSessionId);
//				pdu.SetCommandId(MES_OFFLINEMSG);
//				SendPdu(&pdu);
//				break;
//				
//			}
				
		 }
		 else if(MES_GRPCHAT_DELIVER == offLineTotal.cmdid())
		 {
		 	GroupChat chat;
			chat.ParseFromString(content);

			printf("%03d: offlineTotal 0x%x from %s - %s, uread %d,  and latest chat from user:%s - grp:%s\r\n", 
			index, MES_GRPCHAT_DELIVER, 	offLineTotal.sfromid().c_str(), offLineTotal.stoid().c_str(), offLineTotal.unreadtotal(),
				chat.sfromid().c_str(), chat.sgroupid().c_str());

//			if(offLineTotal.unreadtotal() > 1)
//			{
//				MESOfflineMsg offlineMsg;
//				offlineMsg.set_sfromid(offLineTotal.sfromid());
//				offlineMsg.set_stoid(offLineTotal.stoid());
//				offlineMsg.set_count(40);
//
//				CImPdu pdu;
//				pdu.SetPBMsg(&offlineMsg);
//				pdu.SetSessionId(gSessionId);
//				pdu.SetCommandId(MES_OFFLINEMSG);
//				SendPdu(&pdu);
//				break;
//				
//			}
		 }
		 else if(MES_GRPNOTIFY_DELIVER == offLineTotal.cmdid())
		 {
		 	MESGrpNotify notify;
			notify.ParseFromString(content);
			
		 	printf("%03d: offlineTotal 0x%x from %s - %s, uread %d,  and latest notify from grp:%s - user:%s\r\n", 
					 index, MES_GRPNOTIFY_DELIVER,	 offLineTotal.sfromid().c_str(), offLineTotal.stoid().c_str(), offLineTotal.unreadtotal(),
					notify.sgrpid().c_str(), notify.stoid().c_str());
		 }
		 	
	}
	printf("total %d user's offlineMsg\r\n", msg.offlinetotals_size());
		
}

void CServConn::_HandleOfflineTotalAck(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu) { return;}

}

void CServConn::_HandleOfflineMsgAck(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)  
	{
		return;
	}
	
	MESOfflineMsgAck msg;
	if(!msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	{
		printf("para msg failed\r\n");
	}

	printf("%s recv %s offline msgAck, size = %d\r\n",
		msg.stoid().c_str(),msg.sfromid().c_str(), msg.msglist_size());

//	MESOfflineMsgDelivered delivered;
//	delivered.set_sfromid(msg.suserid());
//	delivered.set_smsgid("msgDeliver");
	int index = 0;
	for (index = 0; index < msg.msglist_size(); ++index)
	{
		 const OfflineMsgData& offlineMsg = msg.msglist(index);
		 if(MES_CHAT_DELIVER == offlineMsg.cmdid())
		 {
		 	MESChat chat;
			const string& content = offlineMsg.smsgdata();
			chat.ParseFromString(content);

//			delivered.set_stoid(chat.sfromid());

			printf("%03d: offlineMsg from %s - %s, msgId = %s \r\n", index+1,				
				chat.sfromid().c_str(), chat.stoid().c_str(), chat.smsgid().c_str());

//			OfflineDeliveredMsg* deliveredMsg = delivered.add_lsmsgs();
//			deliveredMsg->set_cmdid(offlineMsg.cmdid());
//			deliveredMsg->set_sfromid(chat.sfromid());
//			deliveredMsg->set_stoid(chat.stoid());
//			deliveredMsg->set_smsgid(chat.smsgid());
				
		 }
		 else if(MES_GRPCHAT_DELIVER == offlineMsg.cmdid())
		 {
			const string& content = offlineMsg.smsgdata();
		 	GroupChat chat;
			chat.ParseFromString(content);

			printf("%03d: grp chat(0x%x) from %s(%s), content %s\r\n", 
			index+1, MES_GRPCHAT_DELIVER, chat.sgroupid().c_str(),
			chat.sfromid().c_str(), chat.scontent().c_str());

//			if(offLineTotal.unreadtotal() > 1)
//			{
//				MESOfflineMsg offlineMsg;
//				offlineMsg.set_sfromid(offLineTotal.sfromid());
//				offlineMsg.set_stoid(offLineTotal.stoid());
//				offlineMsg.set_count(40);
//
//				CImPdu pdu;
//				pdu.SetPBMsg(&offlineMsg);
//				pdu.SetSessionId(gSessionId);
//				pdu.SetCommandId(MES_OFFLINEMSG);
//				SendPdu(&pdu);
//				break;
//				
//			}
		 }
	}
//
//	CImPdu pdu;
//	pdu.SetPBMsg(&delivered);
//	pdu.SetSessionId(gSessionId);
//	pdu.SetCommandId(MES_OFFLINEMSG_DELIVERED);
//	SendPdu(&pdu);
	
}


void CServConn::_HandleOfflineMsgDeliveredNotify(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)
	{
		return ;
	}	
	MESOfflineMsgDelivered msg;
	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());

	MESOfflineMsgDeliveredNotifyAck ack;
	ack.set_sfromid(msg.stoid());
	ack.set_smsgid(msg.smsgid());

	int32_t index = 0;
	for (index = 0; index < msg.lsmsgs_size(); ++index)
	{
		const OfflineDeliveredMsg& deliveredMsg = msg.lsmsgs(index);
  		OfflineDeliveredMsg* pDeliveredMsg = ack.add_lsmsgs();
		*pDeliveredMsg = deliveredMsg;
	}
	CImPdu pdu;
	pdu.SetPBMsg(&ack);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(MES_OFFLINEMSG_DELIVERED_NOTIFICATION_ACK);
	SendPdu(&pdu);
	
}

void CServConn::_HandleGrpNotifyDeliver(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)
	{
		return ;
	}	
	MESGrpNotify msg;
	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
	printf("recv grpNotify(0x%x) %s from %s-->%s, content %s\n",pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sgrpid().c_str(), msg.stoid().c_str(), msg.scontent().c_str());

	MESGrpNotifyDeliverAck ack;
	ack.set_suserid(msg.stoid());
	ack.set_smsgid(msg.smsgid());
	ack.set_sgrpid(msg.sgrpid());

	CImPdu pdu;
	pdu.SetPBMsg(&ack);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(MES_GRPNOTIFY_DELIVER_ACK);
	SendPdu(&pdu);	
}

void CServConn::_HandleGrpChatAck(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)
	{
		return ;
	}
	static int count = 0;
	printf("recv grpchatAck %d times \t", ++count);

}


void CServConn::_HandleExchangeKeyDeliver(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)
	{
		return ;
	}	
	MESExchangeKeyDeliver msg;
	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
//	printf("recv exchangeKeyDeliver(0x%x) %s from %s:%s-->%s\n",pPdu->GetCommandId(),
//		msg.smsgid().c_str(), msg.sgrpid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());

	MESExchangeKeyDeliverAck ack;
	ack.set_sfromid(msg.stoid());
	ack.set_stoid(msg.sfromid());
	ack.set_smsgid(msg.smsgid());
	ack.set_sgrpid(msg.sgrpid());
	ack.set_skey("target user's key");
	ack.set_errcode(NON_ERR);

	CImPdu pdu;
	pdu.SetPBMsg(&ack);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(MES_EXCHANGE_KEY_DELIVER_ACK);
	SendPdu(&pdu);	
}

void CServConn::_HandleExchangeKeyDeliverNotify(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)
	{
		return ;
	}	
	MESExchangeKeyDeliverAck msg;
	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
	printf("recv exchangeKeyDeliverNotify(0x%x) %s from %s:%s-->%s, content %s\n",pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sgrpid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), msg.skey().c_str());

	MESExchangeKeyDeliverNotifyAck ack;
	ack.set_smsgid(msg.smsgid());
	ack.set_suserid(msg.stoid());

	CImPdu pdu;
	pdu.SetPBMsg(&ack);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(MES_EXCHANGE_KEY_DELIVERD_NOTIFY_ACK);
	SendPdu(&pdu);	
}

void CServConn::_HandleSponsorP2PCallDeliver(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)
	{
		return ;
	}	
	SIGSponsorCall msg;
	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
	printf("recv SIGSponsorCallDeliver(0x%x) %s from %s-->%s, content %s\n",pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sfromid().c_str(), msg.sinviteid().c_str(), msg.scontent().c_str());

	SIGSponsorCallDeliverAck ack;
	ack.set_suserid(msg.sinviteid());
	ack.set_smsgid(msg.smsgid());

	CImPdu pdu;
	pdu.SetPBMsg(&ack);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(SIG_SPONSORP2PCALL_DELIVER_ACK);
	SendPdu(&pdu);	

	SIGSponsorCallAns ans;
	ans.set_sfromid(msg.sinviteid());
	ans.set_stoid(msg.sfromid());
	ans.set_smsgid(msg.smsgid());
	ans.set_errcode(im::NON_ERR);
	ans.set_scontent("sponsor call ans");

	CImPdu pdu2;
	pdu2.SetPBMsg(&ans);
	pdu2.SetSessionId(gSessionId);
	pdu2.SetCommandId(SIG_SPONSORP2PCALL_ANS);
	SendPdu(&pdu2);	


}
void CServConn::_HandleSponsorP2PCallAnsDeliver(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)
	{
		return ;
	}	
	SIGSponsorCallAns msg;
	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
	printf("recv SIGSponsorCallAnsDeliver(0x%x) %s from %s-->%s, content %s\n",pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), msg.scontent().c_str());

	SIGSponsorCallAnsDeliverACK ack;
	ack.set_suserid(msg.stoid());
	ack.set_smsgid(msg.smsgid());

	CImPdu pdu;
	pdu.SetPBMsg(&ack);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(SIG_SPONSORP2PCALL_ANS_DELIVER_ACK);
	SendPdu(&pdu);	

}
void CServConn::_HandleP2PCallExchangeNatInfoDeliver(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)
	{
		return ;
	}	
	SIGP2PCallExchangeNatInfo msg;
	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
	printf("recv SIGP2PCallExchangeNatInfoDeliver(0x%x) %s from %s-->%s, content %s\n",pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), msg.scontent().c_str());

	SIGP2PCallExchangeNatInfoDeliverAck ack;
	//ack.set_sfromid(msg.stoid());
	//ack.set_stoid(msg.sfromid());
	ack.set_suserid(msg.sfromid());
	ack.set_smsgid(msg.smsgid());	
	//ack.set_errcode(im::NON_ERR);
	//ack.set_scontent("SIGP2PCallExchangeNatInfoDeliverAck");

	CImPdu pdu;
	pdu.SetPBMsg(&ack);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(SIG_P2PCALL_EXCHANGE_NATINFO_DELIVER_ACK);
	SendPdu(&pdu);	

}
void CServConn::_HandleP2PCallExchangeNatInfoDeliverNotify(std::shared_ptr<CImPdu> pPdu)
{
	#if 0
	if(NULL == pPdu)
	{
		return ;
	}	
	SIGP2PCallExchangeNatInfoDeliverAck msg;
	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
	printf("recv P2PCallExchangeNatInfoDeliverNotify(0x%x) %s from %s-->%s, content %s\n",pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), msg.scontent().c_str());

	SIGP2PCallExchangeNatInfoDeliverdNotifyAck ack;
	ack.set_suserid(msg.stoid());
	ack.set_smsgid(msg.smsgid());

	CImPdu pdu;
	pdu.SetPBMsg(&ack);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(SIG_P2PCALL_EXCHANGE_NATINFO_DELIVERD_NOTIFY_ACK);
	SendPdu(&pdu);	
	#endif

}
void CServConn::_HandleP2PCallHangupDeliver(std::shared_ptr<CImPdu> pPdu)
{
	if(NULL == pPdu)
	{
		return ;
	}	
	SIGHangUp msg;
	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
	printf("recv P2PCallHangupDeliver(0x%x) %s from %s-->%s\n",pPdu->GetCommandId(),
		msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());

	SIGHangUpDeliverAck ack;
	ack.set_suserid(msg.stoid());
	ack.set_smsgid(msg.smsgid());

	CImPdu pdu;
	pdu.SetPBMsg(&ack);
	pdu.SetSessionId(gSessionId);
	pdu.SetCommandId(SIG_P2PCALLHANGUPDElIVER_ACK);
	SendPdu(&pdu);	

}


void CServConn::_HandleJoinGrpDeliver(std::shared_ptr<CImPdu> pPdu)
{
//	if(NULL == pPdu)
//	{
//		return ;
//	}	
//	MESJoinGrp msg;
//	msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());
//	printf("recv joinGrpDeliver(0x%x) %s from %s:%s-->%s, content %s\n",pPdu->GetCommandId(),
//		msg.smsgid().c_str(), msg.sgrpid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), msg.sselfintroduce().c_str());
//
//	MESJoinGrpDeliverAck ack;
//	ack.set_smsgid(msg.smsgid());
//	ack.set_suserid(msg.stoid());
//	ack.set_sgrpid(msg.sgrpid());
//
//	CImPdu pdu;
//	pdu.SetPBMsg(&ack);
//	pdu.SetSessionId(gSessionId);
//	pdu.SetCommandId(MES_JOINGRP_DELIVER_ACK);
//	SendPdu(&pdu);	
//
//	MESJoinGrpAns ans;
//	ans.set_smsgid(msg.smsgid());
//	ans.set_sfromid(msg.stoid());
//	ans.set_stoid(msg.sfromid());
//	ans.set_sgrpid(msg.sgrpid());
//	ans.set_rsptype(msg.reqtype());
//	ans.set_errcode(NON_ERR);
//	
//	CImPdu ansPdu;
//	ansPdu.SetPBMsg(&ans);
//	ansPdu.SetSessionId(gSessionId);
//	ansPdu.SetCommandId(MES_JOINGRP_ANS);
//	SendPdu(&ansPdu);	
	
}

void CServConn::_HandleDBASetSystemReadAck(std::shared_ptr<CImPdu> pPdu)
{
#if 0
	CConfigFileReader config_file(GetConfigFileName().c_str());
	
	char* strResponserId = config_file.GetConfigName("responserId");
	char* strResponserNickName = config_file.GetConfigName("responserNickname");
	char* strResponserGender = config_file.GetConfigName("responserGender");
	char* strResponserAvatar = config_file.GetConfigName("responserAvatar");
	char* strBuddyUid = config_file.GetConfigName("buddyUid");
	char* strMsgId = config_file.GetConfigName("msgId");
	char* strAgree = config_file.GetConfigName("agree");
	char* strSubGruopId = config_file.GetConfigName("subGruopId");

	IM::Message::BuddyVerify msg;
	msg.set_responserid(atoll(strResponserId));
	msg.set_responsernickname(strResponserNickName);
	msg.set_responsergender(atoi(strResponserGender));
	msg.set_responseravatar(strResponserAvatar);
	msg.set_buddyuid(atoll(strBuddyUid));
	msg.set_msgid(atoll(strMsgId));
	msg.set_agree(atoi(strAgree));
	msg.set_subgruopid(atoi(strSubGruopId));

	CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetSessionId(pPdu->GetSessionId());
        pdu.SetCommandId(CID_MESSAGE_BUDDY_VERIFY);

	SendPdu(&pdu);
#endif	
}


