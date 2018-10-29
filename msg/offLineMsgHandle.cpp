/******************************************************************************
Filename: friendhandle.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/06/29
Description:
******************************************************************************/
#include <map>
#include "configfilereader.h"
#include "offLineMsgHandle.h"
#include "im_loginInfo.h"
#include "im.mes.pb.h"
#include "im.group.pb.h"
#include "im.pub.pb.h"
#include "redisLoginInfoMgr.h"
#include "mysqlFriendMgr.h"
#include "redisFriendMgr.h"
#include "util.h"
#include "commonTaskMgr.h"

using namespace im;
using namespace std;

COfflineMsgHandler::COfflineMsgHandler(CConfigFileReader* pConfigReader, int nNumOfInst)
    :CBaseHandle(pConfigReader),m_grpOfflineMsgMgr(GRPOFFLINEMSG_MONGO_DB_NAME, GRPOFFLINEMSG_MONGO_COLL_NAME), m_nNumberOfInst(nNumOfInst)
{

}

COfflineMsgHandler::~COfflineMsgHandler()
{

}

void GetContentForGrpOfflineMsg(MESOfflineSummaryAck& offlineMsgSummaryAck, CGrpOfflineMsgMgr& grpOfflineMsgMgr)
{
    //CUsecElspsedTimer elspedTimer;
    //elspedTimer.start();
    int index = 0;
    for (index = 0; index < offlineMsgSummaryAck.offlinetotals_size(); ++index)
    {
        OfflineTotal* pOffLineTotal = offlineMsgSummaryAck.mutable_offlinetotals(index);
        if (MES_GRPCHAT_DELIVER == pOffLineTotal->cmdid())
        {
            const string& content = grpOfflineMsgMgr.GetGrpOfflineMsgData(pOffLineTotal->sfromid(), pOffLineTotal->srecentmsgid());
#if 0
            GroupChat grpChat;
            if (grpChat.ParseFromString(content))
            {
                log("get %s's data, from %s:%s to %s", pOffLineTotal->srecentmsgid().c_str(),
                    grpChat.sgroupid().c_str(), grpChat.sfromid().c_str(), pOffLineTotal->stoid().c_str());
            }
#endif
            pOffLineTotal->clear_srecentmsgid();
            pOffLineTotal->set_srecentcontent(content);
        }
    }
    //DbgLog("get content for MESOfflineSummaryAck use %llu useconds", elspedTimer.elapsed());
}

void COfflineMsgHandler::OnMsgOfflineSummary(std::shared_ptr<CImPdu> pPdu)
{
    assert(NULL != pPdu);
    MESOfflineSummary msg;
    if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        return;
    }
    log("\r\n\r\n\r\nMESOfflineSummary(0x%x) received, request %s's msgSummary",
        pPdu->GetCommandId()/*MES_OFFLINESUMMARY*/, msg.suserid().c_str());

    if (msg.suserid().empty())
    {
        ErrLog("!!!lack of required parameter");
        return;
    }

    MESOfflineSummaryAck offlineMsgSummaryAck = m_offlineMsgMgr.GetUserOfflineMsgSummaryGrpByFromId(msg.suserid());
    GetContentForGrpOfflineMsg(offlineMsgSummaryAck, m_grpOfflineMsgMgr);


    sendAck(&offlineMsgSummaryAck, MES_OFFLINESUMMARY_ACK, pPdu->GetSessionId());
    log("****send MESOfflineSummaryAck(0x%x) to %s,lsSize = %d", MES_OFFLINESUMMARY_ACK,
        offlineMsgSummaryAck.suserid().c_str(), offlineMsgSummaryAck.offlinetotals_size());
}

void COfflineMsgHandler::OnMsgOfflineTotal(std::shared_ptr<CImPdu> pPdu)
{
    assert(NULL != pPdu);
    MESOfflineTotal msg;
    if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        return;
    }
    log("\r\n\r\n\r\nMsgOfflineTotal(0x%x) received, request %s's msg",
        pPdu->GetCommandId()/*MES_OFFLINETOTAL*/, msg.stoid().c_str());

    if (msg.sfromid().empty() || msg.stoid().empty())
    {
        ErrLog("!!!lack of required parameter");
        return;
    }

    MESOfflineSummaryAck offlineMsgTotalAck = m_offlineMsgMgr.GetUserOfflineTotal(msg.stoid(), msg.sfromid(), msg.cmdid());
    GetContentForGrpOfflineMsg(offlineMsgTotalAck, m_grpOfflineMsgMgr);

    sendAck(&offlineMsgTotalAck, MES_OFFLINETOTAL_ACK, pPdu->GetSessionId());
    log("****send MESOfflineTotalAck(0x%x) to %s,lsSize = %d", MES_OFFLINETOTAL_ACK,
        offlineMsgTotalAck.suserid().c_str(), offlineMsgTotalAck.offlinetotals_size());
}

