#include "groupnotify.h"
#include "im.mes.pb.h"
#include "im_grpmem.h"
#include "pdusender.h"
#include "redisGrpMgr.h"
#include "mysqlGrpmemMgr.h"
#include "im_time.h"
#include "grpOfflineMsgMgr.h"
#include "commonTaskMgr.h"


CGroupNotify::CGroupNotify(CConfigFileReader* pConfigReader, int nNumOfInst)
	: CBaseHandle(pConfigReader), m_nNumberOfInst(nNumOfInst)
{
}

CGroupNotify::~CGroupNotify()
{

}

bool CGroupNotify::Initialize(void)
{

	RegistPacketExecutor();
	StartThread();
	
	return true;
}

bool CGroupNotify::RegistPacketExecutor(void)	 //Regist command process function to network frame. 
{
	CmdRegist(im::MES_GRPINTERNOTIFY_ACK, m_nNumberOfInst, CommandProc(&CGroupNotify::OnNotifyAck));

	return true;
}

bool CGroupNotify::OnNotifyAck(std::shared_ptr<CImPdu> pPdu)
{
	//static int notifyAckCount = 0;
	//DbgLog("notifyAck received %d times", ++notifyAckCount);
	return true;
}

//int CGroupNotify::sendNotify(const string& grpId, const string& toId, im::NotifyType notifyType, const string& content, 
//	im::ErrCode errcode, const string& oprUserId /*= string("")*/, const std::vector<string>& mnpledUserId /*= std::vector<string>()*/)
//{
//	//im::MESGrpNotify notify;
//	im::MESGrpNotify grpNotify;
//	grpNotify.set_sgrpid(grpId);
//	grpNotify.set_stoid(toId);
//	grpNotify.set_notifytype(notifyType);
//	string msgId = getuuid();
//	transform(msgId.begin(), msgId.end(), msgId.begin(), ::toupper);
//	grpNotify.set_smsgid(msgId);
//	grpNotify.set_msgtime(getCurrentTime());
//	grpNotify.set_scontent(content);
//	grpNotify.set_sopruserid(oprUserId);
//	for (unsigned int i = 0; i < mnpledUserId.size(); ++i)
//	{
//		std::string * pId = grpNotify.add_smnpleduserid();
//		*pId = mnpledUserId[i];
//	}
//	//grpNotify.set_smnpleduserid(mnpledUserId);
//	grpNotify.set_errcode(errcode);
//
//	return sendNotify(grpNotify);
//}

void _sendNotify(const std::shared_ptr<::google::protobuf::MessageLite>& pNotifyTask, void* data)
{
	//static int globalIndex = 0;

	CUsecElspsedTimer elspsedTimer;
	elspsedTimer.start();

	const std::shared_ptr<im::_InnerGrpNotify> pNotify = dynamic_pointer_cast<im::_InnerGrpNotify>(pNotifyTask);
	const im::_InnerGrpNotify& notify = *pNotify;
	DbgLog("grp %s at time %llu usecond start send notify, msgId = %s", notify.sgrpid().c_str(), getCurrentTime_usec(), notify.smsgid().c_str());

	//cp info to MESGrpNotify except for toId
	im::_MESGrpInterNotify notifyToMem;
	notifyToMem.set_sgrpid(notify.sgrpid());
	notifyToMem.set_smsgid(notify.smsgid());
    notifyToMem.set_msgtime(notify.msgtime() ? notify.msgtime() : getCurrentTime());//set server time
	notifyToMem.set_scontent(notify.scontent());
	notifyToMem.set_notifytype(notify.notifytype());
	//notifyToMem.set_errcode(im::NON_ERR/*notify.errcode()*/);
	notifyToMem.set_errcode(notify.errcode());
	notifyToMem.set_extend(notify.extend());

	if (!notify.sopruserid().empty())
	{
		notifyToMem.set_sopruserid(notify.sopruserid());
	}

	for (int i = 0; i < notify.smnpleduserid_size(); i++)
	{
		string *smnpleduserid = notifyToMem.add_smnpleduserid();
		*smnpleduserid = notify.smnpleduserid(i);
	}

	//只给指定的用户发送通知
	if (/*im::NOTIFY_TYPE_GRPINVITE_RESULT == notify.notifytype() ||*/ 
		im::NOTIFY_TYPE_GRPAPPLY_RESULT == notify.notifytype() /*|| im::NOTIFY_TYPE_GRPMASTER_CHANGED == notify.notifytype()*/ || im::NOTIFY_TYPE_GRP_ACTIVE == notify.notifytype())
	{
		if (0 == notify.lstoid_size()) return;
		for (int index = 0; index < notify.lstoid_size(); index++)
		{
			notifyToMem.add_stoid(notify.lstoid(index));			
		}
		CPduSender::getInstance()->sendReq(&notifyToMem, im::MES_GRPINTERNOTIFY, imsvr::MSG);
		return ;
	}
	
	/*****                   toID是群组ID，需要发送通知给所有的群成员         ************/
	//获取群中所有的成员信息
	std::map<std::string, CGrpMem> grpmems = GetGrpMems(notify.sgrpid());
	//需要发送通知给被移除的成员
	if (im::NOTIFY_TYPE_GRPMEM_REMOVE == notify.notifytype())
	{
		for (int i = 0; i < notify.smnpleduserid_size(); i++)
		{
			//因为后面要判断状态，故临时给一个有效状态
			grpmems.insert(std::pair<std::string, CGrpMem>(
				notify.smnpleduserid(i), CGrpMem(notify.sgrpid(), notify.smnpleduserid(i), CGrpMem::GRP_MEM_STATE_APPLY_MEMBER)));
		}
	}

	//已经成功的获取了所有的要发送的成员列表
	std::map<std::string, CGrpMem>::const_iterator iter = grpmems.begin();
	while (iter != grpmems.end())
	{
		//之前已经退出的成员不会发送通知
		if ((int)CGrpMem::GRP_MEM_STATE_QUIT_MEMBER == iter->second.GetState())
		{
			++iter;
			continue;
		}
		//const string& toId = iter->second.GetMemId();
		//notifyToMem.set_stoid(toId);
		notifyToMem.add_stoid(iter->second.GetMemId());

		if (notifyToMem.stoid_size() >= 500)
		{
			int retCode = CPduSender::getInstance()->sendReq(&notifyToMem, im::MES_GRPINTERNOTIFY, imsvr::MSG);
			if (retCode < 0)
			{
				//添加任务，重发
				ErrLog("!!!!grp %s dispatch grpNotify(0x%x) %s failed", notifyToMem.sgrpid().c_str(),
					MES_GRPINTERNOTIFY, notifyToMem.smsgid().c_str());
			}
			notifyToMem.clear_stoid();
		}
		++iter;
	}

	if (notifyToMem.stoid_size() > 0)
	{
		int retCode = CPduSender::getInstance()->sendReq(&notifyToMem, im::MES_GRPINTERNOTIFY, imsvr::MSG);
		if (retCode < 0)
		{
			//添加任务，重发
			ErrLog("!!!!grp %s dispatch grpNotify(0x%x) %s failed", notifyToMem.sgrpid().c_str(),
				MES_GRPINTERNOTIFY, notifyToMem.smsgid().c_str());
		}
		notifyToMem.clear_stoid();
	}

}

int CGroupNotify::sendNotify(const im::_InnerGrpNotify& notify)
{
	return CCommonTaskMgr::InsertCommonTask(std::shared_ptr<::google::protobuf::MessageLite>(new im::_InnerGrpNotify(notify)), _sendNotify, NULL) ? 0 : -1;
}


