/******************************************************************************
Filename: im_friend.h
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/05
Description: 
******************************************************************************/
#ifndef __IM_FRIEND_H__
#define __IM_FRIEND_H__

#include <string>
#include "ostype.h"
using std::string;

namespace im
{
	class MESAddFriend;
}

class  CFriendRelation
{
public:
	enum FRIEND_STATE
	{
		FRIEND_RELATION_STATE_ADDING = 0x01,		//正在添加
		FRIEND_RELATION_STATE_NORMAL = 0x02,		//正常
		FRIEND_RELATION_STATE_REFUSED = 0x04,		//被拒
		FRIEND_RELATION_STATE_BLOCKED = 0x08,		//已拉黑
		FRIEND_RELATION_STATE_DELETED = 0x10,		//已删除
		FRIEND_RELATION_STATE_VERIFYING = 0x20,		//验证中，主要用于换机后拉取他人添加为好友请求
		FRIEND_RELATION_STATE_REFUSE = 0x40			//验证,已拒绝添加好友
	};
public:
	 CFriendRelation(const im::MESAddFriend& addFriendMsg);
	 CFriendRelation(const string& userId, const string& friendId, uint16_t state = 0, uint16_t notifyMsgDelivered = 1,
		 uint16_t soundOff = 0, uint16_t hideFlag = 0, const string& selfIntroduce = "", const string & memoName = "", const string& desc = "", uint16_t packetId = 0);
	 ~CFriendRelation();

	 const string&	GetUserId()const{ return m_sUserId; }
	 const string&	GetFriendId()const{ return m_sFrinedId; }
	 const string&	GetMemoName()const{ return m_sMemoName; }
	 const string&	GetDesc()const{ return m_sDesc; }
	 uint16_t		GetState()const{ return m_nState; }
	 uint16_t		GetNotifyMsgDelivered()const{ return m_nNotifyMsgDelivered; }
	 int32_t		GetPacketId()const{ return m_nPacketId; }
	 const string&  GetSelfIntroduce()const{ return m_sSelfIntroduce; }
	 bool			IsFlagSet(FRIEND_STATE state)const { return m_nState & state; }
	 void			SetState(uint16_t state){ m_nState = state; }

	 bool			IsHidenModel()const{ return m_nHidenFlag; }
	 bool			IsNoInterruption()const{ return m_nSoundOff; }
private:
	const string		m_sUserId;
	const string		m_sFrinedId;
	const string		m_sMemoName = "";			//昵称
	const string		m_sDesc = "";				//备注
	uint16_t			m_nState;	 
	const int32_t		m_nPacketId = -1;			// 好友所在分组
	const uint16_t		m_nNotifyMsgDelivered = 1;	// 是否通知对方消息已达
	const string		m_sSelfIntroduce = "";
	const uint16_t		m_nSoundOff = 0;	// 是否设置了消息免打扰
	const uint16_t		m_nHidenFlag = 0;		// 是否是隐藏联系人模式
};
#endif // __IM_FRIEND_H__