//void GetContentForGrpOfflineMsg(MESOfflineMsgAck& offlineMsgAck, CGrpOfflineMsgMgr& grpOfflineMsgMgr)
//{
//	CUsecElspsedTimer elspedTimer;
//	elspedTimer.start();
//	int index = 0;
//	//这里可以优化，值查询一次************************************************************
//	for (index = 0; index < offlineMsgAck.msglist_size(); ++index)
//	{
//		OfflineMsgData* pOfflineMsg = offlineMsgAck.mutable_msglist(index);
//		if (MES_GRPCHAT_DELIVER == pOfflineMsg->cmdid())
//		{
//			const string& content = grpOfflineMsgMgr.GetGrpOfflineMsgData(offlineMsgAck.sfromid(), pOfflineMsg->smsgid());
//#if 1
//			GroupChat grpChat;
//			if (grpChat.ParseFromString(content))
//			{
//				log("get %s's data, from %s:%s to %s", pOfflineMsg->smsgid().c_str(),
//					grpChat.sgroupid().c_str(), grpChat.sfromid().c_str(), offlineMsgAck.stoid().c_str());
//			}
//#endif
//			pOfflineMsg->set_smsgdata(content);
//			pOfflineMsg->clear_smsgid();
//		}
//	}
//	log("get content for offlineMsg use %llu useconds", elspedTimer.elapsed());
//}

static bool _isMaxLenLimitTouched(const MESOfflineMsgAck& offlineMsgAck, int maxLen)
{

    return offlineMsgAck.ByteSize() >= maxLen - 2000;
}

void COfflineMsgHandler::GetGrpOfflineMsgContent(MESOfflineMsgAck& offlineMsgAck, CGrpOfflineMsgMgr& grpOfflineMsgMgr, int maxLen)
{
    //CUsecElspsedTimer elspedTimer;
    //elspedTimer.start();
    int index = 0;
    std::set<string> msgIds;				//所有群聊天的消息的msgID, 不重复
    std::multimap<string, OfflineMsgData*> grpOfflineMsg;	//<msgId, offlineMsgData>pairs,管理offlineMsgAck中的群聊天离线消息部分，可重复
    std::multimap<string, string> allgrpMsgIds;//可重复

    //这里优化，只查询一次************************************************************
    for (index = 0; index < offlineMsgAck.msglist_size(); ++index)
    {
        OfflineMsgData* pOfflineMsg = offlineMsgAck.mutable_msglist(index);
        if (MES_GRPCHAT_DELIVER == pOfflineMsg->cmdid())
        {
            //msgIds.push_back(pOfflineMsg->smsgid());
            msgIds.insert(pOfflineMsg->smsgid());
            grpOfflineMsg.insert(std::pair<string, OfflineMsgData*>(pOfflineMsg->smsgid(), pOfflineMsg));//what is the defaul inserting order?
            allgrpMsgIds.insert(pair<string, string>(pOfflineMsg->sfromid(), pOfflineMsg->smsgid()));
        }
    }

    //bool isMaxLenLimitTouched = _isMaxLenLimitTouched(offlineMsgAck, maxLen);

    //查询每一个群组的信息
    typedef multimap<string, string>::iterator grpMsgIdsIter;
    //int grpCount = 0;
    bool isLimitTouched  = false;
    for (grpMsgIdsIter grpIdMsgIdIter = allgrpMsgIds.begin(); grpIdMsgIdIter != allgrpMsgIds.end(); grpIdMsgIdIter = allgrpMsgIds.upper_bound(grpIdMsgIdIter->first))
    {
        if (isLimitTouched) {
            break;
        }

        //get one grp all msg Ids once
        msgIds.clear();
        const string grpId = grpIdMsgIdIter->first;
        pair<grpMsgIdsIter, grpMsgIdsIter> oneGrpMsgIds = allgrpMsgIds.equal_range(grpId);
        for (grpMsgIdsIter k = oneGrpMsgIds.first; k != oneGrpMsgIds.second; k++)
        {
            //msgIds.push_back(k->second);
            msgIds.insert(k->second);
        }

        //get a grp all msg datas
        const std::vector<MsgIdContents> msgContents =
            grpOfflineMsgMgr.GetGrpOfflineMsgData(grpId, msgIds);

        //遍历查到到的单个群的消息内容，顺道清理offlinemsg collection里的无效记录
    //////////////clear invalid data;
        std::vector<MsgIdContents>::const_iterator idContentIter = msgContents.begin();
        while(idContentIter != msgContents.end()) {
            if (msgIds.find(idContentIter->msgId) != msgIds.end()) {
                msgIds.erase(idContentIter->msgId);
            }
            ++idContentIter;
        }

        std::set<string>::iterator it = msgIds.begin();
        while(it != msgIds.end()) {
            //这个 std::map<string, OfflineMsgData*>::const_iterator iter 用于删除dirty data
            std::multimap<string, OfflineMsgData*>::const_iterator iter = grpOfflineMsg.find(*it);
            OfflineMsgData *data = iter->second;
            int cmdId = data->cmdid();
            string toId = offlineMsgAck.stoid();
            m_offlineMsgMgr.DelOfflineMsg(toId, cmdId, *it, NULL, NULL);
            ++it;
        }
    ///////////////////////////
        //将群消息填充到离线消息中
        idContentIter = msgContents.begin();
        while (idContentIter != msgContents.end())
        {
            //这个 std::map<string, OfflineMsgData*>::const_iterator iter 用于填充数据
            std::multimap<string, OfflineMsgData*>::const_iterator iter = grpOfflineMsg.find((*idContentIter).msgId);
            if (iter != grpOfflineMsg.end())
            {
                iter->second->set_smsgdata((*idContentIter).content);
                 //如果长度已经到达最大值或者已经获取了超过6个群的离线消息，则直接清空群消息中未填充数据部分的数据;
                if (_isMaxLenLimitTouched(offlineMsgAck, maxLen))
                {
                    iter->second->clear_smsgdata();
                    //这个 std::map<string, OfflineMsgData*>::const_iterator iterator 用于删除msgdata字段为空的offlineMsgData
                    std::multimap<string, OfflineMsgData*>::const_iterator iterator = grpOfflineMsg.begin();
//                    while(iterator != grpOfflineMsg.end()&& iterator->second->smsgdata().empty()) {
//                        iterator->second->Clear();
//                        ++iterator;
//                    }
                      while(iterator != grpOfflineMsg.end()) {
                        if(iterator->second->smsgdata().empty()) {
                            iterator->second->Clear();
                        }
                        ++iterator;
                    }

                    isLimitTouched = true;
                    DbgLog("------------[offlineMsgAck.size = %d, >= (%d-%d)]-------------", offlineMsgAck.ByteSize(), maxLen,  2000);
                    break;
                }
            }
            ++idContentIter;
        }
    }
    //并重建msglist,保证其不带无内容元素，影响长度
//    const google::protobuf::RepeatedPtrField<::im::OfflineMsgData> msgDataList = offlineMsgAck.msglist();
//    DbgLog("<<<<<<<<<<<<<<google::protobuf::RepeatedPtrField<::im::OfflineMsgData> msgDataList size:<%d>>>>>>>>>>>>", msgDataList.size());
//    offlineMsgAck.clear_msglist();
//    for (int i = 0; i < msgDataList.size() ;i++) {
//        if(!msgDataList.Get(i).smsgdata().empty()) {
//           im::OfflineMsgData* msgdata = offlineMsgAck.add_msglist();
//           *msgdata = msgDataList.Get(i);
//        }
//    }

    return;
}

void OnGetOfflineMsgStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pMsg, void* paras)
{
    OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
    COfflineMsgHandler* pHandle = (COfflineMsgHandler*)pCallBackPara->m_handle;
    if (NULL == pHandle)
    {
        delete pCallBackPara;
        return;
    }

    const std::shared_ptr<im::MESOfflineMsg> pGetOfflineMsg = dynamic_pointer_cast<im::MESOfflineMsg>(pMsg);
    //const im::MESChat& msg = *pChat;

    pHandle->HandleGetOfflineMsg(*pGetOfflineMsg, pCallBackPara->m_sessionID);
    delete pCallBackPara;
}


void COfflineMsgHandler::HandleGetOfflineMsg(const im::MESOfflineMsg& msg, const UidCode_t& sessionId)
{
    CUsecElspsedTimer elspsedTimer;
    elspsedTimer.start();

	//process ack before pull data
	DbgLog("toID = %s,lsmsgs_size = %d",msg.stoid().c_str(),msg.lsmsgs_size());
	if( 0 < msg.lsmsgs_size() )
	{
		HandleOfflineMsgDelivered(msg);
	}

    int totals = 0;
    totals = m_offlineMsgMgr.GetUserOfflineMsgSummaryBytoId(msg.stoid());
    DbgLog("----------There are totally %d offlineMsgs for %s---------", totals , msg.stoid().c_str());

    MESOfflineMsgAck offlineMsgAck;

    if (totals > 0) {

        offlineMsgAck = m_offlineMsgMgr.GetUserOfflineMsg(msg.stoid());
        GetGrpOfflineMsgContent(offlineMsgAck, m_grpOfflineMsgMgr);

        //just for printf log
        for(int i = 0; i < offlineMsgAck.msglist().size(); i++)
        {
            const OfflineMsgData& data = offlineMsgAck.msglist(i);
            DbgLog("[offlineMsg to client]<<<<cmdid = %u, sfromid = %s, smsgid = %s, msgdataLen= %d>>>>", data.cmdid(),data.sfromid().c_str(), data.smsgid().c_str(), data.smsgdata().size());
        }
    } else {
        offlineMsgAck.set_stoid(msg.stoid());
        offlineMsgAck.set_errcode(NON_ERR);
        DbgLog("<<<<<<<<<<<<<[%s] has no offlinemsg>>>>>>>>>>", msg.stoid().c_str());
    }

    offlineMsgAck.set_smsgid(msg.smsgid());
    offlineMsgAck.set_msgtime(getCurrentTime());
    offlineMsgAck.set_msgtotal(totals);
    sendAck(&offlineMsgAck, MES_OFFLINEMSG_ACK, sessionId);

    DbgLog("****send MESOfflineMsgAck(0x%x)%s to %s,lsSize = %d, byteSize = %d", MES_OFFLINEMSG_ACK, offlineMsgAck.smsgid().c_str(), offlineMsgAck.stoid().c_str(),
           offlineMsgAck.msglist_size(), offlineMsgAck.ByteSize());
}

