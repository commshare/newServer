/******************************************************************************
Filename: im_friend.cpp
Author:TongHuaizhi 			Version:im-1.0 		Date:2017/07/05
Description: 
******************************************************************************/
#include "im_friend.h"
#include "im.mes.pb.h"

CFriendRelation::CFriendRelation(const im::MESAddFriend& addFriendMsg)
	:m_sUserId(addFriendMsg.sfromid()), m_sFrinedId(addFriendMsg.stoid()), m_sMemoName(addFriendMsg.smemoname()), m_sDesc(addFriendMsg.sdesc()),
	 m_nState(FRIEND_RELATION_STATE_ADDING), m_nPacketId(addFriendMsg.packetid())/*, m_sSelfIntroduce(addFriendMsg.sselfintroduce())*/
{

}

CFriendRelation::CFriendRelation(const string& userId, const string& friendId, uint16_t state
	, uint16_t notifyMsgDelivered, uint16_t soundOff , uint16_t hideFlag, const string& selfIntroduce, const string & memoName, const string& desc, uint16_t packetId)
	:m_sUserId(userId), m_sFrinedId(friendId), m_sMemoName(memoName), m_sDesc(desc), m_nState(state),
	m_nPacketId(packetId), m_nNotifyMsgDelivered(notifyMsgDelivered), m_sSelfIntroduce(selfIntroduce), m_nSoundOff(soundOff), m_nHidenFlag(hideFlag)
{

}

CFriendRelation::~CFriendRelation()
{

}
