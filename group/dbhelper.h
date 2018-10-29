#ifndef __DBHELPER_H__
#define __DBHELPER_H__
#include <string.h>
#include <stdlib.h>
#include <tr1/unordered_map>
#include "encdec.h"

#include "configfilereader.h"
#include "threadpool.h"
#include "task.h"
#include "packet.h"
#include "redishelper.h"
#include "mysqlhelper.h"
#include "im.pub.pb.h"
#include "im.group.pb.h"

using namespace std;
using namespace std::tr1;
using namespace im;

#define SQL_BATCH_BUFSIZE 4*1024*1024
#define PREFIX_CMCACHE "CM_"
#define PREFIX_GRPCACHE "GRP_"
#define PREFIX_GRPMEMBERCACHE "GRPM_"
#define GROUP_COUNT_INC 0
#define GROUP_COUNT_DEC 1
#define GROUP_NONHIDE_STYLE 0
#define GROUP_HIDE_STYLE 1
#define MAX_GROUPMEMBER 500

typedef enum  _db_task
{
	CREATE_GROUP_TASK,
	JOIN_GROUP_TASK,
	DIRECTLYJOIN_GROUP_TASK,
	REMOVE_GROUP_TASK,
	MODIFY_GROUP_TASK
} db_task_t;

typedef enum _db_groupstatus
{
	DISMISS_GROUPSTATUS,    //The group has bee dismissed .  
	NORMAL_GROUPSTATUS,		//Normal group status
	DEPOSIT_GROUPSTATUS		//the group will be deposited by service. this attribute reserves for future using. 
} db_groupstatus_t;

typedef enum _db_memberinsertmode
{
	CREATE_INSERT_MEMBER,
	COMMON_INSERT_MEMBER
} db_memberinsertmode_t;

typedef enum _db_groupcreatemode
{
	NORMAL_CREATION,
	NORMAL_CREATIONWITHHIDE,
	TWOBARCODE_CREATION,
	TWOBARCODE_CREATIONWITHHIDE
} db_groupcreatemode_t;

typedef enum _db_appygroupmode
{
	NORMAL_APPY,		// 普通加群
	FACE2FACE_APPY		// 面对面加群
}db_appygroupmode_t;

//enum 
//{
//	DIRECTLYJOIN_GROUP,
//	PERMITJOIN_GROUP,
//	FORBITTONJOION_GROUP
//};

typedef enum _db_groupjoinmode
{
	APPLY_JOIN_MODE,
	INVITE_JOIN_MODE,
	PERMIT_JOIN_MODE
	
} db_groupjoinmode_t;

typedef enum _db_groupremovemode
{
	QUIT_REMOVE_MODE,
	KICKOUT_REMOVE_MODE
	
} db_groupremovemode_t;


typedef enum _db_groupmemberstatus
{
	QUIT_MEMBER,
	APPLY_MEMBER,
	INVITE_MEMBER,
	MASTER_MEMBER
} db_groupmemberstatus_t;
	
typedef unordered_map<string,uint8_t>  AccessUserMap_t;


typedef struct _AccessUserPara
{
	void* pUserData;					// In normally it points to protobuf struct 
	AccessUserMap_t InviteeUserMap;		// invitee user information . user id and it's permission attribute. 
	string sGroupId;					// reserved for creating group
	string sMasterId;					// group master id
	UidCode_t sessionId;				// current sessionid used to send packet.
	uint8_t bMode;						// to distinguish action mode . 0: apply ; 1: invite; 2: permit
	uint8_t grpNewState;				// -1:no change 0:dismiss 1:active 2:deposit 
	_AccessUserPara()
		:grpNewState(-1){}
} AccessUserPara_t;

typedef void(*AccessCallback)(CPacket* pAccessPack,AccessUserPara_t* pAccessUserPara,ErrCode errCode);

typedef struct _SqlAccess
{
	acl::string sAccessSql;
	db_task_t accessTask;
	AccessUserPara_t* pAccessPara;
	acl::db_pool* pAccessSqlPool;
	acl::redis_client_cluster* pAccessRedisCluster;
	CPacket* pAccessPacket;
	AccessCallback accessCb;
} SqlAccess_t;