void COfflineMsgHandler::OnGetOfflineMsg(std::shared_ptr<CImPdu> pPdu)
{
    assert(NULL != pPdu);
    std::shared_ptr<im::MESOfflineMsg> pMsg(new im::MESOfflineMsg);
    if (!pMsg) return;
    MESOfflineMsg& msg = *pMsg;
    //MESOfflineMsg msg;
    if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        return;
    }

    if (/*msg.sfromid().empty() ||*/ msg.stoid().empty() || msg.smsgid().empty())
    {
        ErrLog("!!!lack of required parameter");
        return;
    }

    if (!CCommonTaskMgr::InsertCommonTask(pMsg, OnGetOfflineMsgStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId())))
    {
        WarnLog("!!!create thread task failed");
    }

    log("\r\n\r\n\r\nMESOfflineMsg(0x%x) %s prehandled, request %s-->%s's %s(0x%x) msg",
        pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str(), getofflineTypeStr(msg.cmdid()), msg.cmdid());
}

void OfflineMsgChatDeliveredNotifyInserted(const std::shared_ptr<IMongoDataEntry>& pOfflineMsg, bool bInsertSuccess, void* paras)
{
    COfflineMsgHandler* pHandle = (COfflineMsgHandler*)paras;
    if (pHandle)
    {
        const std::shared_ptr<COfflineMsg> pMsg = dynamic_pointer_cast<COfflineMsg>(pOfflineMsg);
        pHandle->OnOfflineMsgChatDeliverNotifyInserted(*pMsg, bInsertSuccess);
    }
}


void OnOfflineMsgDeliveredStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pMsg, void* paras)
{
    OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
    COfflineMsgHandler* pHandle = (COfflineMsgHandler*)pCallBackPara->m_handle;
    if (NULL == pHandle)
    {
        delete pCallBackPara;
        return;
    }

    const std::shared_ptr<im::MESOfflineMsgDelivered> pOfflineMsgDeliveredMsg = dynamic_pointer_cast<im::MESOfflineMsgDelivered>(pMsg);
    //const im::MESChat& msg = *pChat;

    pHandle->HandleOfflineMsgDelivered(*pOfflineMsgDeliveredMsg, pCallBackPara->m_sessionID);
    delete pCallBackPara;
}

class _DeleteKey
{
public:
    _DeleteKey(const string& toid, google::protobuf::uint32 cmdId) :m_toId(toid), m_cmdId(cmdId){}
    bool operator<(const _DeleteKey BObj) const{
        return m_toId < BObj.m_toId || (m_toId == BObj.m_toId && m_cmdId < BObj.m_cmdId);
    }
    const string& getToId()const { return m_toId; }
    const string& getToId(){ return m_toId; }
    int getCmdId()const{ return m_cmdId; }
private:
    const string m_toId;
    google::protobuf::uint32	m_cmdId;
};


