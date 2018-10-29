/******************************************************************************
Filename: offLineMsgHandle.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/07
Description:
******************************************************************************/
#ifndef __OFFLINEMSGHANDLE_H__
#define __OFFLINEMSGHANDLE_H__
#include "basehandle.h"
#include "offlineMsg.h"
#include "offlineMsgMgr.h"
#include "grpOfflineMsgMgr.h"


class CImPdu;
class CConfigFileReader;

class COfflineMsgHandler : public CBaseHandle
{
public:
    COfflineMsgHandler(CConfigFileReader* pConfigReader, int nNumOfInst);
    ~COfflineMsgHandler();

    void OnMsgOfflineSummary(std::shared_ptr<CImPdu> pPdu);				// 响应添加好友请求
    void OnMsgOfflineTotal(std::shared_ptr<CImPdu> pPdu);				// 收到添加好友推送应答
    void OnGetOfflineMsg(std::shared_ptr<CImPdu> pPdu);					// 响应添加好友操作结果（同意、拒绝）
    void HandleGetOfflineMsg(const im::MESOfflineMsg& msg, const UidCode_t& sessionId);

    void onOfflineMsgDelivered(std::shared_ptr<CImPdu> pPdu);			// 收到添加好友操作结果推送应答
    void HandleOfflineMsgDelivered(const im::MESOfflineMsgDelivered& msg, const UidCode_t& sessionId);
	void HandleOfflineMsgDelivered(const im::MESOfflineMsg& msg);

    void HandleOfflineMsgDeliveredNotifyAckTask(const im::MESOfflineMsgDeliveredNotifyAck& msg);
    void onOfflineMsgDeliveredNotifyAck(std::shared_ptr<CImPdu> pPdu);	// 响应删除好友请求

    void OnOfflineMsgChatDeliverNotifyInserted(const COfflineMsg& Msg, bool bInsertSuccess);
    void GetGrpOfflineMsgContent(im::MESOfflineMsgAck &offlineMsgAck, CGrpOfflineMsgMgr &grpOfflineMsgMgr, int maxLen = 65535);
protected:
    virtual bool RegistPacketExecutor(void);
private:
    //COfflineMsgMgr m_offlineMsgMgr;
    CGrpOfflineMsgMgr m_grpOfflineMsgMgr;
    int 			m_nNumberOfInst;

};

#endif // __OFFLINEMSGHANDLE_H__