class CDbHelper //: public CMysqlHelper , public CRedisHelper
{
public:
	CDbHelper();
	~CDbHelper();

	bool Initialize(CConfigFileReader * pConfigReader);
	bool DatabaseHelper(SqlAccess_t &sqlAccess);
private:
	CImThreadPool* m_pThreadPool;
	acl::redis_client_cluster* m_pRedisCluster;
	acl::db_pool* m_pMysqlPool;
	uint16_t m_nPoolSize;
};


class CDbTask : public CTask
{
public:
 	CDbTask(SqlAccess_t &sqlAccess);
	~CDbTask();

	virtual void run();

protected:

	virtual void CreateGroupProc(void);
	virtual void JoinGroupProc(void);
	virtual void JoinDirectlyGroupProc(void);
	virtual void ModifyGroupProc(void);
	virtual void RemoveGroupProc(void);
	
	virtual ErrCode ApplyGroup(void);
	virtual ErrCode InviteGroup(void);
	virtual ErrCode QuitGroup(void);
	virtual ErrCode KickoutGroup(void);
	virtual ErrCode ModifyGroupAnnouncement(string sUserId,string sGroupId,string sContent);
	virtual ErrCode ModifyGroupName(string sUserId,string sGroupId,string sContent);
	virtual ErrCode ModifyGroupAvatar(string sUserId,string sGroupId,string sContent);
	virtual ErrCode ModifyGroupNickName(string sUserId,string sGroupId,string sContent);
	virtual ErrCode ModifyGroupMaster(string sUserId,string sGroupId,string sInvolvedId);
	virtual ErrCode CreateInviteGroup(GroupCreate* pInstData);
protected:	
	virtual bool GetGroupInfo(GroupInfo_t &info);
	ErrCode IsMember(string sUserId,string sGroupId,uint8_t &bMemberStatus);
	ErrCode IsMaster(string sUserId,string sGroupId);
	ErrCode VerifyGroupMasterPermission(string sUserId, string sGroupId);
	ErrCode VerifyGroupUserPermission(string sUserId, uint8_t &bGroupPermit);
	//ErrCode VerifyGroupPermission(string sUserId,string sGroupId);
	ErrCode VerifyGroupCreatingAuth(string sUserId,uint8_t bRole,uint16_t nGroupLimit);
	ErrCode VerifyGroupJoiningAuth(string sGroupId, uint8_t bRole, uint16_t nMemberLimit,uint16_t nJoinNum = 1, uint32_t appyType = 0);
	ErrCode InsertGroupMember(string sUserId,string sGroupId,uint8_t bStatus,
								uint8_t bHide=GROUP_NONHIDE_STYLE,
								db_memberinsertmode_t bMode=COMMON_INSERT_MEMBER); //0: create mode ;1: normal insert;
	ErrCode UpdateGroupMember(string sUserId,string sGroupId,uint8_t bStatus);								
	ErrCode InsertMultiGroupMember(string sGroupId,uint8_t bStatus);
	ErrCode InsertGroup(GroupCreate* pCreateInst,string sAppId);
	ErrCode UpdateGroupCount(string sGroupId, uint16_t nCount,uint8_t bMethod=GROUP_COUNT_INC); //0: inc group count ; 1: dec group coount;
	ErrCode UpdateGroupStatus(string sGroupId,uint8_t bStatus);
	ErrCode RemoveGroupMember(string sUserId,string sGroupId, bool bKick = true);

	void ParseInviteeList(GroupInvite* pInstData);
	void ParseCreateInvitee(GroupCreate* pInstData);
	void GenerateGroupId(string& sGroupId,string sAppId);
	
private:
	acl::db_pool* m_pAccessSqlPool;	
	acl::redis_client_cluster* m_pAccessRedisCluster;
	CPacket* m_pPacketProcess;
	AccessUserPara_t* m_pAccessPara;
	AccessCallback m_accessCb;
	db_task_t m_accessTask;
	acl::string m_sAccessSql;
	acl::redis_hash m_hashRedis;    // redis hash handle, Allocate from redis cluster pool.
	acl::redis m_redis;				// redis common handle , Also allocate from redis cluster pool	
	acl::db_handle* m_hDB;			// mysql access handle, Allocate from mysql conntection pool. 	
};



#endif