void COfflineMsgHandler::HandleOfflineMsgDelivered(const im::MESOfflineMsgDelivered& msg, const UidCode_t& sessionId)
{
    CUsecElspsedTimer elspsedTimer;
    elspsedTimer.start();

    //针对MESOfflineMsgDelivered中已经确认送达的MES_CHAT_DELIVER消息，发送MESofflineMsgDeliveredNotify。
    MESOfflineMsgDelivered offlineMsgDeliveredNotify;
    offlineMsgDeliveredNotify.set_sfromid(msg.sfromid());
    offlineMsgDeliveredNotify.set_smsgid(msg.smsgid());

    //记录确认收到了每一个toId对应有多少条MES_CHAT_DELIVER,需要产生MES_CHAT_DELIVERED_NOTIFICATION
    std::multimap<string, int> msgChatDeliverMap;		//<usrId, index>键值对, MES_CHAT_DELIVER的的发起人usrId（需要向其发送notify）和其在MESOfflineMsgDelivered中的index值

    std::multimap<_DeleteKey, string> DeleteOffLineMsgsMap;		//需要删除的离线消息

    //
    im::ErrCode retCode = NON_ERR;
    int msgSize = msg.lsmsgs_size();
    for (int index = 0; index < msgSize; ++index)
    {
        const OfflineDeliveredMsg& offLineMsg = msg.lsmsgs(index);

        //只有msgChatDeliver这样的离线消息响应后才会生成新的离线消息，其他的离线消息收到响应后直接删除掉
        if (offLineMsg.cmdid() != MES_CHAT_DELIVER)	//
        {
            //DbgLog("delete from mongo, toId = %s cmdid = %X and msg id = %s", offLineMsg.stoid().c_str(), offLineMsg.cmdid(), offLineMsg.smsgid().c_str());
            //m_offlineMsgMgr.DelOfflineMsg(offLineMsg.stoid(), offLineMsg.cmdid(), offLineMsg.smsgid());
            DeleteOffLineMsgsMap.insert(pair<_DeleteKey, string>(_DeleteKey(offLineMsg.stoid(), offLineMsg.cmdid()), offLineMsg.smsgid()));
        }
        else
        {
            //生成消息已送达（MES_CHAT_DELIVERED_NOTIFICATION）通知
            //typedef  MESChatDeliveredAck MESChatDelivereNotify;
            /*MESChatDeliveredAck notifymsg;
            notifymsg.set_sfromid(offLineMsg.stoid());
            notifymsg.set_stoid(offLineMsg.sfromid());
            notifymsg.set_smsgid(offLineMsg.smsgid());
            m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(notifymsg), OfflineMsgChatDeliveredNotifyInserted, this);*/

            msgChatDeliverMap.insert(pair<string, int>(offLineMsg.sfromid(), index));
        }
    }

    //如果是非MES_CHAT_DELIVER消息，直接批量删除
    typedef multimap<_DeleteKey, string>::iterator DeleteOffLineMsgsMapIter;
    vector<string> msgIds;
    for (DeleteOffLineMsgsMapIter deleteMsgIter = DeleteOffLineMsgsMap.begin(); deleteMsgIter != DeleteOffLineMsgsMap.end(); deleteMsgIter = DeleteOffLineMsgsMap.upper_bound(deleteMsgIter->first))
    {
        msgIds.clear();
        pair<DeleteOffLineMsgsMapIter, DeleteOffLineMsgsMapIter> deleteMsgIds = DeleteOffLineMsgsMap.equal_range(deleteMsgIter->first);
        for (DeleteOffLineMsgsMapIter k = deleteMsgIds.first; k != deleteMsgIds.second; k++)
        {
            msgIds.push_back(k->second);
        }
        m_offlineMsgMgr.DelOfflineMsg(deleteMsgIter->first.getToId(), deleteMsgIter->first.getCmdId(), msgIds);
    }

    /* send MESOfflineMsgDelivereddAck to sender */
    MESOfflineMsgDelivereddAck deliveredAck;
    deliveredAck.set_smsgid(msg.smsgid());
    deliveredAck.set_suserid(msg.sfromid());
    deliveredAck.set_errcode(retCode);
    sendAck(&deliveredAck, MES_OFFLINEMSG_DELIVERED_ACK, sessionId);
    log("****send MESOfflineMsgDeliveredAck(0x%x) %s to %s", MES_OFFLINEMSG_DELIVERED_ACK,
        deliveredAck.smsgid().c_str(), deliveredAck.suserid().c_str());


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //处理MESOfflineMsgDelivered中repeated部分的MES_CHAT_DELIVER消息

    //查询每一个需要发送离线消息送达(chatDelivereNotify)的userId
    typedef multimap<string, int>::iterator msgChatDeliverMapIter;
    for (msgChatDeliverMapIter deliveredMsgIter = msgChatDeliverMap.begin(); deliveredMsgIter != msgChatDeliverMap.end(); deliveredMsgIter = msgChatDeliverMap.upper_bound(deliveredMsgIter->first))
    {
        //get one grp all msg Ids once
        offlineMsgDeliveredNotify.clear_lsmsgs();
        std::vector<COfflineMsg> chatDelivereNotifyMsgs;
        std::vector<string> chatDeliveredMsgIds;
        //chatDelivereNotifyMsgs.clear();
        //chatDeliveredMsgIds.clear();

        const string toId = deliveredMsgIter->first;
        bool bSendDeliverNofityReq = true;		//判断是否需要发送0xb01a
        std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(toId);
        if (!pLogin || !pLogin->IsLogin())
            bSendDeliverNofityReq = false;

        pair<msgChatDeliverMapIter, msgChatDeliverMapIter> userDeliveredMsgIds = msgChatDeliverMap.equal_range(toId);
        for (msgChatDeliverMapIter k = userDeliveredMsgIds.first; k != userDeliveredMsgIds.second; k++)
        {
            //对于已经拉取成功的0xb004（MES_CHAT_DELIVER）, 生成0xb006(MES_CHAT_DELIVERED_NOTIFICATION)保存到mongo
            const OfflineDeliveredMsg& offLineMsg = msg.lsmsgs(k->second);
            MESChatDeliveredAck notifymsg;
            notifymsg.set_sfromid(offLineMsg.stoid());
            notifymsg.set_stoid(offLineMsg.sfromid());
            notifymsg.set_smsgid(offLineMsg.smsgid());
            chatDelivereNotifyMsgs.push_back(COfflineMsg(notifymsg));	//添加要插入的0xb006

            chatDeliveredMsgIds.push_back(offLineMsg.smsgid());			//记录要删除的0xb004的msgId

            if (bSendDeliverNofityReq)
            {
                OfflineDeliveredMsg* pMsg = offlineMsgDeliveredNotify.add_lsmsgs();
                *pMsg = offLineMsg;
                pMsg->set_cmdid(MES_CHAT_DELIVERED_NOTIFICATION);
            }
        }

        //如果0xb006插入成功，则删除0xb004
        if (chatDelivereNotifyMsgs.size() > 0 && m_offlineMsgMgr.InsertOfflineMsg(chatDelivereNotifyMsgs))
        {
            m_offlineMsgMgr.DelOfflineMsg(msg.sfromid(), MES_CHAT_DELIVER, chatDeliveredMsgIds);
        }

        if (!bSendDeliverNofityReq)
        {
            continue;
        }
        offlineMsgDeliveredNotify.set_stoid(toId);
        DbgLog("msg deliver notify size = %d", offlineMsgDeliveredNotify.lsmsgs_size());
        sendReq(&offlineMsgDeliveredNotify, MES_OFFLINEMSG_DELIVERED_NOTIFICATION,
            pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
        log("****send MES_OFFLINEMSG_DELIVERED_NOTIFICATION(0x%x) %s to %s,lsSize = %d", MES_OFFLINEMSG_DELIVERED_NOTIFICATION,
            offlineMsgDeliveredNotify.smsgid().c_str(), offlineMsgDeliveredNotify.stoid().c_str(), offlineMsgDeliveredNotify.lsmsgs_size());
    }

    DbgLog("handle OfflineMsgDelivered %s use %lu useconds", msg.smsgid().c_str() , elspsedTimer.elapsed());
}



void COfflineMsgHandler::HandleOfflineMsgDelivered(const im::MESOfflineMsg& msg)
{
    CUsecElspsedTimer elspsedTimer;
    elspsedTimer.start();

    //针对MESOfflineMsgDelivered中已经确认送达的MES_CHAT_DELIVER消息，发送MESofflineMsgDeliveredNotify。
    MESOfflineMsgDelivered offlineMsgDeliveredNotify;
    offlineMsgDeliveredNotify.set_sfromid(msg.stoid());
    offlineMsgDeliveredNotify.set_smsgid(msg.smsgid());

    //记录确认收到了每一个toId对应有多少条MES_CHAT_DELIVER,需要产生MES_CHAT_DELIVERED_NOTIFICATION
    std::multimap<string, int> msgChatDeliverMap;		//<usrId, index>键值对, MES_CHAT_DELIVER的的发起人usrId（需要向其发送notify）和其在MESOfflineMsgDelivered中的index值

    std::multimap<_DeleteKey, string> DeleteOffLineMsgsMap;		//需要删除的离线消息

    //
    im::ErrCode retCode = NON_ERR;
    int msgSize = msg.lsmsgs_size();

	
    for (int index = 0; index < msgSize; ++index)
    {
        const OfflineDeliveredMsg& offLineMsg = msg.lsmsgs(index);
		DbgLog("delete from mongo, toId = %s cmdid = %X and msg id = %s", offLineMsg.stoid().c_str(), offLineMsg.cmdid(), offLineMsg.smsgid().c_str());
        //只有msgChatDeliver这样的离线消息响应后才会生成新的离线消息，其他的离线消息收到响应后直接删除掉
        if (offLineMsg.cmdid() != MES_CHAT_DELIVER)	//
        {
            //DbgLog("delete from mongo, toId = %s cmdid = %X and msg id = %s", offLineMsg.stoid().c_str(), offLineMsg.cmdid(), offLineMsg.smsgid().c_str());
            //m_offlineMsgMgr.DelOfflineMsg(offLineMsg.stoid(), offLineMsg.cmdid(), offLineMsg.smsgid());
            DeleteOffLineMsgsMap.insert(pair<_DeleteKey, string>(_DeleteKey(offLineMsg.stoid(), offLineMsg.cmdid()), offLineMsg.smsgid()));
        }
        else
        {
            //生成消息已送达（MES_CHAT_DELIVERED_NOTIFICATION）通知
            //typedef  MESChatDeliveredAck MESChatDelivereNotify;
            /*MESChatDeliveredAck notifymsg;
            notifymsg.set_sfromid(offLineMsg.stoid());
            notifymsg.set_stoid(offLineMsg.sfromid());
            notifymsg.set_smsgid(offLineMsg.smsgid());
            m_offlineMsgMgr.InsertOfflineMsg(COfflineMsg(notifymsg), OfflineMsgChatDeliveredNotifyInserted, this);*/

            msgChatDeliverMap.insert(pair<string, int>(offLineMsg.sfromid(), index));
        }
    }

    //如果是非MES_CHAT_DELIVER消息，直接批量删除
    typedef multimap<_DeleteKey, string>::iterator DeleteOffLineMsgsMapIter;
    vector<string> msgIds;
    for (DeleteOffLineMsgsMapIter deleteMsgIter = DeleteOffLineMsgsMap.begin(); deleteMsgIter != DeleteOffLineMsgsMap.end(); deleteMsgIter = DeleteOffLineMsgsMap.upper_bound(deleteMsgIter->first))
    {
        msgIds.clear();
        pair<DeleteOffLineMsgsMapIter, DeleteOffLineMsgsMapIter> deleteMsgIds = DeleteOffLineMsgsMap.equal_range(deleteMsgIter->first);
        for (DeleteOffLineMsgsMapIter k = deleteMsgIds.first; k != deleteMsgIds.second; k++)
        {
            msgIds.push_back(k->second);
        }
        m_offlineMsgMgr.DelOfflineMsg(deleteMsgIter->first.getToId(), deleteMsgIter->first.getCmdId(), msgIds);
    }

    /* send MESOfflineMsgDelivereddAck to sender */
    //MESOfflineMsgDelivereddAck deliveredAck;
    //deliveredAck.set_smsgid(msg.smsgid());
    //deliveredAck.set_suserid(msg.sfromid());
    //deliveredAck.set_errcode(retCode);
    //sendAck(&deliveredAck, MES_OFFLINEMSG_DELIVERED_ACK, sessionId);
    //log("****send MESOfflineMsgDeliveredAck(0x%x) %s to %s", MES_OFFLINEMSG_DELIVERED_ACK,
    //    deliveredAck.smsgid().c_str(), deliveredAck.suserid().c_str());


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //处理MESOfflineMsgDelivered中repeated部分的MES_CHAT_DELIVER消息

    //查询每一个需要发送离线消息送达(chatDelivereNotify)的userId
    typedef multimap<string, int>::iterator msgChatDeliverMapIter;
    for (msgChatDeliverMapIter deliveredMsgIter = msgChatDeliverMap.begin(); deliveredMsgIter != msgChatDeliverMap.end(); deliveredMsgIter = msgChatDeliverMap.upper_bound(deliveredMsgIter->first))
    {
        //get one grp all msg Ids once
        offlineMsgDeliveredNotify.clear_lsmsgs();
        std::vector<COfflineMsg> chatDelivereNotifyMsgs;
        std::vector<string> chatDeliveredMsgIds;
        //chatDelivereNotifyMsgs.clear();
        //chatDeliveredMsgIds.clear();

        const string toId = deliveredMsgIter->first;
        bool bSendDeliverNofityReq = true;		//判断是否需要发送0xb01a
        std::shared_ptr<CLoginInfo> pLogin = CLoginInfoMgr::GetLoginInfo(toId);
        if (!pLogin || !pLogin->IsLogin())
            bSendDeliverNofityReq = false;

        pair<msgChatDeliverMapIter, msgChatDeliverMapIter> userDeliveredMsgIds = msgChatDeliverMap.equal_range(toId);
        for (msgChatDeliverMapIter k = userDeliveredMsgIds.first; k != userDeliveredMsgIds.second; k++)
        {
            //对于已经拉取成功的0xb004（MES_CHAT_DELIVER）, 生成0xb006(MES_CHAT_DELIVERED_NOTIFICATION)保存到mongo
            const OfflineDeliveredMsg& offLineMsg = msg.lsmsgs(k->second);
            MESChatDeliveredAck notifymsg;
            notifymsg.set_sfromid(offLineMsg.stoid());
            notifymsg.set_stoid(offLineMsg.sfromid());
            notifymsg.set_smsgid(offLineMsg.smsgid());
            chatDelivereNotifyMsgs.push_back(COfflineMsg(notifymsg));	//添加要插入的0xb006

            chatDeliveredMsgIds.push_back(offLineMsg.smsgid());			//记录要删除的0xb004的msgId

            if (bSendDeliverNofityReq)
            {
                OfflineDeliveredMsg* pMsg = offlineMsgDeliveredNotify.add_lsmsgs();
                *pMsg = offLineMsg;
                pMsg->set_cmdid(MES_CHAT_DELIVERED_NOTIFICATION);
            }
        }

        //如果0xb006插入成功，则删除0xb004
        if (chatDelivereNotifyMsgs.size() > 0 && m_offlineMsgMgr.InsertOfflineMsg(chatDelivereNotifyMsgs))
        {
            m_offlineMsgMgr.DelOfflineMsg(msg.stoid(), MES_CHAT_DELIVER, chatDeliveredMsgIds);
        }

        if (!bSendDeliverNofityReq)
        {
            continue;
        }
        offlineMsgDeliveredNotify.set_stoid(toId);
        DbgLog("msg deliver notify size = %d", offlineMsgDeliveredNotify.lsmsgs_size());
        sendReq(&offlineMsgDeliveredNotify, MES_OFFLINEMSG_DELIVERED_NOTIFICATION,
            pLogin->GetCmIp(), atoi(pLogin->GetCmPort().c_str()));
        log("****send MES_OFFLINEMSG_DELIVERED_NOTIFICATION(0x%x) %s to %s,lsSize = %d", MES_OFFLINEMSG_DELIVERED_NOTIFICATION,
            offlineMsgDeliveredNotify.smsgid().c_str(), offlineMsgDeliveredNotify.stoid().c_str(), offlineMsgDeliveredNotify.lsmsgs_size());
    }

    DbgLog("handle OfflineMsgDelivered %s use %lu useconds", msg.smsgid().c_str() , elspsedTimer.elapsed());
}

void COfflineMsgHandler::onOfflineMsgDelivered(std::shared_ptr<CImPdu> pPdu)
{
    assert(NULL != pPdu);
    std::shared_ptr<im::MESOfflineMsgDelivered> pMsg(new im::MESOfflineMsgDelivered);
    if (!pMsg) return;
    MESOfflineMsgDelivered& msg = *pMsg;
    //MESOfflineMsgDelivered msg;
    if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        ErrLog("MESOfflineMsgDelivered para error");
        return;
    }

    if (msg.sfromid().empty()/* || msg.stoid().empty()*/)
    {
        ErrLog("!!!lack of required parameter");
        return;
    }

    CCommonTaskMgr::InsertCommonTask(pMsg, OnOfflineMsgDeliveredStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()));
    log("\r\n\r\n\r\nMESOfflineMsgDelivered(0x%x) %s prehandled, dir:%s-->%s",
        pPdu->GetCommandId()/*MES_OFFLINEMSG_DELIVERED*/, msg.smsgid().c_str(), msg.sfromid().c_str(), msg.stoid().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OnOfflineMsgDeliveredNotifyAckStartUp(const std::shared_ptr<::google::protobuf::MessageLite>& pTask, void* paras)
{
    OfflineMsgInsertCallBackParas_t* pCallBackPara = (OfflineMsgInsertCallBackParas_t*)paras;
    COfflineMsgHandler* pHandle = (COfflineMsgHandler*)pCallBackPara->m_handle;
    if (NULL == pHandle)
    {
        delete pCallBackPara;
        return;
    }

    const std::shared_ptr<im::MESOfflineMsgDeliveredNotifyAck> pChatReadDeliverAck = dynamic_pointer_cast<im::MESOfflineMsgDeliveredNotifyAck>(pTask);

    pHandle->HandleOfflineMsgDeliveredNotifyAckTask(*pChatReadDeliverAck);
    delete pCallBackPara;
}

void COfflineMsgHandler::HandleOfflineMsgDeliveredNotifyAckTask(const im::MESOfflineMsgDeliveredNotifyAck& msg)
{
    std::multimap<int, string> cmdMsgIdMap;
    for (int index = 0; index < msg.lsmsgs_size(); ++index)
    {
        const OfflineDeliveredMsg& offLineMsg = msg.lsmsgs(index);
        cmdMsgIdMap.insert(pair<int, string>(offLineMsg.cmdid(), offLineMsg.smsgid()));
        /*m_offlineMsgMgr.DelOfflineMsg(msg.sfromid(), offLineMsg.cmdid(), offLineMsg.smsgid());*/
    }
    typedef multimap<int, string>::iterator CmdMsgIdMapIter;
    for (CmdMsgIdMapIter iter = cmdMsgIdMap.begin(); iter != cmdMsgIdMap.end(); iter = cmdMsgIdMap.upper_bound(iter->first))
    {
        std::vector<string> MsgIds;
        const int cmdId = iter->first;

        pair<CmdMsgIdMapIter, CmdMsgIdMapIter> specifiedCmdMsgIds = cmdMsgIdMap.equal_range(cmdId);
        for (CmdMsgIdMapIter k = specifiedCmdMsgIds.first; k != specifiedCmdMsgIds.second; k++)
        {
            MsgIds.push_back(k->second);
        }

        //如果0xb006插入成功，则删除0xb004
        if (MsgIds.size() > 0)
        {
            m_offlineMsgMgr.DelOfflineMsg(msg.sfromid(), cmdId, MsgIds);
        }
    }
}


void COfflineMsgHandler::onOfflineMsgDeliveredNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
    assert(NULL != pPdu);
    std::shared_ptr<im::MESOfflineMsgDeliveredNotifyAck> pMsg(new im::MESOfflineMsgDeliveredNotifyAck);
    if (!pMsg) return;
    MESOfflineMsgDeliveredNotifyAck& msg = *pMsg;
    //MESOfflineMsgDeliveredNotifyAck msg;
    if (!pPdu->GetBodyData() || !msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        return;
    }
    CCommonTaskMgr::InsertCommonTask(pMsg, OnOfflineMsgDeliveredNotifyAckStartUp, new OfflineMsgInsertCallBackParas_t(this, pPdu->GetSessionId()), BKDRHash(msg.sfromid().c_str()));
    log("\r\n\r\n\r\nMESOfflineMsgDeliveredNotifyAck(0x%x) %s prehandled,from %s, lsSize = %d",
        pPdu->GetCommandId(), msg.smsgid().c_str(), msg.sfromid().c_str(), msg.lsmsgs_size());
}

