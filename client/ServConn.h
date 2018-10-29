/*
 * LoginServConn.h
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#ifndef LOGINSERVCONN_H_
#define LOGINSERVCONN_H_

#include "imconn.h"
#include "ServInfo.h"
#include <memory>


class CServConn : public CImConn
{
public:
	CServConn();
	virtual ~CServConn();

	bool IsOpen() { return m_bOpen; }

	void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
	virtual void Close();

	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(std::shared_ptr<CImPdu> pPdu);
private:
	void _HandleShowChatMessage(std::shared_ptr<CImPdu> pPdu);
	void _HandleShowRecentMsgAck(std::shared_ptr<CImPdu> pPdu);
	void _HandleAddBuddyAck(std::shared_ptr<CImPdu> pPdu);
	void _HandleAddBuddyAnsAck(std::shared_ptr<CImPdu> pPdu);	
	void _HandleAddBuddyDeliver(std::shared_ptr<CImPdu> pPdu);	
	void _HandleDBASetSystemReadAck(std::shared_ptr<CImPdu> pPdu);
	void _HandleMessageRegAck(std::shared_ptr<CImPdu> pPdu);
	void _HandleLoginAck(std::shared_ptr<CImPdu> pPdu);
	void _HandleChatDeliver(std::shared_ptr<CImPdu> pPdu);
	void _HandleOfflineSummaryAck(std::shared_ptr<CImPdu> pPdu);
	void _HandleOfflineTotalAck(std::shared_ptr<CImPdu> pPdu);
	void _HandleOfflineMsgAck(std::shared_ptr<CImPdu> pPdu);
	void _HandleOfflineMsgDeliveredNotify(std::shared_ptr<CImPdu> pPdu);
	void _HandleGrpNotifyDeliver(std::shared_ptr<CImPdu> pPdu);
	void _HandleGrpChatAck(std::shared_ptr<CImPdu> pPdu);
	void _HandleExchangeKeyDeliver(std::shared_ptr<CImPdu> pPdu);
	void _HandleExchangeKeyDeliverNotify(std::shared_ptr<CImPdu> pPdu);
	void _HandleJoinGrpDeliver(std::shared_ptr<CImPdu> pPdu);
	void _HandleSponsorP2PCallDeliver(std::shared_ptr<CImPdu> pPdu);
	void _HandleSponsorP2PCallAnsDeliver(std::shared_ptr<CImPdu> pPdu);
	void _HandleP2PCallExchangeNatInfoDeliver(std::shared_ptr<CImPdu> pPdu);
	void _HandleP2PCallExchangeNatInfoDeliverNotify(std::shared_ptr<CImPdu> pPdu);
	void _HandleP2PCallHangupDeliver(std::shared_ptr<CImPdu> pPdu);
	
		
		
	
	
	bool 		m_bOpen;
	uint32_t	m_serv_idx;
};

void init_serv_conn(serv_info_t* server_list, uint32_t server_count, const char* msg_server_ip_addr1,
		const char* msg_server_ip_addr2, uint16_t msg_server_port, uint32_t max_conn_cnt);
bool is_server_available();
void send_to_all_server(std::shared_ptr<CImPdu> pPdu);

#endif /* MSGCONN_LS_H_ */