bool COfflineMsgHandler::RegistPacketExecutor(void)
{
    CmdRegist(MES_OFFLINESUMMARY,			m_nNumberOfInst,  CommandProc(&COfflineMsgHandler::OnMsgOfflineSummary));
    CmdRegist(MES_OFFLINETOTAL,				m_nNumberOfInst,  CommandProc(&COfflineMsgHandler::OnMsgOfflineTotal));
    CmdRegist(MES_OFFLINEMSG,				m_nNumberOfInst,  CommandProc(&COfflineMsgHandler::OnGetOfflineMsg));
    CmdRegist(MES_OFFLINEMSG_DELIVERED,		m_nNumberOfInst,  CommandProc(&COfflineMsgHandler::onOfflineMsgDelivered));
    CmdRegist(MES_OFFLINEMSG_DELIVERED_NOTIFICATION_ACK, m_nNumberOfInst,  CommandProc(&COfflineMsgHandler::onOfflineMsgDeliveredNotifyAck));
    return true;
}

void COfflineMsgHandler::OnOfflineMsgChatDeliverNotifyInserted(const COfflineMsg& offlineMsg, bool bInsertSuccess)
{
    if (bInsertSuccess)
    {
        m_offlineMsgMgr.DelOfflineMsg(offlineMsg.GetFromId(), MES_CHAT_DELIVER, offlineMsg.GetMsgId());
    }
}
